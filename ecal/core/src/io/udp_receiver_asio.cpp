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


#include <io/udp_receiver_asio.h>

#ifdef __linux__
#include "linux/ecal_socket_option_linux.h"
#endif

namespace eCAL
{
  ////////////////////////////////////////////////////////
  // Default ASIO based receiver class implementation
  ////////////////////////////////////////////////////////
  CUDPReceiverAsio::CUDPReceiverAsio(const SReceiverAttr& attr_) :
    CUDPReceiverBase(attr_),
    m_created(false),
    m_broadcast(attr_.broadcast),
    m_unicast(attr_.unicast),
    m_socket(m_iocontext)
  {
    if (m_broadcast && m_unicast)
    {
      std::cerr << "CUDPReceiverAsio: Setting broadcast and unicast option true is not allowed." << std::endl;
      return;
    }

    // create socket
    const asio::ip::udp::endpoint listen_endpoint(asio::ip::udp::v4(), static_cast<unsigned short>(attr_.port));
    {
      asio::error_code ec;
      m_socket.open(listen_endpoint.protocol(), ec);
      if (ec)
      {
        std::cerr << "CUDPReceiverAsio: Unable to open socket: " << ec.message() << std::endl;
        return;
      }
    }

    // set socket reuse
    {
      asio::error_code ec;
      m_socket.set_option(asio::ip::udp::socket::reuse_address(true), ec);
      if (ec)
      {
        std::cerr << "CUDPReceiverAsio: Unable to set reuse-address option: " << ec.message() << std::endl;
      }
    }

    // bind socket
    {
      asio::error_code ec;
      m_socket.bind(listen_endpoint, ec);
      if (ec)
      {
        std::cerr << "CUDPReceiverAsio: Unable to bind socket to " << listen_endpoint.address().to_string() << ":" << listen_endpoint.port() << ": " << ec.message() << std::endl;
        return;
      }
    }

    if (!m_unicast)
    {
      // set loopback option
      const asio::ip::multicast::enable_loopback loopback(attr_.loopback);
      asio::error_code ec;
      m_socket.set_option(loopback, ec);
      if (ec)
      {
        std::cerr << "CUDPReceiverAsio: Unable to enable loopback: " << ec.message() << std::endl;
      }
    }

    // set receive buffer size (default = 1 MB)
    {
      int rcvbuf = 1024 * 1024;
      if (attr_.rcvbuf > 0) rcvbuf = attr_.rcvbuf;
      const asio::socket_base::receive_buffer_size recbufsize(rcvbuf);
      asio::error_code ec;
      m_socket.set_option(recbufsize, ec);
      if (ec)
      {
        std::cerr << "CUDPReceiverAsio: Unable to set receive buffer size: " << ec.message() << std::endl;
      }
    }

    // join multicast group
    AddMultiCastGroup(attr_.ipaddr.c_str());

    // state successful creation
    m_created = true;
  }

  bool CUDPReceiverAsio::AddMultiCastGroup(const char* ipaddr_)
  {
    if (!m_broadcast && !m_unicast)
    {
      // join multicast group
#ifdef __linux__
      if (eCAL::Config::IsUdpMulticastJoinAllIfEnabled())
      {
        if (!set_socket_mcast_group_option(m_socket.native_handle(), ipaddr_, MCAST_JOIN_GROUP))
        {
          return(false);
        }
      }
      else
#endif
      {
        asio::error_code ec;
        m_socket.set_option(asio::ip::multicast::join_group(asio::ip::make_address(ipaddr_)), ec);
        if (ec)
        {
          std::cerr << "CUDPReceiverAsio: Unable to join multicast group: " << ec.message() << std::endl;
          return(false);
        }
      }

#ifdef ECAL_JOIN_MULTICAST_TWICE
      // this is a very bad workaround because of an identified bug on a specific embedded device
      // we join the multicast group multiple times otherwise the socket will not receive any data
      std::cerr << "eCAL was compiled with ECAL_JOIN_MULTICAST_TWICE" << std::endl;
      m_socket.set_option(asio::ip::multicast::leave_group(asio::ip::make_address(ipaddr_)));
      m_socket.set_option(asio::ip::multicast::join_group(asio::ip::make_address(ipaddr_)));
#endif // ECAL_JOIN_MULTICAST_TWICE
    }
    return(true);
  }

  bool CUDPReceiverAsio::RemMultiCastGroup(const char* ipaddr_)
  {
    if (!m_broadcast && !m_unicast)
    {
      // Leave multicast group
#ifdef __linux__
      if (eCAL::Config::IsUdpMulticastJoinAllIfEnabled())
      {
        if (!set_socket_mcast_group_option(m_socket.native_handle(), ipaddr_, MCAST_LEAVE_GROUP))
        {
          return(false);
        }
      }
      else
#endif
      {
        asio::error_code ec;
        m_socket.set_option(asio::ip::multicast::leave_group(asio::ip::make_address(ipaddr_)), ec);
        if (ec)
        {
          std::cerr << "CUDPReceiverAsio: Unable to leave multicast group: " << ec.message() << std::endl;
          return(false);
        }
      }
    }
    return(true);
  }

  size_t CUDPReceiverAsio::Receive(char* buf_, size_t len_, int timeout_, ::sockaddr_in* address_ /* = nullptr */)
  {
    if (!m_created) return 0;

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

    // retrieve underlaying raw socket informations
    if (address_)
    {
      if (m_sender_endpoint.address().is_v4())
      {
        asio::detail::sockaddr_in4_type* in4 = reinterpret_cast<asio::detail::sockaddr_in4_type*>(m_sender_endpoint.data());
        address_->sin_addr = in4->sin_addr;
        address_->sin_family = in4->sin_family;
        address_->sin_port = in4->sin_port;
        memset(&(address_->sin_zero), 0, 8);
      }
      else
      {
        std::cout << "CUDPReceiverAsio: ipv4 address conversion failed." << std::endl;
      }
    }

    return (reclen);
  }

  void CUDPReceiverAsio::RunIOContext(const asio::chrono::steady_clock::duration& timeout)
  {
    // restart the io_context, as it may have been left in the "stopped" state by a previous operation
    m_iocontext.restart();

    // block until the asynchronous operation has completed, or timed out
    m_iocontext.run_for(timeout);

    // stop the context if even the operation was not successful completed
    if (!m_iocontext.stopped())
    {
      // cancel the outstanding asynchronous operation
      m_socket.cancel();

      // run the io_context again until the operation completes
      m_iocontext.run();
    }
  }
}
