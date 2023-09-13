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

#include "server_session_impl_v1.h"

#include "protocol_v1.h"

#include "log_helpers.h"
#include "log_defs.h"

///////////////////////////////////////////////
// Create, Constructor, Destructor
///////////////////////////////////////////////

namespace eCAL
{
  namespace service
  {
    constexpr std::uint8_t ServerSessionV1::MIN_SUPPORTED_PROTOCOL_VERSION;
    constexpr std::uint8_t ServerSessionV1::MAX_SUPPORTED_PROTOCOL_VERSION;

    std::shared_ptr<ServerSessionV1> ServerSessionV1::create(const std::shared_ptr<asio::io_context>&          io_context
                                                            , const ServerServiceCallbackT&                    service_callback
                                                            , const std::shared_ptr<asio::io_context::strand>& service_callback_strand
                                                            , const ServerEventCallbackT&                      event_callback
                                                            , const ShutdownCallbackT&                         shutdown_callback
                                                            , const LoggerT&                                   logger)
    {
      std::shared_ptr<ServerSessionV1> instance = std::shared_ptr<ServerSessionV1>(new ServerSessionV1(io_context, service_callback, service_callback_strand, event_callback, shutdown_callback, logger));
      return instance;
    }

    ServerSessionV1::ServerSessionV1(const std::shared_ptr<asio::io_context>&          io_context
                                    , const ServerServiceCallbackT&                    service_callback
                                    , const std::shared_ptr<asio::io_context::strand>& service_callback_strand
                                    , const ServerEventCallbackT&                      event_callback
                                    , const ShutdownCallbackT&                         shutdown_callback
                                    , const LoggerT&                                   logger)
      : ServerSessionBase(io_context, service_callback, service_callback_strand, event_callback, shutdown_callback)
      , state_                    (State::NOT_CONNECTED)
      , accepted_protocol_version_(0)
      , logger_                   (logger)
    {
      ECAL_SERVICE_LOG_DEBUG_VERBOSE(logger_, "Server Session Created");
    }

    // Destructor
    ServerSessionV1::~ServerSessionV1()
    {
      ServerSessionV1::stop();
      ECAL_SERVICE_LOG_DEBUG_VERBOSE(logger_, "Server Session Deleted");
    }

    ///////////////////////////////////////////////
    // Data receiving and sending
    ///////////////////////////////////////////////
    void ServerSessionV1::start()
    {
      ECAL_SERVICE_LOG_DEBUG(logger_, "[" + get_connection_info_string(socket_) + "] " + "Starting...");

      // Disable Nagle's algorithm. Nagles Algorithm will otherwise cause the
      // Socket to wait for more data, if it encounters a frame that can still
      // fit more data. Obviously, this is an awfull default behaviour, if we
      // want to transmit our data in a timely fashion.
      {
        asio::error_code socket_option_ec;
        {
          const std::lock_guard<std::mutex> socket_lock(socket_mutex_);
          socket_.set_option(asio::ip::tcp::no_delay(true), socket_option_ec);
        }
        if (socket_option_ec)
        {
          logger_(LogLevel::Warning, "[" + get_connection_info_string(socket_) + "] " + "Failed setting tcp::no_delay option: " + socket_option_ec.message());
        }                                      
      }

      receive_handshake_request();
    }

    void ServerSessionV1::stop()
    {
      const std::lock_guard<std::mutex> socket_lock(socket_mutex_);
      if (socket_.is_open())
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
    }

    eCAL::service::State ServerSessionV1::get_state() const
    {
      return state_;
    }

