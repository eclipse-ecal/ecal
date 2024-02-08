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

#include "udp_receiver.h"
#include "io/udp/ecal_udp_configurations.h"

#include "udp_receiver_asio.h"
#include <cstddef>
#include <memory>
#include <mutex>
#ifdef ECAL_CORE_NPCAP_SUPPORT
#include "udp_receiver_npcap.h"
#endif

#include <iostream>

namespace IO
{
  namespace UDP
  {
    ////////////////////////////////////////////////////////
    // udp receiver class
    ////////////////////////////////////////////////////////
    CUDPReceiver::CUDPReceiver()
      : m_use_npcap(false)
    {
#ifdef ECAL_CORE_NPCAP_SUPPORT
      if (eCAL::UDP::IsNpcapEnabled())
      {
        m_use_npcap = Udpcap::Initialize(); // Only use NPCAP if we can initialize it (or it has already been initialized successfully)
        if (!m_use_npcap)
        {
          std::cerr << "Npcap is enabled, but cannot be initialized. Using socket fallback mode." << std::endl;
        }
      }
#endif //ECAL_CORE_NPCAP_SUPPORT
    }

    bool CUDPReceiver::Create(const SReceiverAttr& attr_)
    {
      if (m_socket_impl) return false;

#ifdef ECAL_CORE_NPCAP_SUPPORT
      if (m_use_npcap)
      {
        m_socket_impl = std::make_shared<CUDPReceiverPcap>(attr_);
        return true;
      }
#endif // ECAL_CORE_NPCAP_SUPPORT

      m_socket_impl = std::make_shared<CUDPReceiverAsio>(attr_);
      return(true);
    }

    bool CUDPReceiver::Destroy()
    {
      if (!m_socket_impl) return(false);

      const std::lock_guard<std::mutex> lock(m_socket_mtx);
      m_socket_impl.reset();

      return(true);
    }

    bool CUDPReceiver::AddMultiCastGroup(const char* ipaddr_)
    {
      if (!m_socket_impl) return(false);

      const std::lock_guard<std::mutex> lock(m_socket_mtx);
      return(m_socket_impl->AddMultiCastGroup(ipaddr_));
    }

    bool CUDPReceiver::RemMultiCastGroup(const char* ipaddr_)
    {
      if (!m_socket_impl) return(false);

      const std::lock_guard<std::mutex> lock(m_socket_mtx);
      return(m_socket_impl->RemMultiCastGroup(ipaddr_));
    }

    size_t CUDPReceiver::Receive(char* buf_, size_t len_, int timeout_, ::sockaddr_in* address_ /* = nullptr */)
    {
      if (!m_socket_impl) return(0);

      const std::lock_guard<std::mutex> lock(m_socket_mtx);
      return(m_socket_impl->Receive(buf_, len_, timeout_, address_));
    }
  }
}
