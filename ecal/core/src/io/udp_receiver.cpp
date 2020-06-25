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

#include <iostream>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4834)
#endif
#include <asio.hpp>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "udp_receiver.h"

#ifdef ECAL_NPCAP_SUPPORT
#include "ecal_config_hlp.h"
#include <udpcap/npcap_helpers.h>
#include <udpcap/udpcap_socket.h>
#endif //ECAL_NPCAP_SUPPORT

namespace eCAL
{
  ////////////////////////////////////////////////////////
  // udp receiver class implementation
  ////////////////////////////////////////////////////////
  class CUDPReceiverImpl
  {
  public:
    CUDPReceiverImpl(const SReceiverAttr& attr_);

    bool AddMultiCastGroup(const char* ipaddr_);

    size_t Receive(char* buf_, size_t len_, int timeout_, ::sockaddr_in* address_ = nullptr);

  protected:
    void RunIOContext(const asio::chrono::steady_clock::duration& timeout);
      
    bool                    m_broadcast;
    bool                    m_unicast;
    asio::io_context        m_iocontext;
    asio::ip::udp::socket   m_socket;
    asio::ip::udp::endpoint m_sender_endpoint;
  };

  CUDPReceiverImpl::CUDPReceiverImpl(const SReceiverAttr& attr_) :
                      m_broadcast(attr_.broadcast),
                      m_unicast(attr_.unicast),
                      m_socket(m_iocontext)
  {
    if (m_broadcast && m_unicast)
    {
      std::cerr << "CUDPReceiver: Setting broadcast and unicast option true is not allowed." << std::endl;
      return;
    }

    // create socket
    asio::ip::udp::endpoint listen_endpoint(asio::ip::udp::v4(), static_cast<unsigned short>(attr_.port));
    {
      asio::error_code ec;      
      m_socket.open(listen_endpoint.protocol());
      if (ec)
        std::cerr << "CUDPReceiver: Unable to open socket: " << ec.message() << std::endl;    
    }

    // set socket reuse
    {
      asio::error_code ec;                        
      m_socket.set_option(asio::ip::udp::socket::reuse_address(true), ec);
      if (ec)
        std::cerr << "CUDPReceiver: Unable to set reuse-address option: " << ec.message() << std::endl;        
    }

    // bind socket
    {
      asio::error_code ec;                  
      m_socket.bind(listen_endpoint, ec);
      if (ec)
        std::cerr << "CUDPReceiver: Unable to bind socket to " << listen_endpoint.address().to_string() << ":" << listen_endpoint.port() << ": " << ec.message() << std::endl;
    }

    if (!m_unicast)
    {
      // set loopback option
      asio::ip::multicast::enable_loopback loopback(attr_.loopback);
      asio::error_code ec;            
      m_socket.set_option(loopback, ec);
      if (ec)
        std::cerr << "CUDPReceiver: Unable to enable loopback: " << ec.message() << std::endl;
    }

    // set receive buffer size (default = 1 MB)
    {
      int rcvbuf = 1024 * 1024;
      if (attr_.rcvbuf > 0) rcvbuf = attr_.rcvbuf;
      asio::socket_base::receive_buffer_size recbufsize(rcvbuf);
      asio::error_code ec;      
      m_socket.set_option(recbufsize, ec);
      if (ec)
        std::cerr << "CUDPReceiver: Unable to set receive buffer size: " << ec.message() << std::endl;
    }

    // join multicast group
    AddMultiCastGroup(attr_.ipaddr.c_str());
  }

