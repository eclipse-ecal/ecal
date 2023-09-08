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

#include "protocol_v1.h"

#include "protocol_layout.h"

namespace eCAL
{
  namespace service
  {
    namespace ProtocolV1
    {
      namespace
      {
        void read_header_start(asio::ip::tcp::socket& socket, std::mutex& socket_mutex, const ErrorCallbackT& error_cb, const ReceiveSuccessCallback& success_cb);
        void read_header_rest(asio::ip::tcp::socket& socket, std::mutex& socket_mutex, const std::shared_ptr<std::vector<char>>& header_buffer, size_t bytes_already_read, const ErrorCallbackT& error_cb, const ReceiveSuccessCallback& success_cb);
        void read_payload(asio::ip::tcp::socket& socket, std::mutex& socket_mutex, const std::shared_ptr<std::vector<char>>& header_buffer, const ErrorCallbackT& error_cb, const ReceiveSuccessCallback& success_cb);

        ///////////////////////////////////////////////////
        // Read and write implementation
        ///////////////////////////////////////////////////
        void read_header_start(asio::ip::tcp::socket& socket, std::mutex& socket_mutex, const ErrorCallbackT& error_cb, const ReceiveSuccessCallback& success_cb)
        {
          // Get size of the entire header as it is currently known. What comes from
          // the network may be larger or smaller.
          constexpr size_t header_size = sizeof(eCAL::service::TcpHeaderV1);

          // We read the first 8 bytes of the header first. The header does not start
          // with the header_size, but it contains it at byte 7+8, which is why we
          // read the first 8 bytes here, do determine the header size.
          constexpr size_t bytes_to_read_now = 8;

          // Allocate a buffer for the header as we know it. We can make it larger
          // later, if necessary. We zero the buffer.
          const std::shared_ptr<std::vector<char>> header_buffer = std::make_shared<std::vector<char>>(header_size, '\0');

          // Receive data from the socket:
          //   - Maximum "bytes_to_read_now"
          //   - At least "bytes_to_read_now"
          // => We read exactly the "bytes_to_read_now" amount of bytes
          const std::lock_guard<std::mutex> socket_lock(socket_mutex);
          asio::async_read(socket
                        , asio::buffer(header_buffer->data(), bytes_to_read_now)
                        , asio::transfer_at_least(bytes_to_read_now)
                        , [&socket, &socket_mutex, header_buffer, error_cb, success_cb](asio::error_code ec, std::size_t bytes_read)
                          {
                            if (ec)
                            {
                              // Call error callback
                              error_cb(ec);
                              return;
                            }

                            // Read the rest of the header!
                            read_header_rest(socket, socket_mutex, header_buffer, bytes_read, error_cb, success_cb);
                          });
        }

        void read_header_rest(asio::ip::tcp::socket& socket, std::mutex& socket_mutex, const std::shared_ptr<std::vector<char>>& header_buffer, size_t bytes_already_read, const ErrorCallbackT& error_cb, const ReceiveSuccessCallback& success_cb)
        {
          // Check how big the remote header claims to be
          const size_t remote_header_size = ntohs(reinterpret_cast<eCAL::service::TcpHeaderV1*>(header_buffer->data())->header_size_n);

          // Resize the header buffer if necessary. We will not be able to use those
          // bytes, as we don't know what they mean, but we must receive them anyways.
          if (remote_header_size > header_buffer->size())
          {
            header_buffer->resize(remote_header_size, '\0');
          }

          // Calculate how many bytes we still need to read until we have received the
          // entire header
          const size_t bytes_still_to_read = header_buffer->size() - bytes_already_read;

          // If the header is finished, we directly go over to reading the payload
          // Note: This should actually never happen, as after the already read
          // 8 bytes should come at least 8 reserved bytes.
          if (bytes_still_to_read <= 0)
          {
            read_payload(socket, socket_mutex, header_buffer, error_cb, success_cb);
            return;
          }

          // Read the remaining bytes from the header
          const std::lock_guard<std::mutex> socket_lock(socket_mutex);
          asio::async_read(socket
                        , asio::buffer(&((*header_buffer)[bytes_already_read]), bytes_still_to_read)
                        , asio::transfer_at_least(bytes_still_to_read)
                        , [&socket, &socket_mutex, header_buffer, error_cb, success_cb](asio::error_code ec, std::size_t /*bytes_read*/)
                          {
                            if (ec)
                            {
                              // Call callback
                              error_cb(ec);
                              return;
                            }

                            // Start reading the payload!
                            const uint32_t payload_size = ntohl(reinterpret_cast<eCAL::service::TcpHeaderV1*>(header_buffer->data())->package_size_n);

                            if (payload_size > 0)
                            {
                              // If there is a payload, read it
                              read_payload(socket, socket_mutex, header_buffer, error_cb, success_cb);
                            }
                            else
                            {
                              // If there is no payload, directly execute the callback with an empty string
                              success_cb(header_buffer, std::make_shared<std::string>());
                            }
                          });
        }

        void read_payload(asio::ip::tcp::socket& socket, std::mutex& socket_mutex, const std::shared_ptr<std::vector<char>>& header_buffer, const ErrorCallbackT& error_cb, const ReceiveSuccessCallback& success_cb)
        {
          // Read how many bytes we will get as payload
          const uint32_t payload_size = ntohl(reinterpret_cast<eCAL::service::TcpHeaderV1*>(header_buffer->data())->package_size_n);

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
      void async_send_payload   (asio::ip::tcp::socket& socket, std::mutex& socket_mutex, const std::shared_ptr<const eCAL::service::TcpHeaderV1>& header_buffer, const std::shared_ptr<const std::string>& payload_buffer, const ErrorCallbackT& error_cb, const SendSuccessCallback& success_cb)
      {        
        const std::vector<asio::const_buffer> buffer_list { asio::buffer(reinterpret_cast<const char*>(header_buffer.get()), sizeof(eCAL::service::TcpHeaderV1))
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

      void async_receive_payload(asio::ip::tcp::socket& socket, std::mutex& socket_mutex, const ErrorCallbackT& error_cb, const ReceiveSuccessCallback& success_cb)
      {
        read_header_start(socket, socket_mutex, error_cb, success_cb);
      }
    } // namespace ProtocolV1
  } // namespace service
} // namespace eCAL
