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

#include "ecal_tcpserver.h"

namespace eCAL
{
  //////////////////////////////////////////////////////////////////
  // CTcpServer
  //////////////////////////////////////////////////////////////////
  CTcpServer::CTcpServer() : m_started(false), m_port(0), m_sock(0)
  {
  }

  CTcpServer::~CTcpServer()
  {
    Destroy();
  }

  void CTcpServer::Create()
  {
  }

  void CTcpServer::Destroy()
  {
    Stop();
  }

  void CTcpServer::Start(RequestCallbackT callback_)
  {
    if (m_started)           return;
    if (m_server != nullptr) return;

    GrabSocket();
    m_server_thread = std::thread(&CTcpServer::ServerThread, this, m_port, callback_);
    FreeSocket();

    m_started = true;
  }

  void CTcpServer::Stop()
  {
    if (!m_started) return;

    if (m_server == nullptr) return;
    if (m_io_service != nullptr) m_io_service->stop();
    m_server_thread.join();

    m_started = false;
  }
  
  void CTcpServer::ServerThread(std::uint32_t port_, RequestCallbackT callback_)
  {
    m_io_service = std::make_shared<asio::io_service>();
    m_server = std::make_shared<CAsioServer>(*m_io_service, static_cast<unsigned short>(port_));
    m_server->add_request_callback(callback_);
    m_io_service->run();
    m_server = nullptr;
  }

  bool CTcpServer::GrabSocket()
  {
    // create socket
    m_sock = socket(AF_INET, SOCK_STREAM, 0);
#ifdef ECAL_OS_WINDOWS
    if (m_sock == INVALID_SOCKET) return false;
#endif
#ifdef ECAL_OS_LINUX
    if (m_sock < 0) return false;
#endif

    // set socket reuse
    int reuse = 1;
    if(setsockopt(m_sock, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&reuse), sizeof(reuse)))
      perror("CTcpServer: Error setting socket option SO_REUSEADDR");

    // bind socket to next free port
    struct sockaddr_in saddr;
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = INADDR_ANY;
    saddr.sin_port = htons(0);
    if (bind(m_sock, (struct sockaddr *) &saddr, sizeof(struct sockaddr_in)) < 0) return false;

    // listen to socket
    if (listen(m_sock, 5) < 0) return false;

    // get available port
    memset(&saddr, 0, sizeof(saddr));
    socklen_t len = sizeof(struct sockaddr);
    getsockname(m_sock, (struct sockaddr*)&saddr, &len);
    m_port = saddr.sin_port;

    return (m_port > 0);
  }

  bool CTcpServer::FreeSocket()
  {
#ifdef ECAL_OS_WINDOWS
    if (m_sock == INVALID_SOCKET) return false;
#endif
#ifdef ECAL_OS_LINUX
    if (m_sock < 0) return false;
#endif

    // shutdown socket
    shutdown(m_sock, 2);

    // close socket
#ifdef ECAL_OS_WINDOWS
    closesocket(m_sock);
#endif

#ifdef ECAL_OS_LINUX
    close(m_sock);
#endif

    return true;
  }
};
