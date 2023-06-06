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

#include "ecal_service_client_session_impl_v1.h"

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
      , service_call_queue_strand_(io_context_)
      , logger_                   (logger)
      , accepted_protocol_version_(0)
      , state_                    (State::NOT_CONNECTED)
      , service_call_in_progress_ (false)
    {
  #if (ECAL_ASIO_TCP_SERVER_LOG_DEBUG_VERBOSE_ENABLED)
      const auto message = get_log_string("DEBUG", "Created");
      std::cout << message << std::endl;
  #endif

      logger_(LogLevel::DebugVerbose, "Created");
    }

    ClientSessionV1::~ClientSessionV1()
    {
      logger_(LogLevel::DebugVerbose, "Deleted");
    }

    //////////////////////////////////////
    // Connection establishement
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

    void ClientSessionV1::send_protocol_handshake_request()
    {
  #if (TCP_PUBSUB_LOG_DEBUG_ENABLED)
      // TODO: Make actual logging
        log_(logger::LogLevel::Debug,  "SubscriberSession " + endpointToString() + ": Sending ProtocolHandshakeRequest.");
  #endif

        logger_(LogLevel::DebugVerbose, "[" + get_connection_info_string(socket_) + "] " + "Sending protocol handshake request...");

        // Go to handshake state
        {
          std::lock_guard<std::mutex> lock(service_state_mutex_);
          state_ = State::HANDSHAKE;
        }

        // Create buffers
        const std::shared_ptr<TcpHeader>  header_buffer  = std::make_shared<TcpHeader>();
        const std::shared_ptr<std::string> payload_buffer = std::make_shared<std::string>();

        // Fill Handshake Request Message
        payload_buffer->resize(sizeof(ProtocolHandshakeRequestMessage), '\0');
        ProtocolHandshakeRequestMessage* handshake_request_message = reinterpret_cast<ProtocolHandshakeRequestMessage*>(const_cast<char*>(payload_buffer->data()));
        handshake_request_message->min_supported_protocol_version = 1;
        handshake_request_message->max_supported_protocol_version = 1;

        // Fill TCP Header
        header_buffer->package_size_n = htonl(sizeof(ProtocolHandshakeRequestMessage));
        header_buffer->version        = 1;
        header_buffer->message_type   = MessageType::ProtocolHandshakeRequest;
        header_buffer->header_size_n  = htons(sizeof(TcpHeader));

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
                                TcpHeader* header = reinterpret_cast<TcpHeader*>(header_buffer->data());
                                if (header->message_type != eCAL::service::MessageType::ProtocolHandshakeResponse)
                                {
                                  // The response is not a Handshake response.
                                  const std::string message = "Received invalid handshake response from server. Expected message type " 
                                                              + std::to_string(static_cast<std::uint8_t>(eCAL::service::MessageType::ProtocolHandshakeResponse)) 
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
                                  if (payload_buffer->size() < sizeof(ProtocolHandshakeResponseMessage))
                                  {
                                    payload_buffer->resize(sizeof(ProtocolHandshakeResponseMessage), '\0');
                                  }
                                  const ProtocolHandshakeResponseMessage* handshake_response = reinterpret_cast<const ProtocolHandshakeResponseMessage*>(payload_buffer->data());

                                  if ((handshake_response->accepted_protocol_version >= MIN_SUPPORTED_PROTOCOL_VERSION)
                                    && (handshake_response->accepted_protocol_version <= MIN_SUPPORTED_PROTOCOL_VERSION))
                                  {
                                    {
                                      std::lock_guard<std::mutex> lock(me->service_state_mutex_);
                                      me->accepted_protocol_version_ = handshake_response->accepted_protocol_version;
                                      me->state_ = State::CONNECTED;
                                    }

                                    const std::string message = "Connected to server. Using protocol version " + std::to_string(me->accepted_protocol_version_);
                                    me->logger_(LogLevel::Debug, "[" + get_connection_info_string(me->socket_) + "] " + message);

                                    // Call event callback
                                    me->event_callback_(eCAL_Client_Event::client_event_connected, message);

                                    // Start sending service requests, if there are any
                                    {
                                      std::lock_guard<std::mutex> lock(me->service_state_mutex_);
                                      if (!me->service_call_queue_.empty())
                                      {
                                        // If there are service calls in the queue, we send the next one.
                                        me->service_call_in_progress_ = true;
                                        me->send_next_service_request(std::move(me->service_call_queue_.front().request), std::move(me->service_call_queue_.front().response_cb));
                                        me->service_call_queue_.pop_front();
                                      }
                                      else
                                      {
                                        // If there are no more service calls to send, we go to error-peeking.
                                        // While error peeking we basically do nothing, except from non-destructively
                                        // reading 1 byte from the socket (i.e. without removing it from the socket).
                                        // This will cause asio / the OS to notify us, when the server closed the connection.

                                        me->service_call_in_progress_ = false;
                                        me->peek_for_error();
                                      }
                                    }

                                    return;
                                  }
                                  else
                                  {
                                    const std::string message = "Error connecting to server. Server reported an un-supported protocol version: " + std::to_string(handshake_response->accepted_protocol_version);
                                    me->logger_(LogLevel::Error, "[" + get_connection_info_string(me->socket_) + "] " + message);

                                    me->handle_connection_loss_error(message);
                                    return;
                                  }

                                }
                              }));
    }

    //////////////////////////////////////
    // Service calls
    //////////////////////////////////////

    void ClientSessionV1::async_call_service(const std::shared_ptr<const std::string>& request, const ResponseCallbackT& response_callback)
    {
      service_call_queue_strand_.post([me = shared_from_this(), request, response_callback]()
                            {
                              // Variable that enables us to unlock the mutex before actually calling the callback
                              bool call_response_callback_with_error(false);
                              
                              {
                                std::lock_guard<std::mutex> lock(me->service_state_mutex_);
                                if (me->state_ != State::FAILED)
                                {
                                  // If we are  not in failed state, let's check
                                  // whether we directly invoke the call of if we add it to the queue

                                  if (!me->service_call_in_progress_ && (me->state_ == State::CONNECTED))
                                  {
                                    // Directly call the the service, iff
                                    // 
                                    //  - There is no call in progress
                                    //  
                                    //  and
                                    // 
                                    //  - We are connected
                                    // 
                                    me->service_call_in_progress_ = true;
                                    me->send_next_service_request(request, response_callback);
                                  }
                                  else
                                  {
                                    // Add the call to the queue, iff:
                                    // 
                                    //  - A call is already in progress
                                    // 
                                    //  or
                                    // 
                                    //  - We are not connected, yet
                                    //
                                    me->service_call_queue_.push_back(ServiceCall{request, response_callback});
                                  }
                                }
                                else
                                {
                                  // If we are in FAILED state, we directly call the callback with an error.
                                  call_response_callback_with_error = true;
                                }
                              }

                              if(call_response_callback_with_error)
                              {
                                // If we are in FAILED state, we directly call the callback with an error.
                                // The mutex is unlocked at this point. That is important, as we have no
                                // influence on when the callback will return.
                                response_callback(eCAL::service::Error::ErrorCode::CONNECTION_CLOSED, nullptr);
                              }
                            });
    }

    void ClientSessionV1::send_next_service_request(const std::shared_ptr<const std::string>& request, const ResponseCallbackT& response_cb)
    {
      logger_(LogLevel::DebugVerbose, "[" + get_connection_info_string(socket_) + "] " + "Sending service request...");

      // Create header_buffer
      const std::shared_ptr<TcpHeader>  header_buffer  = std::make_shared<TcpHeader>();
      header_buffer->package_size_n = htonl(static_cast<std::uint32_t>(request->size()));
      header_buffer->version        = accepted_protocol_version_;
      header_buffer->message_type   = MessageType::ServiceRequest;
      header_buffer->header_size_n  = htons(sizeof(TcpHeader));

      eCAL::service::ProtocolV1::async_send_payload(socket_, header_buffer, request
                              , service_call_queue_strand_.wrap([me = shared_from_this(), response_cb](asio::error_code ec)
                                {
                                  const std::string message = "Failed sending service request: " + ec.message();
                                  me->logger_(LogLevel::Error, "[" + get_connection_info_string(me->socket_) + "] " + message);

                                  // Call the callback with an error
                                  response_cb(Error(Error::ErrorCode::CONNECTION_CLOSED, message), nullptr);
                                  
                                  // Further handle the error, e.g. unwinding pending service calls and calling the event callback
                                  me->handle_connection_loss_error(message);
                                })
                              , [me = shared_from_this(), response_cb]()
                                {
                                  me->logger_(LogLevel::DebugVerbose, "[" + get_connection_info_string(me->socket_) + "] " + "Successfully sent service request.");
                                  me->receive_service_response(response_cb);
                                });
    }

    void ClientSessionV1::receive_service_response(const ResponseCallbackT& response_cb)
    {
      logger_(LogLevel::DebugVerbose, "[" + get_connection_info_string(socket_) + "] " + "Waiting for service response...");

      eCAL::service::ProtocolV1::async_receive_payload(socket_
                            , service_call_queue_strand_.wrap([me = shared_from_this(), response_cb](asio::error_code ec)
                              {
                                const std::string message = "Failed receiving service response: " + ec.message();
                                me->logger_(LogLevel::Error, "[" + get_connection_info_string(me->socket_) + "] " + message);

                                // Call the callback with an error
                                response_cb(Error(Error::ErrorCode::CONNECTION_CLOSED, message), nullptr);

                                // Further handle the error, e.g. unwinding pending service calls and calling the event callback
                                me->handle_connection_loss_error(message);
                              })
                            , service_call_queue_strand_.wrap([me = shared_from_this(), response_cb](const std::shared_ptr<std::vector<char>>& header_buffer, const std::shared_ptr<std::string>& payload_buffer)
                              {
                                TcpHeader* header = reinterpret_cast<TcpHeader*>(header_buffer->data());
                                if (header->message_type != eCAL::service::MessageType::ServiceResponse)
                                {
                                  const std::string message = "Received invalid service response from server. Expected message type " 
                                                              + std::to_string(static_cast<std::uint8_t>(eCAL::service::MessageType::ServiceResponse)) 
                                                              + ", but received " + std::to_string(static_cast<std::uint8_t>(header->message_type));
                                  me->logger_(LogLevel::Error, "[" + get_connection_info_string(me->socket_) + "] " + message);

                                  // Call the callback with an error
                                  response_cb(Error(Error::ErrorCode::PROTOCOL_ERROR, message), nullptr);

                                  // Further handle the error, e.g. unwinding pending service calls and calling the event callback
                                  me->handle_connection_loss_error(message);
                                  return;
                                }
                                else
                                {
                                  // The response is a Service response
                                  me->logger_(LogLevel::DebugVerbose, "[" + get_connection_info_string(me->socket_) + "] " + "Successfully received service response of " + std::to_string(payload_buffer->size()) + " bytes");

                                  // Call the user's callback
                                  // TODO: Currently, this is synchronously. There is potential to execute those in parallel, while already receiving the next data!
                                  response_cb(Error::OK, payload_buffer);

                                  // Check if there are more items in the queue. If so, send the next request
                                  // The mutex must be locket, as we access the queue.
                                  {
                                    std::lock_guard<std::mutex> lock(me->service_state_mutex_);

                                    if (!me->service_call_queue_.empty())
                                    {
                                      // If there are more items, continue calling the service
                                      me->service_call_in_progress_ = true;
                                      me->send_next_service_request(std::move(me->service_call_queue_.front().request), std::move(me->service_call_queue_.front().response_cb));
                                      me->service_call_queue_.pop_front();
                                    }
                                    else
                                    {
                                      // If there are no more service calls to send, we go to error-peeking.
                                      // While error peeking we basically do nothing, except from non-destructively
                                      // reading 1 byte from the socket (i.e. without removing it from the socket).
                                      // This will cause asio / the OS to notify us, when the server closed the connection.

                                      me->service_call_in_progress_ = false;
                                      me->peek_for_error();
                                    }
                                  }
                                }
                              }));


    }

    //////////////////////////////////////
    // Status API
    //////////////////////////////////////
    State ClientSessionV1::get_state() const
    {
      std::lock_guard<std::mutex> lock(service_state_mutex_);
      return state_;
    }

    std::uint8_t ClientSessionV1::get_accepted_protocol_version() const
    {
      return accepted_protocol_version_;
    }

    int ClientSessionV1::get_queue_size() const
    {
      std::lock_guard<std::mutex> lock(service_state_mutex_);
      return static_cast<int>(service_call_queue_.size());
    }

    //////////////////////////////////////
    // Shutdown
    //////////////////////////////////////
    void ClientSessionV1::peek_for_error()
    {
      std::shared_ptr<std::vector<char>> peek_buffer = std::make_shared<std::vector<char>>(1, '\0');

      socket_.async_receive(asio::buffer(*peek_buffer)
                            , asio::socket_base::message_peek
                            , service_call_queue_strand_.wrap([me = shared_from_this(), peek_buffer](const asio::error_code& ec, std::size_t /*bytes_transferred*/) {
                              if (ec)
                              {
                                const std::string message = "Connection loss while idling: " + ec.message();
                                me->logger_(LogLevel::Error, "[" + get_connection_info_string(me->socket_) + "] " + message);
                                me->handle_connection_loss_error("Connection loss while idling: " + ec.message());
                              }
                            }));
    }

    void ClientSessionV1::handle_connection_loss_error(const std::string& error_message)
    {
      bool call_event_callback (false); // Variable that enables us to unlock the mutex before we execute the event callback.

      {
        std::lock_guard<std::mutex> lock(service_state_mutex_);

        // cancel the connection loss handling, if we are already in FAILED state
        if (state_ == State::FAILED)
          return;

        if (state_ == State::CONNECTED)
        {
          // Event callback
          call_event_callback = true;
        }

        // Set the state to FAILED
        state_ = State::FAILED;

        // call all callbacks from the queue with an error
        if (!service_call_queue_.empty())
        {
          // TODO: Add debug log here
          call_all_callbacks_with_error();
        }
      }

      if (call_event_callback)
      {
        event_callback_(eCAL_Client_Event::client_event_disconnected, error_message);
      }
    }

    void ClientSessionV1::call_all_callbacks_with_error()
    {
      service_call_queue_strand_.post([me = shared_from_this()]()
                                      {
                                        ServiceCall first_service_call;
                                        bool        more_service_calls(false);

                                        {
                                          // Lock the mutex and manipulate the queue. We want the mutex unlocked for the event callback call.
                                          std::lock_guard<std::mutex> lock(me->service_state_mutex_);
                                          
                                          if (me->service_call_queue_.empty())
                                            return;

                                          first_service_call = std::move(me->service_call_queue_.front());
                                          me->service_call_queue_.pop_front();

                                          more_service_calls = (!me->service_call_queue_.empty());
                                        }

                                        // Execute the callback with an error
                                        first_service_call.response_cb(eCAL::service::Error::ErrorCode::CONNECTION_CLOSED, nullptr); // TODO: I should probably store the error that lead to this somewhere and tell the actual error.

                                        // If there are more sevice calls, call those with an error, as well
                                        if (more_service_calls)
                                          me->call_all_callbacks_with_error();
                                      });
    }

    void ClientSessionV1::stop()
    {
      {
        asio::error_code ec;
        socket_.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
      }

      {
        asio::error_code ec;
        socket_.close(ec);
      }
    }

  }  // namespace service
} // namespace eCAL
