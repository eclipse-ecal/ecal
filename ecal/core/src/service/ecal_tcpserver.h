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
    CTcpServer() = default;
    ~CTcpServer();

    CTcpServer(const CTcpServer &) = delete;
    CTcpServer(CTcpServer &&) noexcept = delete;

    CTcpServer& operator=(const CTcpServer &) = delete;
    CTcpServer& operator=(CTcpServer &&) noexcept = delete;

    void Start(unsigned int version_, const eCAL::CAsioServer::RequestCallbackT& request_callback_, const eCAL::CAsioServer::EventCallbackT& event_callback_);
    void Stop();

    bool IsConnected();

    unsigned short GetTcpPort() { return (m_server ? m_server->get_port() : 0); }
    unsigned int   GetVersion() { return (m_server ? m_version            : 0); }

  protected:
    void ServerThread(std::uint32_t port_, eCAL::CAsioServer::RequestCallbackT request_callback_, eCAL::CAsioServer::EventCallbackT event_callback_);

    bool                               m_started = false;
    unsigned int                       m_version = 0;

    std::shared_ptr<asio::io_service>  m_io_service;
    std::shared_ptr<CAsioServer>       m_server;
    std::thread                        m_server_thread;
  };
};
