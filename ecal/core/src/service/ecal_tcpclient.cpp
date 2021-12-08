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

/**
 * @brief  eCAL tcp server based on asio c++
**/

#include "ecal_tcpclient.h"
#include "ecal_tcpheader.h"

#include <iostream>

namespace eCAL
{
  //////////////////////////////////////////////////////////////////
  // CTcpClient
  //////////////////////////////////////////////////////////////////
  CTcpClient::CTcpClient() : m_created(false), m_connected(false)
  {
  }

  CTcpClient::CTcpClient(const std::string& host_name_, unsigned short port_) : m_created(false)
  {
    Create(host_name_, port_);
  }

  CTcpClient::~CTcpClient()
  {
    Destroy();
  }

  void CTcpClient::Create(const std::string& host_name_, unsigned short port_)
  {
    if (m_created) return;

    m_host_name  = host_name_;
    m_io_service = std::make_shared<asio::io_service>();
    m_socket     = std::make_shared<asio::ip::tcp::socket>(*m_io_service);
    asio::ip::tcp::resolver resolver(*m_io_service);

    try
    {
      m_idle_work = std::make_shared<asio::io_service::work>(*m_io_service);
      //NOTE: might want to lazy load this in future
      m_async_worker = std::thread(
      [this] 
      {
        m_io_service->run();
      });

      asio::connect(*m_socket, resolver.resolve({ host_name_, std::to_string(port_) }));
      // set TCP no delay, so Nagle's algorithm will not stuff multiple messages in one TCP segment
      asio::ip::tcp::no_delay no_delay_option(true);
      m_socket->set_option(no_delay_option);
      m_connected = true;
    }
    catch (std::exception& /*e*/)
    {
      //std::cerr << "CTcpClient::Connect conect exception: " << e.what() << "\n";
      m_connected = false;
    }

    m_created = true;
  }

  void CTcpClient::Destroy()
  {
    if (!m_created) return;

    m_socket    ->close();
    m_io_service->stop();

    m_async_worker.join();

    m_idle_work  = nullptr;
    m_socket     = nullptr;
    m_io_service = nullptr;
    m_connected  = false;

    m_created = false;
  }

  size_t CTcpClient::ExecuteRequest(const std::string &request_, std::string &response_)
  {
    std::lock_guard<std::mutex> lock(m_socket_write_mutex);

    if (!m_created) return 0;

    if(!SendRequest(request_)) return 0;

    return ReceiveResponse(response_);
  }

  void CTcpClient::ExecuteRequestAsync(const std::string &request_, AsyncCallbackT callback)
  {
    std::unique_lock<std::mutex> lock(m_socket_write_mutex);

    if (!m_created)
      callback("", false);

    //Start waiting for response
    ReceiveResponseAsync(callback);
    
    if(!SendRequest(request_)) 
      callback("", false);
  }

  bool CTcpClient::SendRequest(const std::string &request_)
  {
    size_t written(0);
    try
    {
      // send payload to server
      while (written != request_.size())
      {
        auto bytes_written = m_socket->write_some(asio::buffer(request_.c_str() + written, request_.size() - written));
        written += bytes_written;
      }
    }
    catch (std::exception &e)
    {
      std::cerr << "CTcpClient::ExecuteRequest: Failed to send request: " << e.what() << "\n";
      m_connected = false;
      return false;
    }
  }

  size_t CTcpClient::ReceiveResponse(std::string &response_)
  {
    try
    {
      // read stream header
      STcpHeader tcp_header;
      size_t bytes_read = m_socket->read_some(asio::buffer(&tcp_header, sizeof(tcp_header)));
      if (bytes_read != sizeof(tcp_header))
        return 0;

      // extract data size
      const size_t rsize = static_cast<size_t>(ntohl(tcp_header.psize_n));

      // prepare response buffer
      response_.clear();
      response_.reserve(rsize);

      // read stream data
      do
      {
        const size_t buffer_size(1024);
        char buffer[buffer_size];
        size_t bytes_left = rsize - response_.size();
        size_t bytes_to_read = std::min(buffer_size, bytes_left);
        bytes_read = m_socket->read_some(asio::buffer(buffer, bytes_to_read));
        //std::cout << "CTcpClient::ExecuteRequest read response bytes " << bytes_read << " to " << response_.size() << std::endl;
        response_ += std::string(buffer, bytes_read);
      } while (response_.size() < rsize);

      return response_.size();
    }
    catch (std::exception &e)
    {
      std::cerr << "CTcpClient::ExecuteRequest: Failed to recieve response: " << e.what() << std::endl;
      m_connected = false;
      return 0;
    }
  }

  void CTcpClient::ReceiveResponseAsync(AsyncCallbackT callback_)
  {
    std::shared_ptr<STcpHeader> tcp_header = std::make_shared<STcpHeader>();
    //std::unique_lock<std::mutex> lock(m_socket_read_mutex);

    m_socket->async_read_some(asio::buffer(tcp_header.get(), sizeof(tcp_header)),
    [this, tcp_header, callback_/*, lock = std::move(lock)*/](auto ec, auto bytes_transferred) 
    {
      if(ec) callback_("", false);

      if (bytes_transferred == sizeof(tcp_header))
      {
        const auto resp_size = static_cast<size_t>(ntohl(tcp_header->psize_n));
        this->ReceiveResponseData(resp_size, callback_);
      }
      else
      {
        std::cerr << "CTcpClient::ExecuteRequest: Failed to receive response: " << "tcp_header size is invalid." << "\n";
        callback_("", false);
      }
    });
  }

  void CTcpClient::ReceiveResponseData(const size_t size_, AsyncCallbackT callback_)
  {
    std::string data(size_, ' ');
    asio::error_code ec;

    //We are already in io_worker_thread
    asio::read(*m_socket, asio::buffer(&data[0], data.size()), ec);

    if(ec) callback_("", false);
    else callback_(data, true);
  }
};
