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

#include "ecal_service_tcp_session_v1_client.h"

#include "ecal_service_tcp_protocol_v1.h"
#include "ecal_service_log_helpers.h"

#include <iostream>

namespace eCAL
{
  namespace service
  {
    /////////////////////////////////////
    // Constructor, Destructor, Create
    /////////////////////////////////////
    std::shared_ptr<ClientSessionV1> ClientSessionV1::create(asio::io_context&      io_context_
                                                            , const std::string&    address
                                                            , std::uint16_t         port
                                                            , const EventCallbackT& event_callback
                                                            , const LoggerT&        logger)
    {
      std::shared_ptr<ClientSessionV1> instance(new ClientSessionV1(io_context_, event_callback, logger));

      instance->resolve_endpoint(address, port);

      return instance;
    }

    ClientSessionV1::ClientSessionV1(asio::io_context& io_context_, const EventCallbackT& event_callback, const LoggerT& logger)
      : ClientSessionBase(io_context_, event_callback)
      , resolver_                 (io_context_)
      , state_                    (State::NOT_CONNECTED)
      , accepted_protocol_version_(0)
      , service_call_queue_strand_(io_context_)
      , logger_                   (logger)
      , timer_(io_context_, std::chrono::milliseconds(500)) // TODO Remove
    {
  #if (ECAL_ASIO_TCP_SERVER_LOG_DEBUG_VERBOSE_ENABLED)
      const auto message = get_log_string("DEBUG", "Created");
      std::cout << message << std::endl;
  #endif

      logger_(LogLevel::DebugVerbose, "Created");
    }

    ClientSessionV1::~ClientSessionV1()
    {
      {
        asio::error_code ec;
        socket_.close(ec);
      }

      logger_(LogLevel::DebugVerbose, "Deleted");
    }

    //////////////////////////////////////
    // Implementation
    //////////////////////////////////////
    void ClientSessionV1::resolve_endpoint(const std::string& address, std::uint16_t port)
    {
  #if (ECAL_ASIO_TCP_SERVER_LOG_DEBUG_VERBOSE_ENABLED)
      const auto message = get_log_string("DEBUG", "Resolving endpoint: " + address_ + ":" + std::to_string(port_));
      std::cout << message << std::endl;
  #endif

      logger_(LogLevel::DebugVerbose, "Resolving endpoint [" + address + ":" + std::to_string(port) + "]...");


      const asio::ip::tcp::resolver::query query(address, std::to_string(port));

      resolver_.async_resolve(query
                            , service_call_queue_strand_.wrap([me = enable_shared_from_this<ClientSessionV1>::shared_from_this(), address, port]
                              (asio::error_code ec, const asio::ip::tcp::resolver::iterator& resolved_endpoints)
                              {
                                if (ec)
                                {
#if (ECAL_ASIO_TCP_SERVER_LOG_DEBUG_ENABLED)
                                  // TODO: Decide whether this should always be printed to console
                                  const auto message = me->get_log_string("ERROR", "Error while resolving endpoint " + me->address_ + ":" + std::to_string(me->port_) + ": " + ec.message());
                                  std::cerr << message << std::endl;
                                  // Don't call the callback with "disconnected", as we never told that we are connected
#endif
                                  const std::string message = "Failed resolving endpoint [" + address + ":" + std::to_string(port) + "]: " + ec.message();
                                  me->logger_(LogLevel::Error, message);
                                  me->handle_connection_loss_error(message);
                                  return;
                                }
                                else
                                {
                                  me->connect_to_endpoint(resolved_endpoints);
                                }
                              }));
    }

