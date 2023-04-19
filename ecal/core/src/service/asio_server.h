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

#pragma once

#include <functional>
#include <string>
#include <iostream>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4834)
#endif
#include <asio.hpp>
#include <utility>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "ecal_tcpheader.h"
#include "ecal/cimpl/ecal_callback_cimpl.h"

using RequestCallbackT = std::function<int (const std::string &, std::string &)>;
using EventCallbackT = std::function<void (eCAL_Server_Event, const std::string &)>;

class CAsioSession
{
public:
  CAsioSession(asio::io_service& io_service)
    : socket_(io_service), data_{}
  {
  }

  asio::ip::tcp::socket& socket()
  {
    return socket_;
  }

  void start(unsigned int version)
  {
    if (version == PROTOCOL_VERSION_0)
    {
      socket_.async_read_some(asio::buffer(data_, max_length),
        std::bind(&CAsioSession::handle_read_v0, this,
          std::placeholders::_1,
          std::placeholders::_2));
    }
    else if (version == PROTOCOL_VERSION_1)
    {
      socket_.async_read_some(asio::buffer(data_, max_length),
        std::bind(&CAsioSession::handle_read_v1, this,
          std::placeholders::_1,
          std::placeholders::_2));
    }
    else
    {
      std::cerr << "CAsioSession 'start' called with unknown protocol version: " << version << std::endl;
    }
  }

  void add_request_callback1(RequestCallbackT callback_)
  {
    request_callback_ = std::move(callback_);
  }

  void add_event_callback(EventCallbackT callback_)
  {
    event_callback_ = std::move(callback_);
  }

private:
  void handle_read_v0(const asio::error_code& ec,
    size_t bytes_transferred)
  {
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
          socket_.async_read_some(asio::buffer(data_, max_length),
            std::bind(&CAsioSession::handle_read_v0, this,
              std::placeholders::_1,
              std::placeholders::_2));
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
          asio::async_write(socket_,
            asio::buffer(packed_response_.data(), packed_response_.size()),
            bind(&CAsioSession::handle_write_v0, this,
              std::placeholders::_1,
              std::placeholders::_2));
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
      delete this;
    }
  }

  void handle_read_v1(const asio::error_code& ec,
    size_t bytes_transferred)
  {
    if (!ec)
    {
      if (request_callback_)
      {
        size_t bytes_used = 0;
        std::string const data = std::string(data_, bytes_transferred);

        // collect header
        if (data.size() < sizeof(eCAL::STcpHeader) && header_.size() < sizeof(eCAL::STcpHeader))
        {
          header_ += data;
        }
        else if (data.size() >= sizeof(eCAL::STcpHeader) && header_.size() < sizeof(eCAL::STcpHeader))
        {
          bytes_used += sizeof(eCAL::STcpHeader) - header_.size();
          header_ += data.substr(0, bytes_used);
        }

        // decode header and get request size
        if (header_.size() == sizeof(eCAL::STcpHeader) && header_request_size_ == 0)
        {
          eCAL::STcpHeader tcp_header;
          memcpy(&tcp_header, header_.data(), sizeof(eCAL::STcpHeader));
          header_request_size_ = static_cast<size_t>(ntohl(tcp_header.psize_n));
        }

        // collect request_
        if (header_request_size_ != 0 && request_.size() < header_request_size_)
        {
          request_ += data.substr(bytes_used, bytes_transferred - bytes_used);
        }

        // execute callback
        if (header_request_size_ != 0 && request_.size() == header_request_size_)
        {
          response_.clear();
          request_callback_(request_, response_);
          header_.clear();
          request_.clear();
          header_request_size_ = 0;

          // write response
          packed_response_.clear();
          packed_response_ = pack_write(response_);
          asio::async_write(socket_,
            asio::buffer(packed_response_.data(), packed_response_.size()),
            bind(&CAsioSession::handle_write_v1, this,
              std::placeholders::_1,
              std::placeholders::_2));
        }
        else
        {
          // read some more bytes until all of request data has been received
          socket_.async_read_some(asio::buffer(data_, max_length),
            std::bind(&CAsioSession::handle_read_v1, this,
              std::placeholders::_1,
              std::placeholders::_2));
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
          event_callback_(server_event_disconnected, "CAsioSession disconnected on read: " + ec.message());
        }
      }
      delete this;
    }
  }

  std::vector<char> pack_write(const std::string& response)
  {
    // create header
    eCAL::STcpHeader tcp_header;
    // set up package size
    const size_t psize = response.size();
    tcp_header.psize_n = htonl(static_cast<uint32_t>(psize));
    // repack
    std::vector<char> packed_response(sizeof(tcp_header) + psize);
    memcpy(packed_response.data(), &tcp_header, sizeof(tcp_header));
    memcpy(packed_response.data() + sizeof(tcp_header), response.data(), psize);
    return packed_response;
  }

  void handle_write_v0(const asio::error_code& ec, std::size_t /*bytes_transferred*/)
  {
    if (!ec)
    {
      //std::cout << "CAsioSession::handle_write bytes sent " << bytes_transferred << std::endl;
      socket_.async_read_some(asio::buffer(data_, max_length),
        std::bind(&CAsioSession::handle_read_v0, this,
          std::placeholders::_1,
          std::placeholders::_2));
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
      delete this;
    }
  }

  void handle_write_v1(const asio::error_code& ec, std::size_t /*bytes_transferred*/)
  {
    if (!ec)
    {
      //std::cout << "CAsioSession::handle_write bytes sent " << bytes_transferred << std::endl;
      socket_.async_read_some(asio::buffer(data_, max_length),
        std::bind(&CAsioSession::handle_read_v1, this,
         std::placeholders::_1,
          std::placeholders::_2));
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
      delete this;
    }
  }

  asio::ip::tcp::socket  socket_;
  RequestCallbackT       request_callback_;
  EventCallbackT         event_callback_;
  std::string            header_;
  size_t                 header_request_size_ = 0;
  std::string            request_;
  std::string            response_;
  std::vector<char>      packed_response_;

  enum { max_length = 64 * 1024 };
  char                   data_[max_length];

  const unsigned int     PROTOCOL_VERSION_0 = 0;
  const unsigned int     PROTOCOL_VERSION_1 = 1;
};

