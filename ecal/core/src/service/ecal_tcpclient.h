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

#ifdef ECAL_OS_LINUX
#include <sys/types.h>
#include <sys/socket.h>
#endif

namespace eCAL
{
  class CTcpClient
  {
  public:
    CTcpClient();
    CTcpClient(const std::string& host_name_, unsigned short port_);

    ~CTcpClient();

    void Create(const std::string& host_name_, unsigned short port_);
    void Destroy();

    bool IsConnected() { return m_connected; };

    std::string GetHostName() { return m_host_name; }

    size_t ExecuteRequest(const std::string& request_, std::string& response_);

  protected:
    std::string                            m_host_name;
    std::shared_ptr<asio::io_service>      m_io_service;
    std::shared_ptr<asio::ip::tcp::socket> m_socket;
    bool                                   m_created;
    bool                                   m_connected;
  };
};
