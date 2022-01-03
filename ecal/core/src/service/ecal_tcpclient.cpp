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

#include <chrono>
#include <iostream>
#include <thread>

namespace eCAL
{
  //////////////////////////////////////////////////////////////////
  // CTcpClient
  //////////////////////////////////////////////////////////////////
  CTcpClient::CTcpClient() : m_created(false), m_connected(false), m_async_request_in_progress(false)
  {
  }

  CTcpClient::CTcpClient(const std::string& host_name_, unsigned short port_) : m_created(false), m_connected(false), m_async_request_in_progress(false)
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
      // NOTE: might want to lazy load this in future
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

      // fire event
      if (m_event_callback)
      {
        m_event_callback(client_event_connected, "CTcpClient connected");
      }
    }
    catch (std::exception& /*e*/)
    {
      //std::cerr << "CTcpClient::Connect conect exception: " << e.what() << "\n";
      m_connected = false;

      // fire event
      if (m_event_callback)
      {
        m_event_callback(client_event_disconnected, "CTcpClient disconnected");
      }
    }

    m_created = true;
  }

  void CTcpClient::Destroy()
  {
    if (!m_created) return;

    m_socket->close();
    m_io_service->stop();

    m_async_worker.join();

    m_idle_work  = nullptr;
    m_socket     = nullptr;
    m_io_service = nullptr;

    m_connected                 = false;
    m_async_request_in_progress = false;
    m_created                   = false;
  }

  bool CTcpClient::IsConnected()
  {
    return m_connected;
  }

  bool CTcpClient::AddEventCallback(EventCallbackT callback_)
  {
    m_event_callback = callback_;
    return true;
  }

  bool CTcpClient::RemEventCallback()
  {
    m_event_callback = nullptr;
    return true;
  }

  size_t CTcpClient::ExecuteRequest(const std::string& request_, int timeout_, std::string& response_)
  {
    std::lock_guard<std::mutex> lock(m_socket_write_mutex);

    if (!m_created) return 0;

    if (!SendRequest(request_)) return 0;

    return ReceiveResponse(response_, timeout_);
  }

  void CTcpClient::ExecuteRequestAsync(const std::string& request_, int timeout_, AsyncCallbackT callback)
  {
    std::unique_lock<std::mutex> lock(m_socket_write_mutex);
    if (!m_async_request_in_progress)
    {
      m_async_request_in_progress.store(true);

      if (!m_created)
        ExecuteCallback(callback, "", false);

      // start waiting for response
      ReceiveResponseAsync(callback, timeout_);

      if (!SendRequest(request_))
        ExecuteCallback(callback, "", false);
    }
    else
    {
      std::cerr << "CTcpClient::ExecuteRequestAsync failed: Another request is already in progress" << std::endl;
      ExecuteCallback(callback, "", false);
    }
  }

  bool CTcpClient::SendRequest(const std::string& request_)
  {
    size_t written(0);
    try
    {
      // check for old (timeouted ?) reponses
      const size_t resp_size = m_socket->available();
      if (resp_size > 0)
      {
        std::vector<char> resp_buffer(resp_size);
        m_socket->read_some(asio::buffer(resp_buffer));
      }

      // send payload to server
      while (written != request_.size())
      {
        auto bytes_written = m_socket->write_some(asio::buffer(request_.c_str() + written, request_.size() - written));
        written += bytes_written;
      }
    }
    catch (std::exception& e)
    {
      std::cerr << "CTcpClient::SendRequest: Failed to send request: " << e.what() << "\n";
      m_connected = false;
      return false;
    }

    return true;
  }

  size_t CTcpClient::ReceiveResponse(std::string& response_, int timeout_)
  {
    try
    {
      assert((timeout_ == -1) || (timeout_ > 0));

      STcpHeader tcp_header;
      bool       read_done(false);
      bool       read_failed(false);
      bool       time_expired(false);

      // read stream header (async)
      if (timeout_ != -1)
      {
        // start timer
        asio::steady_timer timer(*m_io_service);
        timer.expires_from_now(asio::chrono::milliseconds(timeout_));
        timer.async_wait(
          [&](const asio::error_code& ec)
          {
            if (!ec) time_expired = true;
          }
        );

        // async read
        m_socket->async_read_some(asio::buffer(&tcp_header, sizeof(tcp_header)),
          [&](const asio::error_code& /*ec*/, std::size_t bytes_read)
          {
            if (bytes_read != sizeof(tcp_header)) read_failed = true;
            else                                  read_done   = true;
          }
        );

        // idle operations
        while (!read_done && !read_failed && !time_expired)
        {
          std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        // stop timer
        timer.cancel();
      }
      // read stream header (sync)
      else
      {
        size_t bytes_read = m_socket->read_some(asio::buffer(&tcp_header, sizeof(tcp_header)));
        if (bytes_read != sizeof(tcp_header)) read_failed = true;
        else                                  read_done   = true;
      }

      // check for expired timer
      if (time_expired)
      {
        // fire event
        if (m_event_callback)
        {
          m_event_callback(client_event_timeout, "ReceiveResponse timeouted");
        }

        // cleanup the socket and return
        m_socket->cancel();
        return 0;
      }

      // check for failed read
      if (!read_done || read_failed)
      {
        // cleanup the socket and return
        m_socket->cancel();
        return 0;
      }

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
        size_t bytes_left    = rsize - response_.size();
        size_t bytes_to_read = std::min(buffer_size, bytes_left);
        size_t bytes_read    = m_socket->read_some(asio::buffer(buffer, bytes_to_read));
        response_ += std::string(buffer, bytes_read);

        //std::cout << "CTcpClient::ReceiveResponse read response bytes " << bytes_read << " to " << response_.size() << std::endl;
      }
      while (response_.size() < rsize);

      return response_.size();
    }
    catch (std::exception& e)
    {
      std::cerr << "CTcpClient::ReceiveResponse: Failed to recieve response: " << e.what() << std::endl;
      m_connected = false;
      return 0;
    }
  }

  void CTcpClient::ReceiveResponseAsync(AsyncCallbackT callback_, int timeout_)
  {
    assert((timeout_ == -1) || (timeout_ > 0));

    // start timer
    //asio::steady_timer timer(*m_io_service);
    //timer.expires_from_now(asio::chrono::milliseconds(timeout_));
    //timer.async_wait(
    //  [&](const asio::error_code& ec)
    //  {
    //    // timer expired
    //    if (!ec)
    //    {
    //      // fire event
    //      if (m_event_callback)
    //      {
    //        m_event_callback(client_event_timeout, "ReceiveResponseAsync timeouted");
    //      }

    //      // cleanup the socket
    //      m_socket->cancel();
    //    }
    //  }
    //);

    std::shared_ptr<STcpHeader> tcp_header = std::make_shared<STcpHeader>();
    //std::unique_lock<std::mutex> lock(m_socket_read_mutex);

    m_socket->async_read_some(asio::buffer(tcp_header.get(), sizeof(tcp_header)),
      [this, tcp_header, callback_/*, lock = std::move(lock)*/](auto ec, auto bytes_transferred)
      {
        if (ec) ExecuteCallback(callback_, "", false);

        if (bytes_transferred == sizeof(tcp_header))
        {
          const auto resp_size = static_cast<size_t>(ntohl(tcp_header->psize_n));
          this->ReceiveResponseData(resp_size, callback_);
        }
        else
        {
          std::cerr << "CTcpClient::ReceiveResponseAsync: Failed to receive response: " << "tcp_header size is invalid." << "\n";
          ExecuteCallback(callback_, "", false);
        }
      });
  }

  void CTcpClient::ReceiveResponseData(const size_t size_, AsyncCallbackT callback_)
  {
    std::string data(size_, ' ');
    asio::error_code ec;

    // we are already in io_worker_thread
    asio::read(*m_socket, asio::buffer(&data[0], data.size()), ec);

    if (ec) ExecuteCallback(callback_, "", false);
    else ExecuteCallback(callback_, data, true);
  }

  void CTcpClient::ExecuteCallback(AsyncCallbackT callback_, const std::string &data_, bool success_)
  {
    m_async_request_in_progress = false;
    callback_(data_, success_);
  }  
};