    void ServerSessionV1::receive_handshake_request()
    {
      ECAL_SERVICE_LOG_DEBUG(logger_, "[" + get_connection_info_string(socket_) + "] " + "Waiting for protocol handshake request...");

      // Go to handshake state
      state_ = State::HANDSHAKE;

      eCAL::service::ProtocolV1::async_receive_payload(socket_, socket_mutex_
                            , [me = shared_from_this()](asio::error_code ec)
                              {
                                me->state_ = State::FAILED;
                                me->logger_(LogLevel::Error, "[" + get_connection_info_string(me->socket_) + "] " + "Failed receiving protocol handshake request: " + ec.message());
                                me->shutdown_callback_(me);
                              }
                            , [me = shared_from_this()](const std::shared_ptr<std::vector<char>>& header_buffer, const std::shared_ptr<std::string>& payload_buffer)
                              {
                                TcpHeaderV1* header = reinterpret_cast<TcpHeaderV1*>(header_buffer->data());
                                if (header->message_type != eCAL::service::MessageType::ProtocolHandshakeRequest)
                                {
                                  // The request is not a Handshake request.
                                  const std::string message = "Received invalid handshake request from client. Expected message type " 
                                                              + std::to_string(static_cast<std::uint8_t>(eCAL::service::MessageType::ProtocolHandshakeRequest)) 
                                                              + ", but received " + std::to_string(static_cast<std::uint8_t>(header->message_type));
                                  me->logger_(LogLevel::Fatal, "[" + get_connection_info_string(me->socket_) + "] " + message);

                                  me->state_ = State::FAILED;
                                  me->shutdown_callback_(me);
                                  return;
                                }
                                else
                                {
                                  // The request is a Handshake request
                                  ECAL_SERVICE_LOG_DEBUG_VERBOSE(me->logger_, "[" + get_connection_info_string(me->socket_) + "] " + "Received a handshake request of " + std::to_string(payload_buffer->size()) + " bytes.");

                                  // Resize payload if necessary. Will probably never be necessary
                                  if (payload_buffer->size() < sizeof(ProtocolHandshakeRequestMessage))
                                  {
                                    payload_buffer->resize(sizeof(ProtocolHandshakeRequestMessage), '\0');
                                  }
                                  const ProtocolHandshakeRequestMessage* handshake_request = reinterpret_cast<const ProtocolHandshakeRequestMessage*>(payload_buffer->data());

                                  // Compute the maximum supported protocol version by this server and the remote client
                                  const std::uint8_t both_supported_max_protocol_version = std::min(handshake_request->max_supported_protocol_version, MAX_SUPPORTED_PROTOCOL_VERSION);
                                  const std::uint8_t both_supported_min_protocol_version = std::max(handshake_request->min_supported_protocol_version, MIN_SUPPORTED_PROTOCOL_VERSION);

                                  if (both_supported_max_protocol_version >= both_supported_min_protocol_version)
                                  {
                                    // We have a protocol version that is supported by both sides. We choose the maximum supported version
                                    me->accepted_protocol_version_ = both_supported_max_protocol_version;
                                    ECAL_SERVICE_LOG_DEBUG_VERBOSE(me->logger_, "[" + get_connection_info_string(me->socket_) + "] " + "Choosing protocol version " + std::to_string(me->accepted_protocol_version_));

                                    // Send the handshake response to the client, telling him the protocol version we will use
                                    me->send_handshake_response();
                                  }
                                  else
                                  {
                                    const std::string message = std::string("Error while accepting connection from client. No common protocol version is found. ")
                                                              + "Client supports [min: " + std::to_string(handshake_request->min_supported_protocol_version) + ", max: " + std::to_string(handshake_request->max_supported_protocol_version) + "]. "
                                                              + "Server supports [min: " + std::to_string(MIN_SUPPORTED_PROTOCOL_VERSION) + ", max: " + std::to_string(MAX_SUPPORTED_PROTOCOL_VERSION) + "].";
                                    me->logger_(LogLevel::Error, "[" + get_connection_info_string(me->socket_) + "] " + message);

                                    //const auto message = get_log_string("ERROR", "Error connecting to server. Server reported an un-supported protocol version: " + std::to_string(handshake_response->accepted_protocol_version));
                                    //std::cerr << message << std::endl;
                                    me->state_ = State::FAILED;
                                    me->shutdown_callback_(me);
                                    return;
                                  }

                                }
                              });

    }

    void ServerSessionV1::send_handshake_response()
    {
      ECAL_SERVICE_LOG_DEBUG(logger_, "[" + get_connection_info_string(socket_) + "] " + "Sending protocol handshake response...");

      // Go to handshake state
      state_ = State::HANDSHAKE;

      // Create buffers
      const std::shared_ptr<TcpHeaderV1>  header_buffer  = std::make_shared<TcpHeaderV1>();
      const std::shared_ptr<std::string> payload_buffer = std::make_shared<std::string>();

      // Fill Handshake Response Message
      payload_buffer->resize(sizeof(ProtocolHandshakeResponseMessage), '\0');
      ProtocolHandshakeResponseMessage* handshake_response_message = reinterpret_cast<ProtocolHandshakeResponseMessage*>(const_cast<char*>(payload_buffer->data()));
      handshake_response_message->accepted_protocol_version = accepted_protocol_version_;

      // Fill TCP Header
      header_buffer->package_size_n = htonl(static_cast<std::uint32_t>(payload_buffer->size()));
      header_buffer->version        = accepted_protocol_version_;
      header_buffer->message_type   = MessageType::ProtocolHandshakeResponse;
      header_buffer->header_size_n  = htons(sizeof(TcpHeaderV1));

      eCAL::service::ProtocolV1::async_send_payload(socket_, socket_mutex_,header_buffer, payload_buffer
                          , [me = shared_from_this()](asio::error_code ec)
                            {
                              me->state_ = State::FAILED;
                              me->logger_(LogLevel::Error, "[" + get_connection_info_string(me->socket_) + "] " + "Failed sending protocol handshake response: " + ec.message());
                              me->shutdown_callback_(me);
                            }
                          , [me = shared_from_this()]()
                            {
                              me->state_ = State::CONNECTED;
                              ECAL_SERVICE_LOG_DEBUG_VERBOSE(me->logger_, "[" + get_connection_info_string(me->socket_) + "] " + "Successfully sent protocol handshake response.");

                              const std::string message = "Client has connected. Using protocol version " + std::to_string(me->accepted_protocol_version_) + ".";
                              me->logger_(LogLevel::Info, "[" + get_connection_info_string(me->socket_) + "] " + message);

                              me->state_ = State::CONNECTED;
                                
                              // call event callback
                              me->event_callback_(eCAL::service::ServerEventType::Connected, message);

                              me->receive_service_request();
                            });
    }

