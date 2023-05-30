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

#include "ecal_service_tcp_server.h"

#include "asio_tcp_server_logging.h"
#if(ECAL_ASIO_TCP_SERVER_LOG_DEBUG_VERBOSE_ENABLED || ECAL_ASIO_TCP_SERVER_LOG_DEBUG_ENABLED)
  #include <iostream>
#endif // (ECAL_ASIO_TCP_SERVER_LOG_DEBUG_VERBOSE_ENABLED || ECAL_ASIO_TCP_SERVER_LOG_DEBUG_ENABLED)


#include "ecal_service_tcp_session_server.h"
#include "ecal_service_tcp_session_v1_server.h"

namespace eCAL
{
  namespace service
  {
    ///////////////////////////////////////////
    // Constructor, Destructor, Create
    ///////////////////////////////////////////

    std::shared_ptr<Server> Server::create(asio::io_context&      io_context
                                                        , unsigned int            protocol_version
                                                        , std::uint16_t           port
                                                        , const ServiceCallbackT& service_callback
                                                        , const EventCallbackT&   event_callback
                                                        , const LoggerT&          logger)
    {
      // Create a new instance with the protected constructor
      // Note: make_shared not possible, because constructor is protected
      auto instance = std::shared_ptr<Server>(new Server(io_context, port, service_callback, event_callback, logger));

      // Directly Start accepting new connections
      instance->start_accept(protocol_version);

      // Return the created and started instance
      return instance;
    }

    Server::Server(asio::io_context&        io_context
                                  , std::uint16_t           port
                                  , const ServiceCallbackT& service_callback
                                  , const EventCallbackT&   event_callback
                                  , const LoggerT&          logger)
      : io_context_      (io_context)
      , acceptor_        (io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
      , service_callback_(service_callback)
      , event_callback_  (event_callback)
      , connection_count_(0)
      , logger_          (logger)
    {
      logger_(LogLevel::DebugVerbose, "Service Created");
    }

    // TODO: check if I must close the acceptor etc.
    Server::~Server()
    {
      // Close acceptor, if necessary
      {
        asio::error_code ec;
        acceptor_.close(ec);
      }

      logger_(LogLevel::DebugVerbose, "Service Deleted");
    }

    ///////////////////////////////////////////
    // API
    ///////////////////////////////////////////

    bool Server::is_connected() const
    {
      return connection_count_ > 0;
    }

    std::uint16_t Server::get_port() const
    {
      return acceptor_.local_endpoint().port();
    }

    void Server::start_accept(unsigned int version)
    {
      logger_(LogLevel::DebugVerbose, "Service waiting for next client...");

      // Create the callbacks for the session. Those callbacks are basically
      // wrapping the class methods of this class. However, the lambda functions
      // will be stored in the session and we have no idea when that session will
      // be destoyed. It may be destoyed AFTER this class has died. Therefore, we
      // save a weak_ptr to this class in the capture and later only execute the
      // callback, if this class is still alive.

      const eCAL::service::ServerSessionBase::ServiceCallbackT service_callback
              = [weak_me = std::weak_ptr<Server>(shared_from_this())](const std::shared_ptr<std::string>& request, const std::shared_ptr<std::string>& response) -> int
                {
                  // Create a shared_ptr to the class. If it doesn't exist
                  // anymore, we will get a nullpointer. In that case, we cannot
                  // execute the callback.
                  const std::shared_ptr<Server> me = weak_me.lock();
                  if (me)
                  {
                    return me->service_callback_(request, response);
                  }
                  else
                  {
                    return 0;
                  }
                };

      const eCAL::service::ServerSessionBase::EventCallbackT event_callback
              = [weak_me = std::weak_ptr<Server>(shared_from_this())](const eCAL_Server_Event event, const std::string& message) -> void
                {
                  // Create a shared_ptr to the class. If it doesn't exist
                  // anymore, we will get a nullpointer. In that case, we cannot
                  // execute the callback.
                  const std::shared_ptr<Server> me = weak_me.lock();
                  if (me)
                  {
                    return me->on_event(event, message);
                  }
                };
      //const eCAL::ServerSession::EventCallbackT event_callback
      //        = [weak_me = std::weak_ptr<Server>(shared_from_this())](eCAL_Server_Event server_event, const std::string& todo_name_this_whatever_it_is) -> void
      //          {
      //            // Create a shared_ptr to the class. If it doesn't exist
      //            // anymore, we will get a nullpointer. In that case, we cannot
      //            // execute the callback.
      //            const std::shared_ptr<Server> me(weak_me);
      //            if (me)
      //            {
      //              me->on_event(server_event, todo_name_this_whatever_it_is);
      //            }
      //          };

      std::shared_ptr<eCAL::service::ServerSessionBase> new_session;
      if (version == 0)
      {
        //new_session = eCAL::ServerSessionV0::create(io_context_, request_callback, event_callback);
      }
      else
      {
        new_session = eCAL::service::ServerSessionV1::create(io_context_, service_callback, event_callback, logger_);
      }

      // Accept new session.
      // By only storing a weak_ptr to this, we assure that the user can still
      // delete the service from the outside.
      acceptor_.async_accept(new_session->socket()
              , [weak_me = std::weak_ptr<Server>(shared_from_this()), new_session, version, logger_copy = logger_](auto ec)
                {
                  // TODO: The Destructor must close the accpetor
                  if (ec)
                  {
                    logger_copy(LogLevel::Info, "Service shutting down: " + ec.message());
                    return; 

                    // TODO: Decide whether to return or to continue accepting. only re-try accepting, if the ec didn't tell us to stop!!!
                  }
                  else
                  {
  #if (ECAL_ASIO_TCP_SERVER_LOG_DEBUG_ENABLED)
                    const auto message = "[Server] Successfully accepted new session: " + new_session->get_connection_info_string() ;
                    std::cout << message << std::endl;
  #endif

                    logger_copy(LogLevel::DebugVerbose, "Service client initiated connection...");

                    // Start the new session, that now has a connection
                    // The session will call the callback and increase the connection count
                    new_session->start();
                  }

                  // TODO: only re-try accepting, if the ec didn't tell us to stop!!!
                  // Continue creating and accepting the next session
                  const std::shared_ptr<Server> me = weak_me.lock();
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

    // TODO Remove
    //int Server::on_request(const std::string& request, std::string& response)
    //{
    //  if (service_callback_)
    //  {
    //    // Call the the callback
    //    return service_callback_(request, response);
    //  }
    //  else
    //  {
    //    // If there is no callback, we return 0
    //    return 0;
    //  }
    //}

    void Server::on_event(eCAL_Server_Event event, const std::string& message)
    {
      // Increase / decrease the connection count
      switch (event)
      {
      case server_event_connected:
        connection_count_++;
        break;
      case server_event_disconnected:
        connection_count_--;
        break;
      default:
        break;
      }

      event_callback_(event, message);
    }

  } // namespace service
} // namespace eCAL