    void ClientSessionV1::connect_to_endpoint(const asio::ip::tcp::resolver::iterator& resolved_endpoints)
    {
      // Look for the best endpoint to connect to. If possible, we use a loopback
      // endpoint. Otherwise, we just use the first one.
      
      logger_(LogLevel::DebugVerbose, "Connecting to endpoint");

      auto endpoint_to_connect_to = resolved_endpoints->endpoint(); // Default to first endpoint
      for (auto it = resolved_endpoints; it != asio::ip::tcp::resolver::iterator(); it++)
      {
        if (it->endpoint().address().is_loopback())
        {
          // If we find a loopback endpoint we use that one.
          endpoint_to_connect_to = it->endpoint();
          break;
        }
      }

      logger_(LogLevel::DebugVerbose, "Successfully resolved endpoint to [" + endpoint_to_string(endpoint_to_connect_to) + "]");

  #if (ECAL_ASIO_TCP_SERVER_LOG_DEBUG_VERBOSE_ENABLED)
      const auto message = get_log_string("DEBUG", "Endpoint resolved to " + endpoint_to_string(endpoint_to_connect_to) + ". Connecting to endpoint...");
      std::cout << message << std::endl;
  #endif

      socket_.async_connect(endpoint_to_connect_to
                              , service_call_queue_strand_.wrap([me = shared_from_this(), endpoint_to_connect_to](asio::error_code ec)
                                {
                                  if (ec)
                                  {
  #if (ECAL_ASIO_TCP_SERVER_LOG_DEBUG_ENABLED)
                                    // TODO: Decide whether error messages should always be printed to console
                                    const auto message = me->get_log_string("ERROR", "Failed to connect to endpoint: " + ec.message());
                                    std::cerr << message << std::endl;
                                    // Don't call the callback with "disconnected", as we never told that we are connected
  #endif
                                    const std::string message = "Failed to connect to endpoint [" + endpoint_to_string(endpoint_to_connect_to) + "]: " + ec.message();
                                    me->logger_(LogLevel::Error, message);
                                    me->handle_connection_loss_error(message);
                                    return;
                                  }
                                  else
                                  {
  #if (ECAL_ASIO_TCP_SERVER_LOG_DEBUG_VERBOSE_ENABLED)
                                    const auto message = me->get_log_string("DEBUG", "Successfully connected to endpoint.");
                                    std::cout << message << std::endl;
  #endif
                                    me->logger_(LogLevel::DebugVerbose, "Successfully connected to endpoint [" + endpoint_to_string(endpoint_to_connect_to) + "]");

                                    // Wait for errors on the socket
                                    //me->wait_for_error(); // TODO: Remove or improve

                                    // Disable Nagle's algorithm. Nagles Algorithm will otherwise cause the
                                    // Socket to wait for more data, if it encounters a frame that can still
                                    // fit more data. Obviously, this is an awfull default behaviour, if we
                                    // want to transmit our data in a timely fashion.
                                    {
                                      asio::error_code socket_option_ec;
                                      me->socket_.set_option(asio::ip::tcp::no_delay(true), socket_option_ec);
                                      if (socket_option_ec)
                                      {
                                        me->logger_(LogLevel::Warning, "[" + get_connection_info_string(me->socket_) + "] " + "Failed setting tcp::no_delay option: " + socket_option_ec.message());
                                      }                                      
                                    }

                                    // Start sending the protocol handshake to the server. This will tell us the actual protocol version.
                                    me->send_protocol_handshake_request();
                                  }
                                }));
    }

    void ClientSessionV1::peek_for_error()
    {
      std::shared_ptr<std::vector<char>> peek_buffer = std::make_shared<std::vector<char>>(1, '\0');

      socket_.async_receive(asio::buffer(*peek_buffer)
                            , asio::socket_base::message_peek
                            , service_call_queue_strand_.wrap([weak_me = std::weak_ptr<ClientSessionV1>(shared_from_this()), peek_buffer](const asio::error_code& ec, std::size_t bytes_transferred) {
                              if (ec)
                              {
                                auto me = weak_me.lock();
                                if (me)
                                {
                                  const std::string message = "Connection loss while idling: " + ec.message();
                                  me->logger_(LogLevel::Error, "[" + get_connection_info_string(me->socket_) + "] " + message);
                                  me->handle_connection_loss_error("Connection loss while idling: " + ec.message());
                                }
                              }
                            }));
    }

