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

#include "ecal_service_server_session_impl_v0.h"

#include "ecal_service_log_helpers.h"
#include "ecal_service_log_defs.h"

#include "ecal_service_tcp_protocol_layout.h"

///////////////////////////////////////////////
// Create, Constructor, Destructor
///////////////////////////////////////////////

namespace eCAL
{
  namespace service
  {

    std::shared_ptr<ServerSessionV0> ServerSessionV0::create(asio::io_context&              io_context_
                                                            , const ServerServiceCallbackT& service_callback
                                                            , const ServerEventCallbackT&   event_callback
                                                            , const DeleteCallbackT&        delete_callback
                                                            , const LoggerT&                logger)
    {
      std::shared_ptr<ServerSessionV0> instance = std::shared_ptr<ServerSessionV0>(new ServerSessionV0(io_context_, service_callback, event_callback, delete_callback, logger));
      return instance;
    }

    ServerSessionV0::ServerSessionV0(asio::io_context&              io_context_
                                    , const ServerServiceCallbackT& service_callback
                                    , const ServerEventCallbackT&   event_callback
                                    , const DeleteCallbackT&        delete_callback
                                    , const LoggerT&                logger)
      : ServerSessionBase(io_context_, service_callback, event_callback, delete_callback)
      , logger_                   (logger)
      , service_strand_           (io_context_)
      , state_                    (State::NOT_CONNECTED)
    {
      ECAL_SERVICE_LOG_DEBUG_VERBOSE(logger_, "Server Session Created");
    }

    // Destructor
    ServerSessionV0::~ServerSessionV0()
    {
      ECAL_SERVICE_LOG_DEBUG_VERBOSE(logger_, "Server Session Deleted");
    }

    ///////////////////////////////////////////////
    // Data receiving and sending
    ///////////////////////////////////////////////
    void ServerSessionV0::start()
    {
      // Go to handshake state
      state_ = State::CONNECTED;

      ECAL_SERVICE_LOG_DEBUG(logger_, "[" + get_connection_info_string(socket_) + "] " + "Starting...");

      const std::string message = "Client has connected. Using protocol version 0.";
      event_callback_(eCAL_Server_Event::server_event_connected, message);
      logger_(LogLevel::Info, "[" + get_connection_info_string(socket_) + "] " + message);

      // Disable Nagle's algorithm. Nagles Algorithm will otherwise cause the
      // Socket to wait for more data, if it encounters a frame that can still
      // fit more data. Obviously, this is an awfull default behaviour, if we
      // want to transmit our data in a timely fashion.
      {
        asio::error_code socket_option_ec;
        socket_.set_option(asio::ip::tcp::no_delay(true), socket_option_ec);
        if (socket_option_ec)
        {
          logger_(LogLevel::Warning, "[" + get_connection_info_string(socket_) + "] " + "Failed setting tcp::no_delay option: " + socket_option_ec.message());
        }                                      
      }

      ECAL_SERVICE_LOG_DEBUG(logger_, "[" + get_connection_info_string(socket_) + "] " + "Waiting for service request...");
      socket_.async_read_some(asio::buffer(data_, max_length)
                            , service_strand_.wrap([me = shared_from_this()](asio::error_code ec, std::size_t bytes_read)
                              {
                                me->handle_read(ec, bytes_read, std::make_shared<std::string>());
                              }));
    }

    void ServerSessionV0::stop()
    {
      ECAL_SERVICE_LOG_DEBUG(logger_, "[" + get_connection_info_string(socket_) + "] " + "Stopping...");
      
      {
        // Shutdown the socket
        asio::error_code ec;
        socket_.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
      }

      {
        // Close the socket
        asio::error_code ec;
        socket_.close(ec);
      }
    }

