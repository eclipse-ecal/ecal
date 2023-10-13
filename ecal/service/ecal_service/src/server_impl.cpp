/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2019 Continental Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *      http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * ========================= eCAL LICENSE =================================
*/

#include "server_impl.h"

#include <algorithm>

#include "server_session_impl_v1.h"
#include "server_session_impl_v0.h"

#include "log_defs.h"

namespace eCAL
{
  namespace service
  {
    ///////////////////////////////////////////
    // Constructor, Destructor, Create
    ///////////////////////////////////////////

    std::shared_ptr<ServerImpl> ServerImpl::create(const std::shared_ptr<asio::io_context>& io_context
                                                  , std::uint8_t                            protocol_version
                                                  , std::uint16_t                           port
                                                  , const ServerServiceCallbackT&           service_callback // TODO: The service callback may block a long time. This may cause the entire network stack to wait for long running service callbacks. Maybe it is a good idea to have some kind of "future" object, that the user can hand to some differen io_context or to a custom thread. That thread will then work on the object and call some function / let it go out of scope, which will then trigger sending the response to the client.
                                                  , bool                                    parallel_service_calls_enabled
                                                  , const ServerEventCallbackT&             event_callback
                                                  , const LoggerT&                          logger)
    {
      // Create a new instance with the protected constructor
      // Note: make_shared not possible, because constructor is protected
      auto instance = std::shared_ptr<ServerImpl>(new ServerImpl(io_context, service_callback, parallel_service_calls_enabled, event_callback, logger));

      // Directly Start accepting new connections
      instance->start_accept(protocol_version, port);

      // Return the created and started instance
      return instance;
    }

    ServerImpl::ServerImpl(const std::shared_ptr<asio::io_context>& io_context
                          , const ServerServiceCallbackT&           service_callback
                          , bool                                    parallel_service_calls_enabled
                          , const ServerEventCallbackT&             event_callback
                          , const LoggerT&                          logger)
      : io_context_                    (io_context)
      , acceptor_                      (*io_context)
      , parallel_service_calls_enabled_(parallel_service_calls_enabled)
      , service_callback_common_strand_(std::make_shared<asio::io_context::strand>(*io_context))
      , service_callback_              (service_callback)
      , event_callback_                (event_callback)
      , logger_                        (logger)
    {
      ECAL_SERVICE_LOG_DEBUG_VERBOSE(logger_, "Service Created");
    }

    ServerImpl::~ServerImpl()
    {
      stop();

      ECAL_SERVICE_LOG_DEBUG_VERBOSE(logger_, "Service Deleted");
    }

    ///////////////////////////////////////////
    // API
    ///////////////////////////////////////////

    bool ServerImpl::is_connected() const
    {
      return get_connection_count() > 0;
    }

    int ServerImpl::get_connection_count() const
    {
      const std::lock_guard<std::mutex> session_list_lock(session_list_mutex_);
      int connection_count = 0;
      
      // Iterate over all sessions and count the ones that are still alive
      for (const auto& session : session_list_)
      {
        auto session_ptr = session.lock();
        if (session_ptr && (session_ptr->get_state() != eCAL::service::State::FAILED))
        {
          ++connection_count;
        }
      }

      return connection_count;
    }

    std::uint16_t ServerImpl::get_port() const
    {
      asio::error_code ec;

      const std::lock_guard<std::mutex> acceptor_lock(acceptor_mutex_);

      auto endpoint = acceptor_.local_endpoint(ec);
      if (!ec)
      {
        return endpoint.port();
      }
      else
      {
        return 0;
      }
    }

    void ServerImpl::start_accept(std::uint8_t protocol_version, std::uint16_t port)
    {
      ECAL_SERVICE_LOG_DEBUG(logger_, "Service starting to accept new connections...");

      // set up the acceptor to listen on the tcp port

      const asio::ip::tcp::endpoint endpoint(asio::ip::address_v6(), port);
    
      ECAL_SERVICE_LOG_DEBUG_VERBOSE(logger_, "Service Server: Opening acceptor...");
      {
        asio::error_code ec;

        {
          const std::lock_guard<std::mutex> acceptor_lock(acceptor_mutex_);
          acceptor_.open(endpoint.protocol(), ec);
        }
        if (ec)
        {
          logger_(eCAL::service::LogLevel::Error, "Service Server: Error opening acceptor:" + ec.message());
          // return false; What do do here?
        }
      }

      ECAL_SERVICE_LOG_DEBUG_VERBOSE(logger_, "Service Server: Setting \"reuse_address\" option...");


      {
        asio::error_code ec;
        {
          acceptor_.set_option(asio::ip::tcp::acceptor::reuse_address(true), ec);
          const std::lock_guard<std::mutex> acceptor_lock(acceptor_mutex_);
        }
        if (ec)
        {
          logger_(eCAL::service::LogLevel::Error, "Service Server: Error setting \"reuse_address\" option:" + ec.message());
          // return false; What do do here?
        }
      }

      ECAL_SERVICE_LOG_DEBUG_VERBOSE(logger_, "Service Server: Binding acceptor to endpoint...");

      {
        asio::error_code ec;
        {
          const std::lock_guard<std::mutex> acceptor_lock(acceptor_mutex_);
          acceptor_.bind(endpoint, ec);
        }
        if (ec)
        {
          logger_(eCAL::service::LogLevel::Error, "Service Server: Error binding acceptor:" + ec.message());
          // return false; What do do here?
        }
      }

      ECAL_SERVICE_LOG_DEBUG_VERBOSE(logger_, "Service Server: Listening on acceptor...");

      {
        asio::error_code ec;
        {
          const std::lock_guard<std::mutex> acceptor_lock(acceptor_mutex_);
          acceptor_.listen(asio::socket_base::max_listen_connections, ec);
        }
        if (ec)
        {
          logger_(eCAL::service::LogLevel::Error, "Service Server: Error listening on acceptor: " + ec.message());
          // return false; What do do here?
        }
      }

      wait_for_next_client(protocol_version);
    }

