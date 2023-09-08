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

#include "protocol_v0.h"

#include "protocol_layout.h"

namespace eCAL
{
  namespace service
  {
    namespace ProtocolV0
    {
      namespace
      {
        void read_header(asio::ip::tcp::socket& socket, std::mutex& socket_mutex, const ErrorCallbackT& error_cb, const ReceiveSuccessCallback& success_cb);
        void read_payload(asio::ip::tcp::socket& socket, std::mutex& socket_mutex, const std::shared_ptr<eCAL::service::TcpHeaderV0>& header_buffer, const ErrorCallbackT& error_cb, const ReceiveSuccessCallback& success_cb);

        ///////////////////////////////////////////////////
        // Read and write implementation
        ///////////////////////////////////////////////////
        void read_header(asio::ip::tcp::socket& socket, std::mutex& socket_mutex, const ErrorCallbackT& error_cb, const ReceiveSuccessCallback& success_cb)
        {
          // Allocate a buffer for the header as we know it. We can make it larger
          // later, if necessary. We zero the buffer.
          const auto header_buffer = std::make_shared<eCAL::service::TcpHeaderV0>();
          constexpr size_t header_buffer_size = sizeof(eCAL::service::TcpHeaderV0);

          // Receive data from the socket:
          //   - Maximum "bytes_to_read_now"
          //   - At least "bytes_to_read_now"
          // => We read exactly the "bytes_to_read_now" amount of bytes
          const std::lock_guard<std::mutex> socket_lock(socket_mutex);
          asio::async_read(socket
                        , asio::buffer(reinterpret_cast<char*>(header_buffer.get()), header_buffer_size)
                        , asio::transfer_at_least(header_buffer_size)
                        , [&socket, &socket_mutex, header_buffer, error_cb, success_cb](asio::error_code ec, std::size_t /*bytes_read*/)
                          {
                            if (ec)
                            {
                              // Call error callback
                              error_cb(ec);
                              return;
                            }

                            // Read the payload that comes after the header
                            read_payload(socket, socket_mutex, header_buffer, error_cb, success_cb);
                          });
        }

        void read_payload(asio::ip::tcp::socket& socket, std::mutex& socket_mutex, const std::shared_ptr<eCAL::service::TcpHeaderV0>& header_buffer, const ErrorCallbackT& error_cb, const ReceiveSuccessCallback& success_cb)
        {
          // Read how many bytes we will get as payload
          const uint32_t payload_size = ntohl(header_buffer->package_size_n);

          // Reserver enough memory for receiving the entire payload. The payload is
          // represented as an std::string for legacy, reasons. It is not textual data.
          const std::shared_ptr<std::string> payload_buffer = std::make_shared<std::string>(payload_size, '\0');

          // Read all the payload data into the payload_buffer
          const std::lock_guard<std::mutex> socket_lock(socket_mutex);
          asio::async_read(socket
                        , asio::buffer(const_cast<char*>(payload_buffer->data()), payload_buffer->size())
                        , asio::transfer_at_least(payload_buffer->size())
                        , [header_buffer, payload_buffer, error_cb, success_cb](asio::error_code ec, std::size_t /*bytes_read*/)
                          {
                            if (ec)
                            {
                              // Call error callback
                              error_cb(ec);
                              return;
                            }
                      
                            // Call success callback
                            success_cb(header_buffer, payload_buffer);
                          });

        }
      }

      ///////////////////////////////////////////////////
      // Public API
      ///////////////////////////////////////////////////
      void async_send_payload_with_header(asio::ip::tcp::socket& socket, std::mutex& socket_mutex, const std::shared_ptr<const eCAL::service::TcpHeaderV0>& header_buffer, const std::shared_ptr<const std::string>& payload_buffer, const ErrorCallbackT& error_cb, const SendSuccessCallback& success_cb)
      {        
        const std::vector<asio::const_buffer> buffer_list { asio::buffer(reinterpret_cast<const char*>(header_buffer.get()), sizeof(eCAL::service::TcpHeaderV0))
                                                          , asio::buffer(*payload_buffer)};

        const std::lock_guard<std::mutex> socket_lock(socket_mutex);
        asio::async_write(socket
                        , buffer_list
                        , [header_buffer, payload_buffer, error_cb, success_cb](asio::error_code ec, std::size_t /*bytes_sent*/)
                          {
                            if (ec)
                            {
                              // Call error callback
                              error_cb(ec);
                              return;
                            }
                            success_cb();
                          });

      }

      void async_receive_payload_with_header(asio::ip::tcp::socket& socket, std::mutex& socket_mutex, const ErrorCallbackT& error_cb, const ReceiveSuccessCallback& success_cb)
      {
        read_header(socket, socket_mutex, error_cb, success_cb);
      }
    } // namespace ProtocolV1
  } // namespace service
} // namespace eCAL