  bool CUDPReceiverImpl::AddMultiCastGroup(const char* ipaddr_)
  {
    if (!m_broadcast && !m_unicast)
    {
      // join multicast group
      {
        asio::error_code ec;
        m_socket.set_option(asio::ip::multicast::join_group(asio::ip::make_address(ipaddr_)), ec);
        if (ec)
          std::cerr << "CUDPReceiver: Unable to join multicast group: " << ec.message() << std::endl;
      }

#ifdef ECAL_JOIN_MULTICAST_TWICE
      // this is a very bad workaround because of an idendified bug on a specific embedded device
      // we join the multicast group multiple times otherwise the socket will not receive any data
      std::cerr << "eCAL was compiled with ECAL_JOIN_MULTICAST_TWICE" << std::endl;
      m_socket.set_option(asio::ip::multicast::leave_group(asio::ip::make_address(ipaddr_)));
      m_socket.set_option(asio::ip::multicast::join_group (asio::ip::make_address(ipaddr_)));
#endif // ECAL_JOIN_MULTICAST_TWICE
    }
    return(true);
  }

  size_t CUDPReceiverImpl::Receive(char* buf_, size_t len_, int timeout_, ::sockaddr_in* address_ /* = nullptr */)
  {
    size_t reclen(0);
    m_socket.async_receive_from(asio::buffer(buf_, len_), m_sender_endpoint,
      [&reclen](std::error_code ec, std::size_t length)
    {
      if (!ec)
      {
        reclen = length;
      }
    });

    // run for timeout ms
    RunIOContext(asio::chrono::milliseconds(timeout_));

    // retrive underlaying raw socket informations
    if (address_)
    {
      if (m_sender_endpoint.address().is_v4())
      {
        asio::detail::sockaddr_in4_type* in4 = reinterpret_cast<asio::detail::sockaddr_in4_type*>(m_sender_endpoint.data());
        address_->sin_addr   = in4->sin_addr;
        address_->sin_family = in4->sin_family;
        address_->sin_port   = in4->sin_port;
        memset(&(address_->sin_zero), 0, 8);
      }
      else
      {
        std::cout << "CUDPReceiver: ipv4 address conversion failed." << std::endl;
      }
    }

    return (reclen);
  }

  void CUDPReceiverImpl::RunIOContext(const asio::chrono::steady_clock::duration& timeout)
  {
    // restart the io_context, as it may have been left in the "stopped" state by a previous operation
    m_iocontext.restart();

    // block until the asynchronous operation has completed, or timed out
    m_iocontext.run_for(timeout);

    // stop the context if even the operation was not succesful completed
    if (!m_iocontext.stopped())
    {
      // cancel the outstanding asynchronous operation
      m_socket.cancel();

      // run the io_context again until the operation completes
      m_iocontext.run();
    }
  }

#ifdef ECAL_NPCAP_SUPPORT
  ////////////////////////////////////////////////////////
  // Npcap based receiver class implementation
  ////////////////////////////////////////////////////////
  class CUDPcapReceiverImpl
  {
  public:
    CUDPcapReceiverImpl(const SReceiverAttr& attr_);

    bool AddMultiCastGroup(const char* ipaddr_);

    size_t Receive(char* buf_, size_t len_, int timeout_, ::sockaddr_in* address_ = nullptr);

  protected:
    bool                 m_unicast;
    Udpcap::UdpcapSocket m_socket;
  };

  CUDPcapReceiverImpl::CUDPcapReceiverImpl(const SReceiverAttr& attr_)
    : m_unicast(attr_.unicast)
  {
    // set receive buffer size (default = 1 MB)
    int rcvbuf = 1024 * 1024;
    if (attr_.rcvbuf > 0)
      rcvbuf = attr_.rcvbuf;
    m_socket.setReceiveBufferSize(rcvbuf);

    // bind socket
    m_socket.bind(Udpcap::HostAddress::Any(), static_cast<uint16_t>(attr_.port));

    if (!m_unicast)
    {
      // set loopback option
      m_socket.setMulticastLoopbackEnabled(attr_.loopback);
    }

    // join multicast group
    AddMultiCastGroup(attr_.ipaddr.c_str());
  }