    void ServerImpl::wait_for_next_client(std::uint8_t protocol_version)
    {
      ECAL_SERVICE_LOG_DEBUG_VERBOSE(logger_, "Service waiting for next client...");

      const eCAL::service::ServerSessionBase::ShutdownCallbackT shutdown_callback
                = [weak_me = std::weak_ptr<ServerImpl>(shared_from_this())](const std::shared_ptr<eCAL::service::ServerSessionBase>& session_to_remove) -> void
                  {
                  // Create a shared_ptr to the class. If it doesn't exist
                  // anymore, we will get a nullpointer. In that case, we cannot
                  // execute the callback.
                  const std::shared_ptr<ServerImpl> me = weak_me.lock();
                  if (me)
                  {
                    const std::lock_guard<std::mutex> session_list_lock(me->session_list_mutex_);

                    // Remove the session from the session list
                    const auto session_it = std::find_if(me->session_list_.begin(), me->session_list_.end()
                          , [session_to_remove](const auto& existing_session) { return session_to_remove == existing_session.lock(); });
                    if (session_it != me->session_list_.end())
                    {
                      me->session_list_.erase(session_it);
                    }
                  }
                };

      std::shared_ptr<eCAL::service::ServerSessionBase> new_session;

      std::shared_ptr<asio::io_context::strand>         service_callback_strand;
      if (parallel_service_calls_enabled_)
      {
        service_callback_strand = std::make_shared<asio::io_context::strand>(*io_context_);
      }
      else
      {
        service_callback_strand = service_callback_common_strand_;
      }

      if (protocol_version == 0)
      {
        new_session = eCAL::service::ServerSessionV0::create(io_context_, service_callback_, service_callback_strand, event_callback_, shutdown_callback, logger_);
      }
      else
      {
        new_session = eCAL::service::ServerSessionV1::create(io_context_, service_callback_, service_callback_strand, event_callback_, shutdown_callback, logger_);
      }

      // Accept new session.
      // By only storing a weak_ptr to this, we assure that the user can still
      // delete the service from the outside.
      const std::lock_guard<std::mutex> acceptor_lock(acceptor_mutex_);
      acceptor_.async_accept(new_session->socket()
              , [weak_me = std::weak_ptr<ServerImpl>(shared_from_this()), new_session, protocol_version, logger_copy = logger_](auto ec)
                {
                  if (ec)
                  {
                    logger_copy(LogLevel::Info, "Service shutting down: " + ec.message());
                    return; 
                  }
                  else
                  {

                    ECAL_SERVICE_LOG_DEBUG_VERBOSE(logger_copy, "Service client initiated connection...");

                    const std::shared_ptr<ServerImpl> me = weak_me.lock();
                    if (me)
                    {
                      // Add the new session to the session list. The session list is a list of weak_ptr.
                      // Therefore, the session list will not keep the session alive, they will do that themselves.
                      const std::lock_guard<std::mutex> session_list_lock(me->session_list_mutex_);
                      me->session_list_.push_back(new_session);

                      // Start the new session, that now has a connection
                      // The session will call the callback and increase the connection count
                      new_session->start();
                    }
                  }

                  // Continue creating and accepting the next session
                  const std::shared_ptr<ServerImpl> me = weak_me.lock();
                  if (me)
                  {
                    me->wait_for_next_client(protocol_version);
                  }
                  else
                  {
                    logger_copy(LogLevel::Info, "Service shutting down.");
                  }
                });

    }

    void ServerImpl::stop()
    {
      {
        // Lock mutex for making the stop function thread safe
        const std::lock_guard<std::mutex> acceptor_lock(acceptor_mutex_);

        // Close acceptor, if necessary
        if (acceptor_.is_open())
        {
          asio::error_code ec;
          acceptor_.close(ec);
        }
      }
      
      // Stop all sessions to clients
      {
        const std::lock_guard<std::mutex> session_list_lock(session_list_mutex_);
        for(const auto& session_weak : session_list_)
        {
          const auto session = session_weak.lock();
          if (session)
            session->stop();
        }
      }
    }
  } // namespace service
} // namespace eCAL
