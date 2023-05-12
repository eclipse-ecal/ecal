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

#include "asio_tcp_server.h"

#include "asio_tcp_server_logging.h"
#if(ECAL_ASIO_TCP_SERVER_LOG_DEBUG_VERBOSE_ENABLED || ECAL_ASIO_TCP_SERVER_LOG_DEBUG_ENABLED)
  #include <iostream>
#endif // (ECAL_ASIO_TCP_SERVER_LOG_DEBUG_VERBOSE_ENABLED || ECAL_ASIO_TCP_SERVER_LOG_DEBUG_ENABLED)

namespace eCAL
{
  ///////////////////////////////////////////
  // Constructor, Destructor, Create
  ///////////////////////////////////////////

  std::shared_ptr<CAsioTcpServer> CAsioTcpServer::create(asio::io_context&      io_context
                                                      , unsigned int            protocol_version
                                                      , std::uint16_t           port
                                                      , const RequestCallbackT& request_callback
                                                      , const EventCallbackT&   event_callback)
  {
    // Create a new instance with the protected constructor
    // Note: make_shared not possible, because constructor is protected
    auto instance = std::shared_ptr<CAsioTcpServer>(new CAsioTcpServer(io_context, port, request_callback, event_callback));

    // Directly Start accepting new connections
    instance->start_accept(protocol_version);

    // Return the created and started instance
    return instance;
  }

  CAsioTcpServer::CAsioTcpServer(asio::io_context&        io_context
                                , std::uint16_t           port
                                , const RequestCallbackT& request_callback
                                , const EventCallbackT&   event_callback)
    : io_context_      (io_context)
    , acceptor_        (io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
    , request_callback_(request_callback)
    , event_callback_  (event_callback)
    , connection_count_(0)
  {
#if (ECAL_ASIO_TCP_SERVER_LOG_DEBUG_VERBOSE_ENABLED)
    const auto message = "[CAsioTcpServer] Created";
    std::cout << message << std::endl;
#endif
  }

  // TODO: check if I must close the acceptor etc.
  CAsioTcpServer::~CAsioTcpServer()
  {
#if (ECAL_ASIO_TCP_SERVER_LOG_DEBUG_VERBOSE_ENABLED)
    const auto message = "[CAsioTcpServer] Deleted";
    std::cout << message << std::endl;
#endif
  }

  ///////////////////////////////////////////
  // API
  ///////////////////////////////////////////

  bool CAsioTcpServer::is_connected() const
  {
    return connection_count_ > 0;
  }

  std::uint16_t CAsioTcpServer::get_port() const
  {
    return acceptor_.local_endpoint().port();
  }

  void CAsioTcpServer::start_accept(unsigned int version)
  {
    // Create the callbacks for the session. Those callbacks are basically
    // wrapping the class methods of this class. However, the lambda functions
    // will be stored in the session and we have no idea when that session will
    // be destoyed. It may be destoyed AFTER this class has died. Therefore, we
    // save a weak_ptr to this class in the capture and later only execute the
    // callback, if this class is still alive.

    eCAL::CAsioTcpServerSession::RequestCallbackT request_callback
            = [weak_me = std::weak_ptr<CAsioTcpServer>(shared_from_this())](const std::string& request, std::string& response) -> int
              {
                // Create a shared_ptr to the class. If it doesn't exist
                // anymore, we will get a nullpointer. In that case, we cannot
                // execute the callback.
                std::shared_ptr<CAsioTcpServer> me(weak_me);
                if (me)
                {
                  return me->on_request(request, response);
                }
                else
                {
                  return 0;
                }
              };

    eCAL::CAsioTcpServerSession::EventCallbackT event_callback
            = [weak_me = std::weak_ptr<CAsioTcpServer>(shared_from_this())](eCAL_Server_Event server_event, const std::string& todo_name_this_whatever_it_is) -> void
              {
                // Create a shared_ptr to the class. If it doesn't exist
                // anymore, we will get a nullpointer. In that case, we cannot
                // execute the callback.
                std::shared_ptr<CAsioTcpServer> me(weak_me);
                if (me)
                {
                  me->on_event(server_event, todo_name_this_whatever_it_is);
                }
              };

    std::shared_ptr<eCAL::CAsioTcpServerSession> new_session;
    if (version == 0)
    {
      new_session = eCAL::CAsioTcpServerSessionV0::create(io_context_, request_callback, event_callback);
    }
    else
    {
      new_session = eCAL::CAsioTcpServerSessionV1::create(io_context_, request_callback, event_callback);
    }

    acceptor_.async_accept(new_session->socket()
            , [this, new_session, version](auto ec) // TODO: the this pointer is used. Check whether this should be a shared_ptr or weak_ptr
              {
                // TODO: The Destructor must close the accpetor and stop the io_service
                if (ec)
                {
#if (ECAL_ASIO_TCP_SERVER_LOG_DEBUG_ENABLED)
                  const auto message = "[CAsioTcpServer] Error accepting session: " + ec.message();
                  std::cerr << message << std::endl;
#endif
                  // TODO: Decide whether to return or to continue accepting. only re-try accepting, if the ec didn't tell us to stop!!!
                }
                else
                {
#if (ECAL_ASIO_TCP_SERVER_LOG_DEBUG_ENABLED)
                  const auto message = "[CAsioTcpServer] Successfully accepted new session: " + new_session->get_connection_info_string() ;
                  std::cout << message << std::endl;
#endif

                  // Start the new session, that now has a connection
                  // The session will call the callback and increase the connection count
                  new_session->start();
                }

                // TODO: only re-try accepting, if the ec didn't tell us to stop!!!
                // Continue creating and accepting the next session
                this->start_accept(version);
              });

  }

  int CAsioTcpServer::on_request(const std::string& request, std::string& response)
  {
    if (request_callback_)
    {
      // Call the the callback
      return request_callback_(request, response);
    }
    else
    {
      // If there is no callback, we return 0
      return 0;
    }
  }

  void CAsioTcpServer::on_event(eCAL_Server_Event event, const std::string& message)
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

    if (event_callback_)
    {
      // Call the the callback
      event_callback_(event, message);
    }
  }

} // namespace eCAL
