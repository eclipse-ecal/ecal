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

#include <asio.hpp>

typedef std::function<int(const std::string& request, std::string& response)> RequestCallbackT;

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

  void add_request_callback(RequestCallbackT callback_)
  {
    request_callback_ = callback_;
  }

private:
  void handle_read(const asio::error_code& ec,
    size_t bytes_transferred)
  {
    if (!ec)
    {
      std::string response;
      if (request_callback_)
      {
        std::string request(data_, bytes_transferred);
        int ret = request_callback_(request, response);
        // -1 means incomplete package
        // read some more ...
        if (ret < 0)
        {
          socket_.async_read_some(asio::buffer(data_, max_length),
            std::bind(&CAsioSession::handle_read, this,
              std::placeholders::_1,
              std::placeholders::_2));
        }
      }
      asio::async_write(socket_,
        asio::buffer(response.data(), response.length()),
        bind(&CAsioSession::handle_write, this,
          std::placeholders::_1));
    }
    else
    {
      delete this;
    }
  }

  void handle_write(const asio::error_code& ec)
  {
    if (!ec)
    {
      socket_.async_read_some(asio::buffer(data_, max_length),
        std::bind(&CAsioSession::handle_read, this,
          std::placeholders::_1,
          std::placeholders::_2));
    }
    else
    {
      delete this;
    }
  }

  asio::ip::tcp::socket  socket_;
  RequestCallbackT       request_callback_;

  enum { max_length = 64 * 1024 };
  char data_[max_length];
};

class CAsioServer
{
public:
  CAsioServer(asio::io_service& io_service, unsigned short port)
    : io_service_(io_service),
    acceptor_(io_service, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
  {
    start_accept();
  }

  void add_request_callback(RequestCallbackT callback)
  {
    request_cb_ = callback;
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
      new_session->start();
      new_session->add_request_callback(std::bind(&CAsioServer::do_request, this, std::placeholders::_1, std::placeholders::_2));
    }
    else
    {
      delete new_session;
    }

    start_accept();
  }

  int do_request(const std::string& request, std::string& response)
  {
    if (request_cb_)
    {
      return request_cb_(request, response);
    }
    return 0;
  }

  asio::io_service&        io_service_;
  asio::ip::tcp::acceptor  acceptor_;
  RequestCallbackT         request_cb_;
};