    void ClientSessionV1::send_protocol_handshake_request()
    {
  #if (TCP_PUBSUB_LOG_DEBUG_ENABLED)
      // TODO: Make actual logging
        log_(logger::LogLevel::Debug,  "SubscriberSession " + endpointToString() + ": Sending ProtocolHandshakeRequest.");
  #endif

        logger_(LogLevel::DebugVerbose, "[" + get_connection_info_string(socket_) + "] " + "Sending protocol handshake request...");

        // Go to handshake state
        state_ = State::HANDSHAKE;

        // Create buffers
        const std::shared_ptr<STcpHeader>  header_buffer  = std::make_shared<STcpHeader>();
        const std::shared_ptr<std::string> payload_buffer = std::make_shared<std::string>();

        // Fill Handshake Request Message
        payload_buffer->resize(sizeof(STcpProtocolHandshakeRequestMessage), '\0');
        STcpProtocolHandshakeRequestMessage* handshake_request_message = reinterpret_cast<STcpProtocolHandshakeRequestMessage*>(const_cast<char*>(payload_buffer->data()));
        handshake_request_message->min_supported_protocol_version = 1;
        handshake_request_message->max_supported_protocol_version = 1;

        // Fill TCP Header
        header_buffer->package_size_n = htonl(sizeof(STcpProtocolHandshakeRequestMessage));
        header_buffer->version        = 1;
        header_buffer->message_type   = MessageType::ProtocolHandshakeRequest;
        header_buffer->header_size_n  = htons(sizeof(STcpHeader));

        eCAL::service::ProtocolV1::async_send_payload(socket_, header_buffer, payload_buffer
                            , service_call_queue_strand_.wrap([me = shared_from_this()](asio::error_code ec)
                              {
                                const std::string message = "Failed sending protocol handshake request: " + ec.message();
                                me->logger_(LogLevel::Error, "[" + get_connection_info_string(me->socket_) + "] " + message);
                                me->handle_connection_loss_error(message);
                              })
                            , [me = shared_from_this()]()
                              {
                                me->logger_(LogLevel::DebugVerbose, "[" + get_connection_info_string(me->socket_) + "] " + "Successfully sent protocol handshake request.");
                                me->receive_protocol_handshake_response();
                              });

  //      asio::async_write(socket_
  //                      , asio::buffer(*buffer)
  //                      , [me = shared_from_this(), buffer](asio::error_code ec, std::size_t /*bytes_sent*/)
  //                        {
  //                          if (ec)
  //                          {
  //#if (ECAL_ASIO_TCP_SERVER_LOG_DEBUG_ENABLED)
  //                            // TODO: Decide whether error messages should always be printed to console
  //                            const auto message = me->get_log_string("ERROR", "Failed to send protocol handshake request: " + ec.message());
  //                            std::cerr << message << std::endl;
  //                            // Don't call the callback with "disconnected", as we never told that we are connected
  //#endif
  //                            me->state_ = State::FAILED;
  //                            return;
  //                          }
  //                          me->read_header_start();
  //                        });
    }

