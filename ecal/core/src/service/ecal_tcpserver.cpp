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

#include <utility>

namespace eCAL
{
  //////////////////////////////////////////////////////////////////
  // CTcpServer
  //////////////////////////////////////////////////////////////////
  CTcpServer::~CTcpServer()
  {
    Stop();
  }

  void CTcpServer::Start(unsigned int version_, const eCAL::CAsioServer::RequestCallbackT& request_callback_, const eCAL::CAsioServer::EventCallbackT& event_callback_)
  {
    if (m_started)           return;
    if (m_server != nullptr) return;

    m_version       = version_;
    m_server_thread = std::thread(&CTcpServer::ServerThread, this, 0, request_callback_, event_callback_);

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

  bool CTcpServer::IsConnected()
  {
    if (!m_started) return false;

    if (m_server == nullptr) return false;

    return m_server->is_connected();
  }
  
  // TODO: Make the parameters const references
  void CTcpServer::ServerThread(std::uint32_t port_, eCAL::CAsioServer::RequestCallbackT request_callback_, eCAL::CAsioServer::EventCallbackT event_callback_)
  {
    m_io_service = std::make_shared<asio::io_service>();

    // TODO: Remove
    //m_server     = std::make_shared<CAsioServer>(*m_io_service, m_version, static_cast<unsigned short>(port_));

    m_server = CAsioServer::create(*m_io_service
                                  , m_version
                                  , static_cast<std::uint16_t>(port_)
                                  , request_callback_
                                  , event_callback_);

    m_io_service->run();

    m_server = nullptr;
  }
};