  bool CUDPcapReceiverImpl::AddMultiCastGroup(const char* ipaddr_)
  {
    if (!m_unicast)
    {
      // join multicast group
      return m_socket.joinMulticastGroup(Udpcap::HostAddress(ipaddr_));
    }
    return(true);
  }

  size_t CUDPcapReceiverImpl::Receive(char* buf_, size_t len_, int timeout_, ::sockaddr_in* address_ /* = nullptr */)
  {
    size_t bytes_received;
    if (address_)
    {
      Udpcap::HostAddress source_address;
      uint16_t source_port;
      bytes_received = m_socket.receiveDatagram(buf_, len_, static_cast<unsigned long>(timeout_), &source_address, &source_port);

      if (bytes_received && source_address.isValid())
      {
        address_->sin_addr.s_addr = source_address.toInt();
        address_->sin_family      = AF_INET;
        address_->sin_port        = source_port;
        memset(&(address_->sin_zero), 0, 8);
      }
    }
    else
    {
      bytes_received = m_socket.receiveDatagram(buf_, len_, static_cast<unsigned long>(timeout_));
    }
    return bytes_received;
  }

#endif //ECAL_NPCAP_SUPPORT

  ////////////////////////////////////////////////////////
  // udp receiver class
  ////////////////////////////////////////////////////////
  CUDPReceiver::CUDPReceiver()
    : CReceiver(CReceiver::SType_ReceiverUDP)
    , m_use_npcap(false)
  {
#ifdef ECAL_NPCAP_SUPPORT
    if (eCALPAR(NET, NPCAP_ENABLED))
    {
      m_use_npcap = Udpcap::Initialize(); // Only use NPCAP if we can initialize it (or it has already been initialized successfully)
      if (!m_use_npcap)
      {
        std::cerr << "Npcap is enabled, but cannot be initialized. Using socket fallback mode." << std::endl;
      }
    }
#endif //ECAL_NPCAP_SUPPORT
  }

  bool CUDPReceiver::Create(const SReceiverAttr& attr_)
  {
#ifdef ECAL_NPCAP_SUPPORT
    if (m_use_npcap)
    {
      if (m_udpcap_socket_impl) return false;
      m_udpcap_socket_impl = std::make_shared<CUDPcapReceiverImpl>(attr_);
      return true;
    }
#endif // ECAL_NPCAP_SUPPORT

    if (m_socket_impl) return(false);
    m_socket_impl = std::make_shared<CUDPReceiverImpl>(attr_);
    return(true);
  }

  bool CUDPReceiver::Destroy()
  {
#ifdef ECAL_NPCAP_SUPPORT
    if (m_use_npcap)
    {
      if (!m_udpcap_socket_impl) return false;
      m_udpcap_socket_impl = nullptr;
      return true;
    }
#endif // ECAL_NPCAP_SUPPORT

    if (!m_socket_impl) return(false);
    m_socket_impl = nullptr;
    return(true);
  }

  bool CUDPReceiver::AddMultiCastGroup(const char* ipaddr_)
  {
#ifdef ECAL_NPCAP_SUPPORT
    if (m_use_npcap)
    {
      if (!m_udpcap_socket_impl) return false;
      return m_udpcap_socket_impl->AddMultiCastGroup(ipaddr_);
    }
#endif // ECAL_NPCAP_SUPPORT

    if (!m_socket_impl) return(0);
    return(m_socket_impl->AddMultiCastGroup(ipaddr_));
  }

  size_t CUDPReceiver::Receive(char* buf_, size_t len_, int timeout_, ::sockaddr_in* address_ /* = nullptr */)
  {
#ifdef ECAL_NPCAP_SUPPORT
    if (m_use_npcap)
    {
      if (!m_udpcap_socket_impl) return(0);
      return(m_udpcap_socket_impl->Receive(buf_, len_, timeout_, address_));
    }
#endif // ECAL_NPCAP_SUPPORT

    if (!m_socket_impl) return(0);
    return(m_socket_impl->Receive(buf_, len_, timeout_, address_));
  }
}
