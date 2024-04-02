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
 * @brief  UDP receiver class
**/

#pragma once

#include <cstddef>
#include <ecal/ecal_os.h>

#ifdef ECAL_OS_WINDOWS
#include "win32/socket_os.h"
#endif

#ifdef ECAL_OS_LINUX
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#endif

#include <memory>
#include <mutex>
#include <string>

namespace IO
{
  namespace UDP
  {
    struct SReceiverAttr
    {
      std::string address;
      int         port      = 0;
      bool        broadcast = false;
      bool        loopback  = true;
      int         rcvbuf    = 1024 * 1024;
    };

    class CUDPReceiverImpl
    {
    public:
      explicit CUDPReceiverImpl(const SReceiverAttr& /*attr_*/) {};
      // We don't technically need a virtual destructor, if we are working with shared_ptrs...
      virtual ~CUDPReceiverImpl() = default;

      // Delete copy / move operations to prevent slicing
      CUDPReceiverImpl(CUDPReceiverImpl&&) = delete;
      CUDPReceiverImpl& operator=(CUDPReceiverImpl&&) = delete;
      CUDPReceiverImpl(const CUDPReceiverImpl&) = delete;
      CUDPReceiverImpl& operator=(const CUDPReceiverImpl&) = delete;

      virtual bool AddMultiCastGroup(const char* ipaddr_) = 0;
      virtual bool RemMultiCastGroup(const char* ipaddr_) = 0;

      virtual size_t Receive(char* buf_, size_t len_, int timeout_, ::sockaddr_in* address_) = 0;
    };

    class CUDPReceiver
    {
    public:
      CUDPReceiver();

      bool Create(const SReceiverAttr& attr_);
      bool Destroy();

      bool AddMultiCastGroup(const char* ipaddr_);
      bool RemMultiCastGroup(const char* ipaddr_);

      size_t Receive(char* buf_, size_t len_, int timeout_, ::sockaddr_in* address_ = nullptr);

    protected:
      bool m_use_npcap;
      std::mutex                        m_socket_mtx;
      std::shared_ptr<CUDPReceiverImpl> m_socket_impl;
    };
  }
}
