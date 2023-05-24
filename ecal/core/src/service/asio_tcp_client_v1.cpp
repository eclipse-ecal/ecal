///* ========================= eCAL LICENSE =================================
// *
// * Copyright (C) 2016 - 2019 Continental Corporation
// *
// * Licensed under the Apache License, Version 2.0 (the "License");
// * you may not use this file except in compliance with the License.
// * You may obtain a copy of the License at
// * 
// *      http://www.apache.org/licenses/LICENSE-2.0
// * 
// * Unless required by applicable law or agreed to in writing, software
// * distributed under the License is distributed on an "AS IS" BASIS,
// * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// * See the License for the specific language governing permissions and
// * limitations under the License.
// *
// * ========================= eCAL LICENSE =================================
//*/
//
//#include "asio_tcp_client_v1.h"
//
//#include "asio_tcp_server_logging.h"
//#include "ecal_tcpheader.h"
//
//#include <iostream>
//
//namespace eCAL
//{
//  /////////////////////////////////////
//  // Constructor, Destructor, Create
//  /////////////////////////////////////
//  std::shared_ptr<eCAL::CAsioTcpClientV1> CAsioTcpClientV1::create(asio::io_context& io_context_, const std::string& address, std::uint16_t port)
//  {
//    std::shared_ptr<eCAL::CAsioTcpClientV1> asio_tcp_client(new eCAL::CAsioTcpClientV1(io_context_, address, port));
//
//    asio_tcp_client->resolve_endpoint();
//
//    return asio_tcp_client;
//  }
//
//  CAsioTcpClientV1::CAsioTcpClientV1(asio::io_context& io_context_, const std::string& address, std::uint16_t port)
//    : CAsioTcpClient(io_context_)
//    , address_                  (address)
//    , port_                     (port)
//    , resolver_                 (io_context_)
//    , state_                    (State::NOT_CONNECTED)
//    , accepted_protocol_version_(0)
//  {
//#if (ECAL_ASIO_TCP_SERVER_LOG_DEBUG_VERBOSE_ENABLED)
//    const auto message = get_log_string("DEBUG", "Created");
//    std::cout << message << std::endl;
//#endif
//  }
//
//  CAsioTcpClientV1::~CAsioTcpClientV1()
//  {
//#if (ECAL_ASIO_TCP_SERVER_LOG_DEBUG_VERBOSE_ENABLED)
//    const auto message = get_log_string("DEBUG", "Deleted");
//    std::cout << message << std::endl;
//#endif
//  }
//
//  //////////////////////////////////////
//  // Implementation
//  //////////////////////////////////////
//  void CAsioTcpClientV1::resolve_endpoint()
//  {
//#if (ECAL_ASIO_TCP_SERVER_LOG_DEBUG_VERBOSE_ENABLED)
//    const auto message = get_log_string("DEBUG", "Resolving endpoint: " + address_ + ":" + std::to_string(port_));
//    std::cout << message << std::endl;
//#endif
//
//    const asio::ip::tcp::resolver::query query(address_, std::to_string(port_));
//
//    resolver_.async_resolve(query
//                          , [me = shared_from_this()](asio::error_code ec, const asio::ip::tcp::resolver::iterator& resolved_endpoints)
//                          {
//                            if (ec)
//                            {
//#if (ECAL_ASIO_TCP_SERVER_LOG_DEBUG_ENABLED)
//                              // TODO: Decide whether this should always be printed to console
//                              const auto message = me->get_log_string("ERROR", "Error while resolving endpoint " + me->address_ + ":" + std::to_string(me->port_) + ": " + ec.message());
//                              std::cerr << message << std::endl;
//                              // Don't call the callback with "disconnected", as we never told that we are connected
//#endif
//                              me->state_ = State::FAILED;
//                              return;
//                            }
//                            else
//                            {
//                              me->connect_to_endpoint(resolved_endpoints);
//                            }
//                          });
//  }
//  void CAsioTcpClientV1::connect_to_endpoint(const asio::ip::tcp::resolver::iterator& resolved_endpoints)
//  {
//    // Look for the best endpoint to connect to. If possible, we use a loopback
//    // endpoint. Otherwise, we just use the first one.
//
//    auto endpoint_to_connect_to = resolved_endpoints->endpoint(); // Default to first endpoint
//    for (auto it = resolved_endpoints; it != asio::ip::tcp::resolver::iterator(); it++)
//    {
//      if (it->endpoint().address().is_loopback())
//      {
//        // If we find a loopback endpoint we use that one.
//        endpoint_to_connect_to = it->endpoint();
//        break;
//      }
//    }
//
//#if (ECAL_ASIO_TCP_SERVER_LOG_DEBUG_VERBOSE_ENABLED)
//    const auto message = get_log_string("DEBUG", "Endpoint resolved to " + endpoint_to_string(endpoint_to_connect_to) + ". Connecting to endpoint...");
//    std::cout << message << std::endl;
//#endif
//
//    socket_.async_connect(endpoint_to_connect_to
//                            , [me = shared_from_this()](asio::error_code ec)
//                              {
//                                if (ec)
//                                {
//#if (ECAL_ASIO_TCP_SERVER_LOG_DEBUG_ENABLED)
//                                  // TODO: Decide whether error messages should always be printed to console
//                                  const auto message = me->get_log_string("ERROR", "Failed to connect to endpoint: " + ec.message());
//                                  std::cerr << message << std::endl;
//                                  // Don't call the callback with "disconnected", as we never told that we are connected
//#endif
//                                  me->state_ = State::FAILED;
//                                  return;
//                                }
//                                else
//                                {
//#if (ECAL_ASIO_TCP_SERVER_LOG_DEBUG_VERBOSE_ENABLED)
//                                  const auto message = me->get_log_string("DEBUG", "Successfully connected to endpoint.");
//                                  std::cout << message << std::endl;
//#endif
//                                  // Disable Nagle's algorithm. Nagles Algorithm will otherwise cause the
//                                  // Socket to wait for more data, if it encounters a frame that can still
//                                  // fit more data. Obviously, this is an awfull default behaviour, if we
//                                  // want to transmit our data in a timely fashion.
//                                  {
//                                    asio::error_code socket_option_ec;
//                                    me->socket_.set_option(asio::ip::tcp::no_delay(true), socket_option_ec);
//                                    if (socket_option_ec)
//                                    {
//                                      std::cerr << me->get_log_string("ERROR", "Failed setting tcp::no_delay option: " + socket_option_ec.message()) << std::endl;
//                                    }                                      
//                                  }
//
//                                  // Start sending the protocol handshake to the server. This will tell us the actual protocol version.
//                                  me->send_protocol_handshake();
//                                }
//                              });
//  }
//
//  void CAsioTcpClientV1::send_protocol_handshake()
//  {
//#if (TCP_PUBSUB_LOG_DEBUG_ENABLED)
//    // TODO: Make actual logging
//      log_(logger::LogLevel::Debug,  "SubscriberSession " + endpointToString() + ": Sending ProtocolHandshakeRequest.");
//#endif
//
//      const std::shared_ptr<std::vector<char>> buffer = std::make_shared<std::vector<char>>();
//      buffer->resize(sizeof(STcpHeader) + sizeof(STcpProtocolHandshakeRequestMessage));
//
//      // Fill TCP Header
//      STcpHeader* header  = reinterpret_cast<STcpHeader*>(buffer->data());
//      header->package_size_n = htonl(sizeof(STcpProtocolHandshakeRequestMessage));
//      header->version        = 1;
//      header->message_type   = MessageType::ProtocolHandshakeRequest;
//      header->header_size_n  = htons(sizeof(STcpHeader));
//
//      // Fill Handshake Request Message
//      STcpProtocolHandshakeRequestMessage* handshake_request_message = reinterpret_cast<STcpProtocolHandshakeRequestMessage*>(&(buffer->operator[](sizeof(STcpHeader))));
//      handshake_request_message->min_supported_protocol_version = 1;
//      handshake_request_message->max_supported_protocol_version = 1;
//
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
//  }
//
//  void CAsioTcpClientV1::read_header_start()
//  {
//#if (ECAL_ASIO_TCP_SERVER_LOG_DEBUG_VERBOSE_ENABLED)
//    const auto message = get_log_string("DEBUG", "Waiting for header...");
//    std::cout << message << std::endl;
//#endif
//
//    // Get size of the entire header as it is currently known. What comes from
//    // the network may be larger or smaller.
//    constexpr size_t header_size = sizeof(eCAL::STcpHeader);
//
//    // We read the first 8 bytes of the header first. The header does not start
//    // with the header_size, but it contains it at byte 7+8, which is why we
//    // read the first 8 bytes here, do determine the header size.
//    constexpr size_t bytes_to_read_now = 8;
//
//    // Allocate a buffer for the header as we know it. We can make it larger
//    // later, if necessary. We zero the buffer.
//    const std::shared_ptr<std::vector<char>> header_buffer = std::make_shared<std::vector<char>>(header_size, '\0');
//
//    // Receive data from the socket:
//    //   - Maximum "bytes_to_read_now"
//    //   - At least "bytes_to_read_now"
//    // => We read exactly the "bytes_to_read_now" amount of bytes
//    asio::async_read(socket_
//                  , asio::buffer(header_buffer->data(), bytes_to_read_now)
//                  , asio::transfer_at_least(bytes_to_read_now)
//                  , [me = shared_from_this(), header_buffer](asio::error_code ec, std::size_t bytes_read)
//                    {
//                      if (ec)
//                      {
//                        // Call disconnection callback
//                        const auto message = me->get_log_string("Error while reading header start : " + ec.message());
//                        if (me->state_ == State::CONNECTED)
//                        {
//                          me->event_callback_(eCAL_Client_Event::client_event_disconnected, message);
//                        }
//#if (ECAL_ASIO_TCP_SERVER_LOG_DEBUG_ENABLED)
//                        std::cerr << message << std::endl;
//#endif
//                        me->state_ = State::FAILED;
//                        return;
//                      }
//                      
//#if (ECAL_ASIO_TCP_SERVER_LOG_DEBUG_VERBOSE_ENABLED)
//                      const auto message = me->get_log_string("DEBUG", "Received header_start of " + std::to_string(bytes_read) + " bytes.");
//                      std::cout << message << std::endl;
//#endif
//
//                      // Read the rest of the header!
//                      me->read_header_rest(header_buffer, bytes_read);
//                    });
//  }
//
//  void CAsioTcpClientV1::read_header_rest (const std::shared_ptr<std::vector<char>>& header_buffer, size_t bytes_already_read)
//  {
//    // Check how big the remote header claims to be
//    const size_t remote_header_size = ntohs(reinterpret_cast<eCAL::STcpHeader*>(header_buffer->data())->header_size_n);
//
//#if (ECAL_ASIO_TCP_SERVER_LOG_DEBUG_VERBOSE_ENABLED)
//    const auto message = get_log_string("DEBUG", "Waiting for header rest. Entire header should contain " + std::to_string(remote_header_size) + " bytes.");
//    std::cout << message << std::endl;
//#endif
//
//    // Resize the header buffer if necessary. We will not be able to use those
//    // bytes, as we don't know what they mean, but we must receive them anyways.
//    if (remote_header_size > header_buffer->size())
//    {
//      header_buffer->resize(remote_header_size, '\0');
//    }
//
//    // Calculate how many bytes we still need to read until we have received the
//    // entire header
//    const size_t bytes_still_to_read = header_buffer->size() - bytes_already_read;
//
//    // If the header is finished, we directly go over to reading the payload
//    // Note: This should actually never happen, as after the already read
//    // 8 bytes should come at least 8 reserved bytes.
//    if (bytes_still_to_read <= 0)
//    {
//      read_payload(header_buffer);
//      return;
//    }
//
//    // Read the remaining bytes from the header
//    asio::async_read(socket_
//                  , asio::buffer(&((*header_buffer)[bytes_already_read]), bytes_still_to_read)
//                  , asio::transfer_at_least(bytes_still_to_read)
//                  , [me = shared_from_this(), header_buffer](asio::error_code ec, std::size_t bytes_read)
//                    {
//                      if (ec)
//                      {
//                        // Call callback
//                        const auto message = me->get_log_string("Error while reading header rest : " + ec.message());
//                        if (me->state_ == State::CONNECTED)
//                        {
//                          me->event_callback_(eCAL_Client_Event::client_event_disconnected, message);
//                        }
//#if (ECAL_ASIO_TCP_SERVER_LOG_DEBUG_ENABLED)
//                        std::cerr << message << std::endl;
//#endif
//                        me->state_ = State::FAILED;
//                        return;
//                      }
//                      
//#if (ECAL_ASIO_TCP_SERVER_LOG_DEBUG_VERBOSE_ENABLED)
//                      const auto message = me->get_log_string("DEBUG", "Received header_rest of " + std::to_string(bytes_read) + " bytes.");
//                      std::cout << message << std::endl;
//#endif
//
//                      // Start reading the payload!
//                      const uint32_t payload_size = ntohl(reinterpret_cast<eCAL::STcpHeader*>(header_buffer->data())->package_size_n);
//
//                      if (payload_size > 0)
//                      {
//                        // If there is a payload, read it
//                        me->read_payload(header_buffer);
//                      }
//                      else
//                      {
//                        // If there is no payload, directly execute the callback with an empty string
//                        me->handle_payload(header_buffer, std::make_shared<std::string>());
//                      }
//                    });
//  }
//
//  void CAsioTcpClientV1::read_payload(const std::shared_ptr<std::vector<char>>& header_buffer)
//  {
//#if (ECAL_ASIO_TCP_SERVER_LOG_DEBUG_VERBOSE_ENABLED)
//    const auto* header = reinterpret_cast<eCAL::STcpHeader*>(header_buffer->data());
//    std::cout << get_log_string("DEBUG", "The received header contains the followign data: ") << std::endl;
//    std::cout << get_log_string("DEBUG", "  header->package_size_n = " + std::to_string(ntohl(header->package_size_n))) << std::endl;
//    std::cout << get_log_string("DEBUG", "  header->version        = " + std::to_string(header->version))               << std::endl;
//    std::cout << get_log_string("DEBUG", "  header->message_type   = " + std::to_string(static_cast<std::uint8_t>(header->message_type))) << std::endl;
//    std::cout << get_log_string("DEBUG", "  header->header_size_n  = " + std::to_string(ntohs(header->header_size_n)))  << std::endl;
//    // TODO: The reserved field is printed in network byte order, as Win7 compatibility of WinSocks2 does not define 64bit byte swap functions. I didn't want to introduce hacks or implement it myself, just for printing an empty reserved field.
//    std::cout << get_log_string("DEBUG", "  header->reserved       = " + std::to_string(header->reserved))              << std::endl;
//
//    const auto message = get_log_string("DEBUG", "Waiting for payload...");
//    std::cout << message << std::endl;
//#endif
//
//    // Read how many bytes we will get as payload
//    const uint32_t payload_size = ntohl(reinterpret_cast<eCAL::STcpHeader*>(header_buffer->data())->package_size_n);
//
//    // Reserver enough memory for receiving the entire payload. The payload is
//    // represented as an std::string for legacy, reasons. It is not textual data.
//    // 
//    // TODO: Check if I can make it a vector<char>
//    //        TODO: If possible, the also remove the const_cast below
//    const std::shared_ptr<std::string> payload_buffer = std::make_shared<std::string>(payload_size, '\0');
//
//    // Read all the payload data into the payload_buffer
//    asio::async_read(socket_
//                  , asio::buffer(const_cast<char*>(payload_buffer->data()), payload_buffer->size())
//                  , asio::transfer_at_least(payload_buffer->size())
//                  , [me = shared_from_this(), header_buffer, payload_buffer](asio::error_code ec, std::size_t bytes_read)
//                    {
//                      if (ec)
//                      {
//                        // Call callback
//                        const auto message = me->get_log_string("Error while reading payload : " + ec.message());
//                        if (me->state_ == State::CONNECTED)
//                        {
//                          me->event_callback_(eCAL_Client_Event::client_event_disconnected, message);
//                        }
//#if (ECAL_ASIO_TCP_SERVER_LOG_DEBUG_ENABLED)
//                        std::cerr << message << std::endl;
//#endif
//                        return;
//                      }
//                      
//#if (ECAL_ASIO_TCP_SERVER_LOG_DEBUG_VERBOSE_ENABLED)
//                      const auto message = me->get_log_string("DEBUG", "Received payload of " + std::to_string(bytes_read) + " bytes.");
//                      std::cout << message << std::endl;
//#endif
//
//                      // Check what to do with the data
//                      me->handle_payload(header_buffer, payload_buffer);
//                    });
//  }
//
//  void CAsioTcpClientV1::handle_payload(const std::shared_ptr<std::vector<char>>& header_buffer, const std::shared_ptr<std::string>& payload_buffer)
//  {
//    const auto* header = reinterpret_cast<eCAL::STcpHeader*>(header_buffer->data());
//
//    if (state_ == State::HANDSHAKE)
//    {
//      // When in handshake state, we expect a handshake response. Nothing else.
//      if (header->message_type == MessageType::ProtocolHandshakeResponse)
//      {
//        // Resize payload if necessary. Will probably never be necessary
//        if (payload_buffer->size() < sizeof(STcpProtocolHandshakeResponseMessage))
//        {
//          payload_buffer->resize(sizeof(STcpProtocolHandshakeResponseMessage), '\0');
//        }
//        const STcpProtocolHandshakeResponseMessage* handshake_response = reinterpret_cast<const STcpProtocolHandshakeResponseMessage*>(payload_buffer->data());
//
//        if ((handshake_response->accepted_protocol_version >= MIN_SUPPORTED_PROTOCOL_VERSION)
//          && (handshake_response->accepted_protocol_version <= MIN_SUPPORTED_PROTOCOL_VERSION))
//        {
//          accepted_protocol_version_ = handshake_response->accepted_protocol_version;
//          state_ = State::CONNECTED;
//          const std::string message = "Connected to server. Using protocol version " + std::to_string(accepted_protocol_version_);
//          event_callback_(eCAL_Client_Event::client_event_connected, message);
//#if (ECAL_ASIO_TCP_SERVER_LOG_DEBUG_VERBOSE_ENABLED)
//          std::cout << message << std::endl;
//#endif
//          return;
//        }
//        else
//        {
//          const auto message = get_log_string("ERROR", "Error connecting to server. Server reported an un-supported protocol version: " + std::to_string(handshake_response->accepted_protocol_version));
//          std::cerr << message << std::endl;
//          state_ = State::FAILED;
//          return;
//        }
//      }
//    }
//    else if (state_ == State::CONNECTED)
//    {
//      // When in connected state, we can only come here after having sent a service request. Therefore, we expect nothing but a service response.
//      if (header->message_type == MessageType::ServiceResponse)
//      {
//        // TODO: Call callback here
//      }
//    }
//    
//    // If we reach this point, we got a response of an unexpected type. This is an error.
//    const auto message = get_log_string("ERROR", "Error while reading payload : Received unexpected message type " + std::to_string(static_cast<std::uint8_t>(header->message_type)) + " while in handshake state.");
//    std::cerr << message << std::endl;
//    state_ = State::FAILED;
//    return;
//  }
//
//} // namespace eCAL
