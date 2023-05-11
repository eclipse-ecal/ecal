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

#include "asio_server_session_v1.h"

#include "ecal_tcpheader.h"

#include  <iostream> //TODO: Remove

namespace eCAL
{
  // TODO: I should probably introduce a data_strand, just in case

  ///////////////////////////////////////////////
  // Create, Constructor, Destructor
  ///////////////////////////////////////////////

  std::shared_ptr<CAsioServerSessionV1> CAsioServerSessionV1::create(asio::io_context& io_context_, const RequestCallbackT& request_callback, const EventCallbackT& event_callback)
  {
    return std::shared_ptr<CAsioServerSessionV1>(new CAsioServerSessionV1(io_context_, request_callback, event_callback));
  }

  // TODO: Call event_callback whenever something happens (e.g. a deletion)
  CAsioServerSessionV1::CAsioServerSessionV1(asio::io_context& io_context_, const RequestCallbackT& request_callback, const EventCallbackT& event_callback)
    : CAsioServerSession(io_context_, request_callback, event_callback)
  {}

  ///////////////////////////////////////////////
  // Data receiving
  ///////////////////////////////////////////////
  void CAsioServerSessionV1::start()
  {
    std::cout << "CAsioServerSessionV1::start()" << std::endl; // TODO: remove

    read_request_header_start();
  }

  void CAsioServerSessionV1::read_request_header_start()
  {
    // Get size of the entire header as it is currently known. What comes from
    // the network may be larger or smaller.
    constexpr size_t header_size = sizeof(eCAL::STcpHeader);

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
    asio::async_read(socket_
                  , asio::buffer(header_buffer->data(), bytes_to_read_now)
                  , asio::transfer_at_least(bytes_to_read_now)
                  , [me = shared_from_this(), header_buffer](asio::error_code ec, std::size_t bytes_read)
                    {
                      if (ec)
                      {
                        std::cerr << "CAsioServerSessionV1: Error while reading header start: " << ec.message() << std::endl; // TODO: Make logging message
                        return;
                      }
                      
                      std::cout << "read_header_start: read " << bytes_read << " bytes" << std::endl; // TODO: Remove

                      // Read the rest of the header!
                      me->read_request_header_rest(header_buffer, bytes_read);
                    });
  }

  void CAsioServerSessionV1::read_request_header_rest (const std::shared_ptr<std::vector<char>>& header_buffer, size_t bytes_already_read)
  {
    // Check how big the remote header claims to be
    const size_t remote_header_size = ntohs(reinterpret_cast<eCAL::STcpHeader*>(header_buffer->data())->header_size_n);

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
      read_request_payload(header_buffer);
      return;
    }

    // Read the remaining bytes from the header
    asio::async_read(socket_
                  , asio::buffer(&((*header_buffer)[bytes_already_read]), bytes_still_to_read)
                  , asio::transfer_at_least(bytes_still_to_read)
                  , [me = shared_from_this(), header_buffer](asio::error_code ec, std::size_t bytes_read)
                    {
                      if (ec)
                      {
                        std::cerr << "CAsioServerSessionV1: Error while reading header rest: " << ec.message() << std::endl; // TODO: Make logging message
                        return;
                      }
                      
                      std::cout << "read_header_rest: read " << bytes_read << " bytes" << std::endl; // TODO: Remove

                      // Start reading the payload!
                      const uint32_t payload_size = ntohl(reinterpret_cast<eCAL::STcpHeader*>(header_buffer->data())->package_size_n);

                      if (payload_size > 0)
                      {
                        // If there is a payload, read it
                        me->read_request_payload(header_buffer);
                      }
                      else
                      {
                        // If there is no payload, directly execute the callback with an empty string
                        me->execute_callback(std::make_shared<std::string>());
                      }
                    });
  }

  void CAsioServerSessionV1::read_request_payload     (const std::shared_ptr<std::vector<char>>& header_buffer)
  {
    // Read how many bytes we will get as payload
    const uint32_t payload_size = ntohl(reinterpret_cast<eCAL::STcpHeader*>(header_buffer->data())->package_size_n);

    // Reserver enough memory for receiving the entire payload. The payload is
    // represented as an std::string for legacy, reasons. It is not textual data.
    // 
    // TODO: Check if I can make it a vector<char>
    //        TODO: If possible, the also remove the const_cast below
    const std::shared_ptr<std::string> payload_buffer = std::make_shared<std::string>(payload_size, '\0');

    // Read all the payload data into the payload_buffer
    asio::async_read(socket_
                  , asio::buffer(const_cast<char*>(payload_buffer->data()), payload_buffer->size())
                  , asio::transfer_at_least(payload_buffer->size())
                  , [me = shared_from_this(), payload_buffer](asio::error_code ec, std::size_t bytes_read)
                    {
                      if (ec)
                      {
                        std::cerr << "CAsioServerSessionV1: Error while reading header rest: " << ec.message() << std::endl; // TODO: Make logging message
                        return;
                      }
                      
                      std::cout << "payload: read " << bytes_read << " bytes" << std::endl; // TODO: Remove

                      // Execute the callback with the received data
                      me->execute_callback(payload_buffer);
                    });
  }

  void CAsioServerSessionV1::execute_callback         (const std::shared_ptr<std::string>& request_payload_buffer)
  {
    // Create an std::string for the response that is create by the callback
    const std::shared_ptr<std::string> response_buffer = std::make_shared<std::string>();

    // Synchronously execute the callback
    // TODO: Is it really safe to execute a callback here in the netcode? How much time can a callback take?
    request_callback_(*request_payload_buffer, *response_buffer);

    // Send the response
    send_response_header(response_buffer);
  }

  void CAsioServerSessionV1::send_response_header     (const std::shared_ptr<std::string>& response)
  {
    const std::shared_ptr<eCAL::STcpHeader> response_header = std::make_shared<eCAL::STcpHeader>();

    response_header->header_size_n  = htons(sizeof(eCAL::STcpHeader));
    response_header->package_size_n = htonl(static_cast<std::uint32_t>(response->size()));

    asio::async_write(socket_
                    , asio::buffer(reinterpret_cast<char*>(&(*response_header)), sizeof(eCAL::STcpHeader))
                    , [me = shared_from_this(), response_header, response](asio::error_code ec, std::size_t bytes_sent)
                    {
                      if (ec)
                      {
                        std::cerr << "CAsioServerSessionV1: Error while sending response header: " << ec.message() << std::endl; // TODO: Make logging message
                        return;
                      }

                      std::cout << "send_response_header: sent " << bytes_sent << " bytes" << std::endl; // TODO: Remove

                      if (!response->empty())
                      {
                        // If there is any payload, send it
                        me->send_response_payload(response);
                      }
                      else
                      {
                        // Else re-start by reading the next request header
                        me->read_request_header_start();
                      }
                    });
  }

  void CAsioServerSessionV1::send_response_payload    (const std::shared_ptr<std::string>& response)
  {
    asio::async_write(socket_
                    , asio::buffer(*response)
                    , [me = shared_from_this(), response](asio::error_code ec, std::size_t bytes_sent)
                    {
                      if (ec)
                      {
                        std::cerr << "CAsioServerSessionV1: Error while sending response payload: " << ec.message() << std::endl; // TODO: Make logging message
                        return;
                      }

                      std::cout << "send_response_payload: sent " << bytes_sent << " bytes" << std::endl; // TODO: Remove

                      // Re-start by receiving the header size
                      me->read_request_header_start();
                    });
  }

} // namespace eCAL
