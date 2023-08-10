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
    size_t Send(const void* buf_, size_t len_, const char* ipaddr_ = nullptr);

  protected:
    bool                    m_broadcast;
    bool                    m_unicast;
    asio::io_context        m_iocontext;
    asio::ip::udp::endpoint m_endpoint;
    asio::ip::udp::socket   m_socket;
    unsigned short          m_port;
  };

  CUDPSenderImpl::CUDPSenderImpl(const SSenderAttr& attr_) :
    m_broadcast(attr_.broadcast),
    m_unicast(attr_.unicast),
    m_endpoint(asio::ip::make_address(attr_.ipaddr), static_cast<unsigned short>(attr_.port)),
    m_socket(m_iocontext, m_endpoint.protocol()),
    m_port(static_cast<unsigned short>(attr_.port))
  {
    if (m_broadcast && m_unicast)
    {
      std::cerr << "CUDPSender: Setting broadcast and unicast option true is not allowed." << std::endl;
      return;
    }

    if (m_broadcast || m_unicast)
    {
      // set unicast packet TTL
      const asio::ip::unicast::hops ttl(attr_.ttl);
      asio::error_code ec;
      m_socket.set_option(ttl, ec);
      if (ec)
        std::cerr << "CUDPSender: Setting TTL failed: " << ec.message() << std::endl;
    }
    else
    {
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

    if (m_broadcast)
    {
      asio::error_code ec;
      m_socket.set_option(asio::socket_base::broadcast(true), ec);
      if (ec)
        std::cerr << "CUDPSender: Setting broadcast mode failed: " << ec.message() << std::endl;
    }
  }

  size_t CUDPSenderImpl::Send(const void* buf_, const size_t len_, const char* ipaddr_)
  {
    const asio::socket_base::message_flags flags(0);
    asio::error_code                 ec;
    size_t                           sent(0);
    if ((ipaddr_ != nullptr) && (ipaddr_[0] != '\0')) sent = m_socket.send_to(asio::buffer(buf_, len_), asio::ip::udp::endpoint(asio::ip::make_address(ipaddr_), m_port), flags, ec);
    else                                              sent = m_socket.send_to(asio::buffer(buf_, len_), m_endpoint, flags, ec);
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

  size_t CUDPSender::Send(const void* buf_, const size_t len_, const char* ipaddr_)
  {
    if (!m_socket_impl) return(0);
    return(m_socket_impl->Send(buf_, len_, ipaddr_));
  }
}
