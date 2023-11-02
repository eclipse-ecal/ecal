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
 * @brief  UDP sender class
**/

#include <iostream>
#include <functional>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4834)
#endif
#include <asio.hpp>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "udp_configurations.h"
#include "udp_sender.h"

namespace eCAL
{
  ////////////////////////////////////////////////////////
  // udp sender class implementation
  ////////////////////////////////////////////////////////
  class CUDPSenderImpl
  {
  public:
    CUDPSenderImpl(const SSenderAttr& attr_);

    size_t Send(const void* buf_, size_t len_);
    size_t Send(const void* buf_, size_t len_, const char* ipaddr_);

  protected:
    std::string           m_ipaddr;
    unsigned short        m_port;
    bool                  m_localhost;
    asio::io_context      m_iocontext;
    asio::ip::udp::socket m_socket;
  };

  CUDPSenderImpl::CUDPSenderImpl(const SSenderAttr& attr_) :
    m_ipaddr(attr_.ipaddr),
    m_port(static_cast<unsigned short>(attr_.port)),
    m_localhost(attr_.localhost),
    m_socket(m_iocontext)
  {
    if (attr_.localhost)
    {
      if (attr_.ipaddr != UDP::LocalHost())
      {
        std::cerr << "CUDPSenderImpl initialized with ip address: \"" << attr_.ipaddr << "\" in local host mode. Using \"" << UDP::LocalHost() << "\" instead." << std::endl;
        return;
      }

      // open socket
      {
        const asio::ip::udp::endpoint listen_endpoint(asio::ip::make_address(UDP::LocalHost()), 0);
        asio::error_code ec;
        m_socket.open(listen_endpoint.protocol(), ec);
        if (ec)
        {
          std::cerr << "CUDPReceiverAsio: Unable to open socket: " << ec.message() << std::endl;
          return;
        }
      }
    }
    else
    {
      // open socket
      {
        const asio::ip::udp::endpoint listen_endpoint(asio::ip::udp::v4(), 0);
        asio::error_code ec;
        m_socket.open(listen_endpoint.protocol(), ec);
        if (ec)
        {
          std::cerr << "CUDPReceiverAsio: Unable to open socket: " << ec.message() << std::endl;
          return;
        }
      }

      // set multicast packet TTL
      {
        const asio::ip::multicast::hops ttl(attr_.ttl);
        asio::error_code ec;
        m_socket.set_option(ttl, ec);
        if (ec)
          std::cerr << "CUDPSender: Setting TTL failed: " << ec.message() << std::endl;
      }

      // set loopback option
      {
        const asio::ip::multicast::enable_loopback loopback(attr_.loopback);
        asio::error_code ec;
        m_socket.set_option(loopback, ec);
        if (ec)
          std::cerr << "CUDPSender: Error setting loopback option: " << ec.message() << std::endl;
      }
    }

    // set socket reuse
    {
      asio::error_code ec;
      m_socket.set_option(asio::ip::udp::socket::reuse_address(true), ec);
      if (ec)
      {
        std::cerr << "CUDPSender: Unable to set reuse-address option: " << ec.message() << std::endl;
      }
    }
  }

  size_t CUDPSenderImpl::Send(const void* buf_, const size_t len_)
  {
    const asio::socket_base::message_flags flags(0);
    asio::error_code ec;
    size_t sent = m_socket.send_to(asio::buffer(buf_, len_), asio::ip::udp::endpoint(asio::ip::make_address(m_ipaddr), m_port), flags, ec);
    if (ec)
    {
      std::cout << "CUDPSender::Send failed with: \'" << ec.message() << "\'" << std::endl;
      return (0);
    }
    return(sent);
  }

  size_t CUDPSenderImpl::Send(const void* buf_, const size_t len_, const char* ipaddr_)
  {
    if (m_localhost)
    {
      std::cout << "CUDPSender::Send failed with explicit ip address: \"" << ipaddr_ << "\" in local host mode." << std::endl;
      return (0);
    }

    const asio::socket_base::message_flags flags(0);
    asio::error_code ec;
    size_t sent = m_socket.send_to(asio::buffer(buf_, len_), asio::ip::udp::endpoint(asio::ip::make_address(ipaddr_), m_port), flags, ec);
    if (ec)
    {
      std::cout << "CUDPSender::Send failed with: \'" << ec.message() << "\'" << std::endl;
      return (0);
    }
    return(sent);
  }

  ////////////////////////////////////////////////////////
  // udp sender class
  ////////////////////////////////////////////////////////
  CUDPSender::CUDPSender(const SSenderAttr& attr_)
  {
    m_socket_impl = std::make_shared<CUDPSenderImpl>(attr_);
  }

  size_t CUDPSender::Send(const void* buf_, const size_t len_)
  {
    if (!m_socket_impl) return(0);
    return(m_socket_impl->Send(buf_, len_));
  }

  size_t CUDPSender::Send(const void* buf_, const size_t len_, const char* ipaddr_)
  {
    if (!m_socket_impl) return(0);
    return(m_socket_impl->Send(buf_, len_, ipaddr_));
  }
}