    void ServerSessionV1::receive_service_request()
    {
      ECAL_SERVICE_LOG_DEBUG(logger_, "[" + get_connection_info_string(socket_) + "] " + "Waiting for service request...");

      eCAL::service::ProtocolV1::async_receive_payload(socket_, socket_mutex_
                            , [me = shared_from_this()](asio::error_code ec)
                              {
                                const std::string message = "Server session disconnected while waiting for request: " + ec.message();
                                me->logger_(LogLevel::Info, "[" + get_connection_info_string(me->socket_) + "] " + message);

                                me->state_ = State::FAILED;
                                
                                // call event callback
                                me->event_callback_(eCAL::service::ServerEventType::Disconnected, message);
                                me->shutdown_callback_(me);
                              }
                            , service_callback_strand_->wrap([me = shared_from_this()](const std::shared_ptr<std::vector<char>>& header_buffer, const std::shared_ptr<std::string>& payload_buffer)
                              {
                                TcpHeaderV1* header = reinterpret_cast<TcpHeaderV1*>(header_buffer->data());
                                if (header->message_type != eCAL::service::MessageType::ServiceRequest)
                                {
                                  const std::string message = "Received invalid service request from client. Expected message type " 
                                                              + std::to_string(static_cast<std::uint8_t>(eCAL::service::MessageType::ServiceRequest)) 
                                                              + ", but received " + std::to_string(static_cast<std::uint8_t>(header->message_type));
                                  me->logger_(LogLevel::Fatal, "[" + get_connection_info_string(me->socket_) + "] " + message);

                                  // The request is not a Service request.
                                  me->state_ = State::FAILED;

                                  // call event callback
                                  me->event_callback_(eCAL::service::ServerEventType::Disconnected, message);
                                  
                                  me->shutdown_callback_(me);
                                  return;
                                }
                                else
                                {
                                  // The request is a Service request
                                  
                                  ECAL_SERVICE_LOG_DEBUG(me->logger_, "[" + get_connection_info_string(me->socket_) + "] " + "Received service request of " + std::to_string(payload_buffer->size()) + " bytes");

                                  // Call the service callback
                                  const std::shared_ptr<std::string> response_buffer = std::make_shared<std::string>();
                                  me->service_callback_(payload_buffer, response_buffer);

                                  // Send the response to the client
                                  me->send_service_response(response_buffer);
                                }
                              }));

    }

    void ServerSessionV1::send_service_response(const std::shared_ptr<std::string>& response_buffer)
    {
      // Create header_buffer
      const std::shared_ptr<TcpHeaderV1>  header_buffer  = std::make_shared<TcpHeaderV1>();
      header_buffer->package_size_n = htonl(static_cast<std::uint32_t>(response_buffer->size()));
      header_buffer->version        = accepted_protocol_version_;
      header_buffer->message_type   = MessageType::ServiceResponse;
      header_buffer->header_size_n  = htons(sizeof(TcpHeaderV1));

      ECAL_SERVICE_LOG_DEBUG(logger_, "[" + get_connection_info_string(socket_) + "] " + "Sending service response...");

      eCAL::service::ProtocolV1::async_send_payload(socket_, socket_mutex_, header_buffer, response_buffer
                            , [me = shared_from_this()](asio::error_code ec)
                              {
                                const std::string message = "Failed sending service response: " + ec.message();
                                me->logger_(LogLevel::Error, "[" + get_connection_info_string(me->socket_) + "] " + message);

                                me->state_ = State::FAILED;
                                
                                // call event callback
                                me->event_callback_(eCAL::service::ServerEventType::Disconnected, message);
                                me->shutdown_callback_(me);
                              }
                            , [me = shared_from_this()]()
                              {
                                ECAL_SERVICE_LOG_DEBUG_VERBOSE(me->logger_, "[" + get_connection_info_string(me->socket_) + "] " + "Successfully sent service response.");

                                // Wait for next request
                                me->receive_service_request();
                              });
    }

  } // namespace service
} // namespace eCAL
