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

#include "udp_receiver_npcap.h"

#include <iostream>

#include <udpcap/udpcap_version.h>

namespace IO
{
  namespace UDP
  {
    ////////////////////////////////////////////////////////
    // Npcap based receiver class implementation
    ////////////////////////////////////////////////////////
    CUDPReceiverPcap::CUDPReceiverPcap(const SReceiverAttr& attr_)
      : CUDPReceiverImpl(attr_)
      , m_created(false)
      , m_broadcast(attr_.broadcast)
    {
      // set receive buffer size (default = 1 MB)
      int rcvbuf = 1024 * 1024;
      if (attr_.rcvbuf > 0)
      {
        rcvbuf = attr_.rcvbuf;
      }
      if (!m_socket.setReceiveBufferSize(rcvbuf))
      {
        std::cerr << "CUDPReceiverPcap: Unable to set receive buffer size." << std::endl;
      }

      // bind socket
      if (!m_socket.bind(Udpcap::HostAddress::Any(), static_cast<uint16_t>(attr_.port)))
      {
        std::cerr << "CUDPReceiverPcap: Unable to bind socket." << std::endl;
        return;
      }

      // set loopback option
      if (!m_broadcast)
      {
        m_socket.setMulticastLoopbackEnabled(attr_.loopback);
      }

      // join multicast group
      AddMultiCastGroup(attr_.address.c_str());

      // state successful creation
      m_created = true;
    }

    CUDPReceiverPcap::~CUDPReceiverPcap()
    {
      // close socket
      m_socket.close();
      
      // state successful destruction
      m_created = false;
    }

    bool CUDPReceiverPcap::AddMultiCastGroup(const char* ipaddr_)
    {
      if (!m_broadcast)
      {
        // join multicast group
        if (!m_socket.joinMulticastGroup(Udpcap::HostAddress(ipaddr_)))
        {
          std::cerr << "CUDPReceiverPcap: Unable to join multicast group." << std::endl;
          return(false);
        }
      }
      return(true);
    }

    bool CUDPReceiverPcap::RemMultiCastGroup(const char* ipaddr_)
    {
      if (!m_broadcast)
      {
        // leave multicast group
        if (!m_socket.leaveMulticastGroup(Udpcap::HostAddress(ipaddr_)))
        {
          std::cerr << "CUDPReceiverPcap: Unable to leave multicast group." << std::endl;
          return(false);
        }
      }
      return(true);
    }

    size_t CUDPReceiverPcap::Receive(char* buf_, size_t len_, int timeout_, ::sockaddr_in* address_ /* = nullptr */)
    {
      if (!m_created) return 0;

      size_t bytes_received;
      if (address_)
      {
        Udpcap::HostAddress source_address;
        uint16_t source_port;

#if UDPCAP_VERSION_MAJOR == 1
        // Show a compiler deprecation warning
        // TODO: Remove for eCAL6
        [[deprecated("Udpcap 1.x is deprecated and prone to data-loss. Please update udpcap to 2.x.")]]

        bytes_received = m_socket.receiveDatagram(buf_, len_, static_cast<unsigned long>(timeout_), &source_address, &source_port);

        if (bytes_received && source_address.isValid())
        {
          address_->sin_addr.s_addr = source_address.toInt();
          address_->sin_family = AF_INET;
          address_->sin_port = source_port;
          memset(&(address_->sin_zero), 0, 8);
        }
#else // Udpcap 2.x
        Udpcap::Error error(Udpcap::Error::GENERIC_ERROR);
        bytes_received = m_socket.receiveDatagram(buf_, len_, timeout_, &source_address, &source_port, error);

        if (!error)
        {
          address_->sin_addr.s_addr = source_address.toInt();
          address_->sin_family = AF_INET;
          address_->sin_port = source_port;
          memset(&(address_->sin_zero), 0, 8);
        }
#endif

      }
      else
      {
#if UDPCAP_VERSION_MAJOR == 1
        // Show a compiler deprecation warning
        // TODO: Remove for eCAL6
        [[deprecated("Udpcap 1.x is deprecated and prone to data-loss. Please update udpcap to 2.x.")]]

        bytes_received = m_socket.receiveDatagram(buf_, len_, static_cast<unsigned long>(timeout_));
#else // Udpcap 2.x
        Udpcap::Error error(Udpcap::Error::GENERIC_ERROR);
        bytes_received = m_socket.receiveDatagram(buf_, len_, timeout_, error);
#endif
      }
      return bytes_received;
    }
  }
}