    void ClientSessionV1::receive_protocol_handshake_response()
    {
      logger_(LogLevel::DebugVerbose, "[" + get_connection_info_string(socket_) + "] " + "Waiting for protocol handshake response...");

      eCAL::service::ProtocolV1::async_receive_payload(socket_
                            , service_call_queue_strand_.wrap([me = shared_from_this()](asio::error_code ec)
                              {
                                const std::string message = "Failed receiving protocol handshake response: " + ec.message();
                                me->logger_(LogLevel::Error, "[" + get_connection_info_string(me->socket_) + "] " + message);
                                me->handle_connection_loss_error(message);
                              })
                            , service_call_queue_strand_.wrap([me = shared_from_this()](const std::shared_ptr<std::vector<char>>& header_buffer, const std::shared_ptr<std::string>& payload_buffer)
                              {
                                STcpHeader* header = reinterpret_cast<STcpHeader*>(header_buffer->data());
                                if (header->message_type != eCAL::MessageType::ProtocolHandshakeResponse)
                                {
                                  // The response is not a Handshake response.
                                  const std::string message = "Received invalid handshake response from server. Expected message type " 
                                                              + std::to_string(static_cast<std::uint8_t>(eCAL::MessageType::ProtocolHandshakeResponse)) 
                                                              + ", but received " + std::to_string(static_cast<std::uint8_t>(header->message_type));
                                  me->logger_(LogLevel::Error, "[" + get_connection_info_string(me->socket_) + "] " + message);

                                  me->handle_connection_loss_error(message);
                                  return;
                                }
                                else
                                {
                                  // The response is a Handshake response
                                  me->logger_(LogLevel::DebugVerbose, "[" + get_connection_info_string(me->socket_) + "] " + "Received a handshake response of " + std::to_string(payload_buffer->size()) + " bytes.");

                                  // Resize payload if necessary. Will probably never be necessary
                                  if (payload_buffer->size() < sizeof(STcpProtocolHandshakeResponseMessage))
                                  {
                                    payload_buffer->resize(sizeof(STcpProtocolHandshakeResponseMessage), '\0');
                                  }
                                  const STcpProtocolHandshakeResponseMessage* handshake_response = reinterpret_cast<const STcpProtocolHandshakeResponseMessage*>(payload_buffer->data());

                                  if ((handshake_response->accepted_protocol_version >= MIN_SUPPORTED_PROTOCOL_VERSION)
                                    && (handshake_response->accepted_protocol_version <= MIN_SUPPORTED_PROTOCOL_VERSION))
                                  {
                                    me->accepted_protocol_version_ = handshake_response->accepted_protocol_version;
                                    me->state_ = State::CONNECTED;

                                    const std::string message = "Connected to server. Using protocol version " + std::to_string(me->accepted_protocol_version_);
                                    me->logger_(LogLevel::Debug, "[" + get_connection_info_string(me->socket_) + "] " + message);

                                    // Call event callback
                                    me->event_callback_(eCAL_Client_Event::client_event_connected, message);

                                    // Start sending service requests, if there are any
                                    if (!me->service_call_queue_.empty())
                                    {
                                      me->send_next_service_request();
                                    }
                                    else
                                    {
                                      // Peek the socket for errors
                                      me->peek_for_error();
                                    }

//                                    const std::string message = "Connected to server. Using protocol version " + std::to_string(accepted_protocol_version_);
//                                    event_callback_(eCAL_Client_Event::client_event_connected, message);
//#if (ECAL_ASIO_TCP_SERVER_LOG_DEBUG_VERBOSE_ENABLED)
//                                    std::cout << message << std::endl;
//#endif

                                    return;
                                  }
                                  else
                                  {
                                    //const auto message = get_log_string("ERROR", "Error connecting to server. Server reported an un-supported protocol version: " + std::to_string(handshake_response->accepted_protocol_version));
                                    //std::cerr << message << std::endl;
                                    const std::string message = "Error connecting to server. Server reported an un-supported protocol version: " + std::to_string(handshake_response->accepted_protocol_version);
                                    me->logger_(LogLevel::Error, "[" + get_connection_info_string(me->socket_) + "] " + message);

                                    me->handle_connection_loss_error(message);
                                    return;
                                  }

                                }
                              }));
    }

    void ClientSessionV1::handle_connection_loss_error(const std::string& error_message)
    {
      // cancel the connection loss handling, if we are already in FAILED state
      if (state_ == State::FAILED)
        return;

      if (state_ == State::CONNECTED)
      {
        // Call callback
        event_callback_(eCAL_Client_Event::client_event_disconnected, error_message);
      }

      // TODO: execute all the service callbacks from the queue with an error

      state_ = State::FAILED;
    }

    void ClientSessionV1::async_call_service(const std::shared_ptr<std::string>& request, const ResponseCallbackT& response_callback)
    {
      service_call_queue_strand_.post([me = shared_from_this(), request, response_callback]()
                            {
                              const bool write_in_progress = !me->service_call_queue_.empty();
                              me->service_call_queue_.push_back(ServiceCall{request, response_callback});
                              if (!write_in_progress && (me->state_ == State::CONNECTED))
                              {
                                // TODO: The parameters need to be removed
                                me->send_next_service_request();
                              }
                            });
    }

