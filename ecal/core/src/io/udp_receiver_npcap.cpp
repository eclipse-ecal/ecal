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
#include <io/udp_receiver_npcap.h>

namespace
{
  bool isLocalAddress(Udpcap::HostAddress& address)
  {
    // Check if the address is a loopback address.
    if (address.isLoopback())
    {
      return true;
    }

    // Check if the address is in the private IP address ranges.
    // For IPv4, the private address ranges are 10.0.0.0/8, 172.16.0.0/12, and 192.168.0.0/16.
    if (address.isValid())
    {
      uint32_t ip = address.toInt();
      return ((ip & 0xFF000000) == 0x0A000000) || // 10.0.0.0/8
             ((ip & 0xFFF00000) == 0xAC100000) || // 172.16.0.0/12
             ((ip & 0xFFFF0000) == 0xC0A80000);   // 192.168.0.0/16
    }

    return false;
  }
}

namespace eCAL
{
  ////////////////////////////////////////////////////////
  // Npcap based receiver class implementation
  ////////////////////////////////////////////////////////
  CUDPReceiverPcap::CUDPReceiverPcap(const SReceiverAttr& attr_)
    : CUDPReceiverBase(attr_)
    , m_created(false)
    , m_localhost(attr_.localhost)
    , m_unicast(attr_.unicast)
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

    if (!m_unicast)
    {
      // set loopback option
      m_socket.setMulticastLoopbackEnabled(attr_.loopback);
    }

    // join multicast group
    AddMultiCastGroup(attr_.ipaddr.c_str());

    // state successful creation
    m_created = true;
  }

  bool CUDPReceiverPcap::AddMultiCastGroup(const char* ipaddr_)
  {
    if (!m_unicast)
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
    if (!m_unicast)
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

    // receive datagram
    Udpcap::HostAddress source_address;
    uint16_t source_port;
    size_t bytes_received = m_socket.receiveDatagram(buf_, len_, static_cast<unsigned long>(timeout_), &source_address, &source_port);

    // did we receive anything ?
    if (bytes_received == 0)
    {
      return 0;
    }

    // is the caller interested in the source address ?
    if (address_)
    {
      if (source_address.isValid())
      {
        address_->sin_addr.s_addr = source_address.toInt();
        address_->sin_family = AF_INET;
        address_->sin_port   = source_port;
        memset(&(address_->sin_zero), 0, 8);
      }
      else
      {
        std::cout << "CUDPReceiverPcap: source address conversion failed." << std::endl;
      }
    }

    // are we running in 'local host only' receiving mode ?
    if (m_localhost)
    {
      // if this is not a local address, return 0
      if (!isLocalAddress(source_address))
      {
        return 0;
      }
    }
  
    return bytes_received;
  }
}
