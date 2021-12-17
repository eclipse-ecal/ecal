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

#include <thread>
#include <memory>

#include <ecal/ecal_os.h>

#ifdef ECAL_OS_WINDOWS
#include "ecal_win_socket.h"
#endif

#ifdef ECAL_OS_LINUX
#include <sys/types.h>
#include <sys/socket.h>
#endif

#include "asio_server.h"

namespace eCAL
{
  class CTcpServer
  {
  public:
    CTcpServer();
    ~CTcpServer();

    void Create();
    void Destroy();

    void Start(RequestCallbackT request_callback_, EventCallbackT event_callback_);
    void Stop();

    bool IsConnected();

    std::shared_ptr<asio::io_service> GetIOService() { return m_io_service; };
    unsigned short GetTcpPort() { return (m_server ? m_server->get_port() : 0); }

  protected:
    void ServerThread(std::uint32_t port_, RequestCallbackT request_callback_, EventCallbackT event_callback_);

    bool                               m_started;

    std::shared_ptr<asio::io_service>  m_io_service;
    std::shared_ptr<CAsioServer>       m_server;
    std::thread                        m_server_thread;
  };
};
