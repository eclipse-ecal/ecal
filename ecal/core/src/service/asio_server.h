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
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "ecal_tcpheader.h"
#include "ecal/cimpl/ecal_callback_cimpl.h"

typedef std::function<int(const std::string& request, std::string& response)>    RequestCallbackT;
typedef std::function<void(eCAL_Server_Event event, const std::string& message)> EventCallbackT;

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

  void start()
  {
    socket_.async_read_some(asio::buffer(data_, max_length),
      std::bind(&CAsioSession::handle_read, this,
        std::placeholders::_1,
        std::placeholders::_2));
  }

  void add_request_callback1(RequestCallbackT callback_)
  {
    request_callback_ = callback_;
  }

  void add_event_callback(EventCallbackT callback_)
  {
    event_callback_ = callback_;
  }

private:
  void handle_read(const asio::error_code& ec,
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
        if (socket_.available())
        {
          // read some more bytes
          socket_.async_read_some(asio::buffer(data_, max_length),
            std::bind(&CAsioSession::handle_read, this,
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
            bind(&CAsioSession::handle_write, this,
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

  void handle_write(const asio::error_code& ec, std::size_t /*bytes_transferred*/)
  {
    if (!ec)
    {
      //std::cout << "CAsioSession::handle_write bytes sent " << bytes_transferred << std::endl;
      socket_.async_read_some(asio::buffer(data_, max_length),
        std::bind(&CAsioSession::handle_read, this,
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
  std::string            request_;
  std::string            response_;
  std::vector<char>      packed_response_;

  enum { max_length = 64 * 1024 };
  char data_[max_length];
};

class CAsioServer
{
public:
  CAsioServer(asio::io_service& io_service, unsigned short port)
    : io_service_(io_service),
    acceptor_(io_service, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)),
    connect_cnt_(0)
  {
    start_accept();
  }

  bool is_connected()
  {
    return (connect_cnt_ > 0);
  }

  void add_request_callback1(RequestCallbackT callback)
  {
    request_cb_ = callback;
  }

  void add_event_callback(EventCallbackT callback)
  {
    event_cb_ = callback;
  }

  uint16_t get_port()
  {
    return acceptor_.local_endpoint().port();
  }

private:
  void start_accept()
  {
    CAsioSession* new_session = new CAsioSession(io_service_);
    acceptor_.async_accept(new_session->socket(),
      std::bind(&CAsioServer::handle_accept, this, new_session,
        std::placeholders::_1));
  }

  void handle_accept(CAsioSession* new_session,
    const asio::error_code& ec)
  {
    if (!ec)
    {
      // handle the connect
      connect_cnt_++;
      if (event_cb_)
      {
        event_cb_(server_event_connected, "CAsioSession connected");
      }

      new_session->start();
      new_session->add_request_callback1(std::bind(&CAsioServer::on_request, this, std::placeholders::_1, std::placeholders::_2));
      new_session->add_event_callback(std::bind(&CAsioServer::on_event,      this, std::placeholders::_1, std::placeholders::_2));
    }
    else
    {
      delete new_session;
    }

    start_accept();
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
  int                      connect_cnt_;
};
