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

#include "asio_tcp_server_session_v0.h"

#include "ecal_tcpheader.h"

#include <iostream> // TODO Remove

namespace eCAL
{

  ///////////////////////////////////////////////
  // Create, Constructor, Destructor
  ///////////////////////////////////////////////

  std::shared_ptr<CAsioTcpServerSessionV0> CAsioTcpServerSessionV0::create(asio::io_context& io_context_, const RequestCallbackT& request_callback, const EventCallbackT& event_callback)
  {
    return std::shared_ptr<CAsioTcpServerSessionV0>(new CAsioTcpServerSessionV0(io_context_, request_callback, event_callback));
  }

  CAsioTcpServerSessionV0::CAsioTcpServerSessionV0(asio::io_context& io_context_, const RequestCallbackT& request_callback, const EventCallbackT& event_callback)
    : CAsioTcpServerSession(io_context_, request_callback, event_callback)
  {}

  ///////////////////////////////////////////////
  // Data receiving
  ///////////////////////////////////////////////
  void CAsioTcpServerSessionV0::start()
  {
    std::cout << "CAsioTcpServerSessionV0::start()" << std::endl; // TODO: remove

    //socket_.async_read_some(asio::buffer(data_, max_length),
    //  std::bind(&CAsioServerSessionV0::handle_read, this,
    //    std::placeholders::_1,
    //    std::placeholders::_2));
    socket_.async_read_some(asio::buffer(data_, max_length)
                          , [me = shared_from_this()](asio::error_code ec, std::size_t bytes_read)
                            {
                               me->handle_read(ec, bytes_read);
                            });
                    
  }

  void CAsioTcpServerSessionV0::handle_read(const asio::error_code& ec, size_t bytes_transferred)
  {
    std::cout << "CAsioTcpServerSessionV0::handle_read()" << std::endl; // TODO: remove
    if (ec)
      std::cerr <<"CAsioTcpServerSessionV0::handle_read() ERROR: " << ec.message() << std::endl; // TODO Remove


    if (!ec)
    {
      if (request_callback_)
      {
        // collect request
        //std::cout << "CAsioSession::handle_read read bytes " << bytes_transferred << std::endl;
        request_ += std::string(data_, bytes_transferred);
        // are there some more data on the socket ?
        if (socket_.available() != 0u)
        {
          // read some more bytes
          //socket_.async_read_some(asio::buffer(data_, max_length),
          //  std::bind(&CAsioServerSessionV0::handle_read, this, // TODO: Make Lambda with shared_from_this in capture
          //    std::placeholders::_1,
          //    std::placeholders::_2));
          socket_.async_read_some(asio::buffer(data_, max_length)
                                , [me = shared_from_this()](asio::error_code ec, std::size_t bytes_read)
                                  {
                                    me->handle_read(ec, bytes_read);
                                  });
        }
        // no more data
        else
        {
          // execute service callback
          //std::cout << "CAsioSession::handle_read final request size " << request_.size() << std::endl;
          response_.clear();
          request_callback_(request_, response_);
          request_.clear();
          //std::cout << "CAsioSession::handle_read server callback executed - reponse size " << response_.size() << std::endl;
    
          // write response back
          packed_response_.clear();
          packed_response_ = pack_write(response_);
          //asio::async_write(socket_,
          //  asio::buffer(packed_response_.data(), packed_response_.size()),
          //  bind(&CAsioServerSessionV0::handle_write, this,  // TODO: Make Lambda with shared_from_this in capture
          //    std::placeholders::_1,
          //    std::placeholders::_2));
          asio::async_write(socket_
                          , asio::buffer(packed_response_.data(), packed_response_.size())
                          , [me = shared_from_this()](asio::error_code ec, std::size_t bytes_written)
                            {
                              me->handle_write(ec, bytes_written);
                            });
        }
      }
    }
    else
    {
      if ((ec == asio::error::eof) ||
        (ec == asio::error::connection_reset))
      {
        // handle the disconnect
        if (event_callback_)
        {
          event_callback_(server_event_disconnected, "CAsioSession disconnected on read");
        }
      }
      //delete this; // TODO: Remove
    }
  }

  void CAsioTcpServerSessionV0::handle_write(const asio::error_code& ec, std::size_t /*bytes_transferred*/)
  {
    std::cout << "CAsioTcpServerSessionV0::handle_write()" << std::endl; // TODO: remove

    if (!ec)
    {
      //std::cout << "CAsioSession::handle_write bytes sent " << bytes_transferred << std::endl;
      //socket_.async_read_some(asio::buffer(data_, max_length), // TODO: Make Lambda with shared_from_this in capture
      //  std::bind(&CAsioServerSessionV0::handle_read, this,
      //    std::placeholders::_1,
      //    std::placeholders::_2));
      socket_.async_read_some(asio::buffer(data_, max_length)
                            , [me = shared_from_this()](asio::error_code ec, std::size_t bytes_read)
                              {
                                me->handle_read(ec, bytes_read);
                              });
    }
    else
    {
      if ((ec == asio::error::eof) ||
          (ec == asio::error::connection_reset))
      {
        // handle the disconnect
        if (event_callback_)
        {
          event_callback_(server_event_disconnected, "CAsioSession disconnected on write");
        }
      }
      //delete this; // TODO: Remove
    }
  }

  std::vector<char> CAsioTcpServerSessionV0::pack_write(const std::string& response)
  {
    // create header
    eCAL::STcpHeader tcp_header;
    // set up package size
    const size_t psize = response.size();
    tcp_header.package_size_n = htonl(static_cast<uint32_t>(psize));
    tcp_header.header_size_n  = htons(static_cast<uint32_t>(sizeof(eCAL::STcpHeader)));
    // repack
    std::vector<char> packed_response(sizeof(tcp_header) + psize);
    memcpy(packed_response.data(), &tcp_header, sizeof(tcp_header));
    memcpy(packed_response.data() + sizeof(tcp_header), response.data(), psize);
    return packed_response;
  }

} // namespace eCAL