class CAsioServer
{
public:
  CAsioServer(asio::io_service& io_service, unsigned int version, unsigned short port) :
    io_service_(io_service),
    acceptor_(io_service, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
  {
    start_accept(version);
  }

  bool is_connected() const
  {
    return (connect_cnt_ > 0);
  }

  void add_request_callback1(RequestCallbackT callback)
  {
    request_cb_ = std::move(callback);
  }

  void add_event_callback(EventCallbackT callback)
  {
    event_cb_ = std::move(callback);
  }

  uint16_t get_port()
  {
    return acceptor_.local_endpoint().port();
  }

private:
  void start_accept(unsigned int version)
  {
    CAsioSession* new_session = new CAsioSession(io_service_);
    acceptor_.async_accept(new_session->socket(),
      std::bind(&CAsioServer::handle_accept, this, new_session, version,
        std::placeholders::_1));
  }

  void handle_accept(CAsioSession* new_session, unsigned int version,
    const asio::error_code& ec)
  {
    if (!ec)
    {
      // handle the connection
      connect_cnt_++;
      if (event_cb_)
      {
        event_cb_(server_event_connected, "CAsioSession connected");
      }

      new_session->start(version);
      new_session->add_request_callback1(std::bind(&CAsioServer::on_request, this, std::placeholders::_1, std::placeholders::_2));
      new_session->add_event_callback(std::bind(&CAsioServer::on_event,      this, std::placeholders::_1, std::placeholders::_2));
    }
    else
    {
      delete new_session;
    }

    start_accept(version);
  }

  int on_request(const std::string& request, std::string& response)
  {
    if (request_cb_)
    {
      return request_cb_(request, response);
    }
    return 0;
  }

  void on_event(eCAL_Server_Event event, const std::string& message)
  {
    switch (event)
    {
    case server_event_connected:
      connect_cnt_++;
      break;
    case server_event_disconnected:
      connect_cnt_--;
      break;
    default:
      break;
    }

    if (event_cb_)
    {
      event_cb_(event, message);
    }
  }

  asio::io_service&        io_service_;
  asio::ip::tcp::acceptor  acceptor_;
  RequestCallbackT         request_cb_;
  EventCallbackT           event_cb_;
  int                      connect_cnt_ = 0;
};
