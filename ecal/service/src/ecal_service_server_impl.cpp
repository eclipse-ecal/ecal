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

#include "ecal_service_server_impl.h"

#include <algorithm>

#include "ecal_service_server_session_impl_v1.h"

#include "ecal_service_log_defs.h"

namespace eCAL
{
  namespace service
  {
    ///////////////////////////////////////////
    // Constructor, Destructor, Create
    ///////////////////////////////////////////

    std::shared_ptr<ServerImpl> ServerImpl::create(asio::io_context&              io_context
                                                  , std::uint8_t                  protocol_version
                                                  , std::uint16_t                 port
                                                  , const ServerServiceCallbackT& service_callback
                                                  , const ServerEventCallbackT&   event_callback
                                                  , const LoggerT&                logger)
    {
      // Create a new instance with the protected constructor
      // Note: make_shared not possible, because constructor is protected
      auto instance = std::shared_ptr<ServerImpl>(new ServerImpl(io_context, port, service_callback, event_callback, logger));

      // Directly Start accepting new connections
      instance->start_accept(protocol_version);

      // Return the created and started instance
      return instance;
    }

    ServerImpl::ServerImpl(asio::io_context&              io_context
                          , std::uint16_t                 port
                          , const ServerServiceCallbackT& service_callback
                          , const ServerEventCallbackT&   event_callback
                          , const LoggerT&                logger)
      : io_context_      (io_context)
      , acceptor_        (io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
      , service_callback_(service_callback)
      , event_callback_  (event_callback)
      , logger_          (logger)
    {
      ECAL_SERVICE_LOG_DEBUG_VERBOSE(logger_, "Service Created");
    }

    ServerImpl::~ServerImpl()
    {
      // Close acceptor, if necessary
      {
        asio::error_code ec;
        acceptor_.close(ec);
      }

      // TODO: Currently, there is no other way to stop the server than letting
      // it run out of scope. So closing the sessions from the destructor is fine.
      // However, should that behavior change in the future, I need to create
      // A special function for that, that is called from other places as well.
      {
        std::lock_guard<std::mutex> session_list_lock(session_list_mutex_);
        for(const auto& session_weak : session_list_)
        {
          const auto session = session_weak.lock();
          if (session)
            session->stop();
        }
      }

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
      std::lock_guard<std::mutex> session_list_lock(session_list_mutex_);
      return static_cast<int>(session_list_.size());
    }

    std::uint16_t ServerImpl::get_port() const
    {
      return acceptor_.local_endpoint().port();
    }

    void ServerImpl::start_accept(unsigned int version)
    {
      ECAL_SERVICE_LOG_DEBUG_VERBOSE(logger_, "Service waiting for next client...");

      const eCAL::service::ServerSessionBase::DeleteCallbackT delete_callback
                = [weak_me = std::weak_ptr<ServerImpl>(shared_from_this())](const std::shared_ptr<eCAL::service::ServerSessionBase>& session_to_remove) -> void
                  {
                  // Create a shared_ptr to the class. If it doesn't exist
                  // anymore, we will get a nullpointer. In that case, we cannot
                  // execute the callback.
                  const std::shared_ptr<ServerImpl> me = weak_me.lock();
                  if (me)
                  {
                    std::lock_guard<std::mutex> session_list_lock(me->session_list_mutex_);

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
      if (version == 0)
      {
        //new_session = eCAL::ServerSessionV0::create(io_context_, request_callback, event_callback);
      }
      else
      {
        new_session = eCAL::service::ServerSessionV1::create(io_context_, service_callback_, event_callback_, delete_callback, logger_);
      }

      // Accept new session.
      // By only storing a weak_ptr to this, we assure that the user can still
      // delete the service from the outside.
      acceptor_.async_accept(new_session->socket()
              , [weak_me = std::weak_ptr<ServerImpl>(shared_from_this()), new_session, version, logger_copy = logger_](auto ec)
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
                      std::lock_guard<std::mutex> session_list_lock(me->session_list_mutex_);
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
                    me->start_accept(version);
                  }
                  else
                  {
                    logger_copy(LogLevel::Info, "Service shutting down.");
                  }
                });

    }

  } // namespace service
} // namespace eCAL