    void ServerSessionV0::handle_read(const asio::error_code& ec, size_t bytes_transferred, const std::shared_ptr<std::string>& request)
    {
      ECAL_SERVICE_LOG_DEBUG_VERBOSE(logger_, "[" + get_connection_info_string(socket_) + "] " + "Received " + std::to_string(bytes_transferred) + " bytes.");

      if (!ec)
      {
        // collect request
        *request += std::string(data_, bytes_transferred);
        // are there some more data on the socket ?
        
        size_t bytes_available_on_socket(0);
        {
          asio::error_code socket_available_ec;
          bytes_available_on_socket = socket_.available(socket_available_ec);

          if (socket_available_ec)
          {
            // -- This code is a copy of the code in the else branch below. --
            state_ = State::FAILED;
            const auto message = "Disconnected on read: " + socket_available_ec.message();
            logger_(eCAL::service::LogLevel::Info, "[" + get_connection_info_string(socket_) + "] " + message);
            event_callback_(server_event_disconnected, message);
            delete_callback_(shared_from_this());
            return;
          }
        }

        if (bytes_available_on_socket != 0u)
        {
          ECAL_SERVICE_LOG_DEBUG_VERBOSE(logger_, "[" + get_connection_info_string(socket_) + "] " + "More data is available on socket! Reading more data...");

          socket_.async_read_some(asio::buffer(data_, max_length)
                                , service_strand_.wrap([me = shared_from_this(), request](asio::error_code ec, std::size_t bytes_read)
                                  {
                                    me->handle_read(ec, bytes_read, request);
                                  }));
        }
        // no more data
        else
        {
          // execute service callback
          ECAL_SERVICE_LOG_DEBUG_VERBOSE(logger_, "[" + get_connection_info_string(socket_) + "] " + "Socket currently doesn't hold any more data.");
          ECAL_SERVICE_LOG_DEBUG(logger_, "[" + get_connection_info_string(socket_) + "] " + "handle_read final request size: " + std::to_string(request->size()) + ". Executing callback...");

          auto response = std::make_shared<std::string>();
          service_callback_(request, response);

          ECAL_SERVICE_LOG_DEBUG_VERBOSE(logger_, "[" + get_connection_info_string(socket_) + "] " + "Server callback executed. Reponse size: " + std::to_string(response->size()) + ".");
   
          const auto header      = std::make_shared<eCAL::service::TcpHeaderV0>();
          header->package_size_n = htonl(static_cast<uint32_t>(response->size()));

          const std::vector<asio::const_buffer> buffer_list { asio::buffer(reinterpret_cast<const char*>(header.get()), sizeof(eCAL::service::TcpHeaderV0))
                                                            , asio::buffer(*response)};

          asio::async_write(socket_
                          , buffer_list
                          , service_strand_.wrap([me = shared_from_this(), header, response](asio::error_code ec, std::size_t bytes_written)
                            {
                              me->handle_write(ec, bytes_written);
                            }));
        }
      }
      else
      {
        state_ = State::FAILED;
        const auto message = "Disconnected on read: " + ec.message();
        logger_(eCAL::service::LogLevel::Info, "[" + get_connection_info_string(socket_) + "] " + message);
        event_callback_(server_event_disconnected, message);
        delete_callback_(shared_from_this());
      }
    }

    void ServerSessionV0::handle_write(const asio::error_code& ec, std::size_t /*bytes_transferred*/)
    {
      if (!ec)
      {
        ECAL_SERVICE_LOG_DEBUG(logger_, "[" + get_connection_info_string(socket_) + "] " + "Waiting for service request...");
        socket_.async_read_some(asio::buffer(data_, max_length)
                              , service_strand_.wrap([me = shared_from_this()](asio::error_code ec, std::size_t bytes_read)
                                {
                                  me->handle_read(ec, bytes_read, std::make_shared<std::string>());
                                }));
      }
      else
      {
        state_ = State::FAILED;
        const auto message = "Disconnected on write: " + ec.message();
        logger_(eCAL::service::LogLevel::Error, "[" + get_connection_info_string(socket_) + "] " + message);
        event_callback_(server_event_disconnected, message);
        delete_callback_(shared_from_this());
      }
    }

  } // namespace service
} // namespace eCAL
