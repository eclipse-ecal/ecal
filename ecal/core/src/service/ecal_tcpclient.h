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

#pragma once

#include <atomic>
#include <thread>
#include <mutex>
#include <memory>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4834)
#endif
#include <asio.hpp>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <ecal/ecal_os.h>

#ifdef ECAL_OS_WINDOWS
#include "ecal_win_socket.h"
#endif
#include "ecal/cimpl/ecal_callback_cimpl.h"

#ifdef ECAL_OS_LINUX
#include <sys/types.h>
#include <sys/socket.h>
#endif

namespace eCAL
{
  class CTcpClient
  {
  public:
    typedef std::function<void(const std::string& data_, bool successful_)> AsyncCallbackT;
    typedef std::function<void(eCAL_Client_Event event, const std::string& message)> EventCallbackT;

    CTcpClient();
    CTcpClient(const std::string& host_name_, unsigned short port_);

    ~CTcpClient();

    void Create(const std::string& host_name_, unsigned short port_);
    void Destroy();

    bool IsConnected();

    std::string GetHostName() { return m_host_name; }

    bool AddEventCallback(EventCallbackT callback_);
    bool RemEventCallback();

    size_t ExecuteRequest(const std::string& request_, int timeout_, std::string& response_);
    void ExecuteRequestAsync(const std::string& request_, int timeout_, AsyncCallbackT callback);

  protected:
    std::string                             m_host_name;
    std::mutex                              m_socket_write_mutex; 
    std::mutex                              m_socket_read_mutex; 
    std::thread                             m_async_worker;
    std::shared_ptr<asio::io_service>       m_io_service;
    std::shared_ptr<asio::io_service::work> m_idle_work;
    std::shared_ptr<asio::ip::tcp::socket>  m_socket;
    EventCallbackT                          m_event_callback;
    bool                                    m_created;
    bool                                    m_connected;
    std::atomic<bool>                       m_async_request_in_progress;

  private:
    bool SendRequest(const std::string &request_);
    size_t ReceiveResponse(std::string &response_, int timeout_);
    void ReceiveResponseAsync(AsyncCallbackT callback_, int timeout_);
    void ReceiveResponseData(const size_t size, AsyncCallbackT callback_);
    void ExecuteCallback(AsyncCallbackT callback_, const std::string &data_, bool success_);
  };
};