    void ClientSessionV1::send_next_service_request()
    {
      // TODO: Create a function that just adds requests to a queue, so they will be sent one after another
      logger_(LogLevel::DebugVerbose, "[" + get_connection_info_string(socket_) + "] " + "Sending service request...");

      // Create header_buffer
      const std::shared_ptr<STcpHeader>  header_buffer  = std::make_shared<STcpHeader>();
      header_buffer->package_size_n = htonl(service_call_queue_.front().request->size());
      header_buffer->version        = accepted_protocol_version_;
      header_buffer->message_type   = MessageType::ServiceRequest;
      header_buffer->header_size_n  = htons(sizeof(STcpHeader));

      eCAL::service::ProtocolV1::async_send_payload(socket_, header_buffer, service_call_queue_.front().request
                              , service_call_queue_strand_.wrap([me = shared_from_this()](asio::error_code ec)
                                {
                                  const std::string message = "Failed sending service request: " + ec.message();
                                  me->logger_(LogLevel::Error, "[" + get_connection_info_string(me->socket_) + "] " + message);

                                  me->handle_connection_loss_error(message);
                                })
                              , [me = shared_from_this()]()
                                {
                                  me->logger_(LogLevel::DebugVerbose, "[" + get_connection_info_string(me->socket_) + "] " + "Successfully sent service request.");
                                  me->receive_service_response();
                                });
    }

    void ClientSessionV1::receive_service_response()
    {
      logger_(LogLevel::DebugVerbose, "[" + get_connection_info_string(socket_) + "] " + "Waiting for service response...");

      eCAL::service::ProtocolV1::async_receive_payload(socket_
                            , service_call_queue_strand_.wrap([me = shared_from_this()](asio::error_code ec)
                              {
                                const std::string message = "Failed receiving service response: " + ec.message();
                                me->logger_(LogLevel::Error, "[" + get_connection_info_string(me->socket_) + "] " + message);
                                me->handle_connection_loss_error(message);
                              })
                            , service_call_queue_strand_.wrap([me = shared_from_this()](const std::shared_ptr<std::vector<char>>& header_buffer, const std::shared_ptr<std::string>& payload_buffer)
                              {
                                STcpHeader* header = reinterpret_cast<STcpHeader*>(header_buffer->data());
                                if (header->message_type != eCAL::MessageType::ServiceResponse)
                                {
                                  const std::string message = "Received invalid service response from server. Expected message type " 
                                                              + std::to_string(static_cast<std::uint8_t>(eCAL::MessageType::ServiceResponse)) 
                                                              + ", but received " + std::to_string(static_cast<std::uint8_t>(header->message_type));
                                  me->logger_(LogLevel::Error, "[" + get_connection_info_string(me->socket_) + "] " + message);

                                  me->handle_connection_loss_error(message);

                                  return;
                                }
                                else
                                {
                                  // The response is a Service response
                                  me->logger_(LogLevel::DebugVerbose, "[" + get_connection_info_string(me->socket_) + "] " + "Successfully received service response of " + std::to_string(payload_buffer->size()) + " bytes");

                                  // Call the user's callback
                                  // TODO: Currently, this is synchronously. There is potential to execute those in parallel, while already receiving the next data!
                                  me->service_call_queue_.front().response_cb(Error::OK, payload_buffer);

                                  // Remove the service call item from the queue
                                  me->service_call_queue_.pop_front();

                                  if (!me->service_call_queue_.empty())
                                  {
                                    // If there are more items, continue calling the service
                                    me->send_next_service_request();
                                  }
                                  else
                                  {
                                    // Otherwise, just check if there are errors
                                    me->peek_for_error();
                                  }
                                }
                              }));


    }

  }  // namespace service
} // namespace eCAL
