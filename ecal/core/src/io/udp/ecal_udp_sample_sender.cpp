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
 * @brief  UDP sample sender to send messages of type eCAL::Sample
**/

#include "ecal_udp_sample_sender.h"
#include "io/udp/ecal_udp_configurations.h"

#include <array>
#include <iostream>
#include <memory>

namespace eCAL
{
  namespace UDP
  {
    CSampleSender::CSampleSender(const SSenderAttr& attr_) :
      m_destination_endpoint(asio::ip::make_address(attr_.address), static_cast<unsigned short>(attr_.port))
    {
      m_io_context = std::make_unique<asio::io_context>();

      // create the socket and set all socket options
      InitializeSocket(attr_);
    }

    CSampleSender::~CSampleSender()
    {
      // close socket
      asio::error_code ec;
      m_socket->close(ec);
      if (ec)
      {
        std::cerr << "CSampleSender: Error closing socket: " << ec.message() << '\n';
      }
    }

    void CSampleSender::InitializeSocket(const SSenderAttr& attr_)
    {
      // create socket
      m_socket = std::make_unique<ecaludp::Socket>(*m_io_context, GeteCALDatagramHeader());

      // open socket
      {
        asio::error_code ec;
        m_socket->open(m_destination_endpoint.protocol(), ec);
        if (ec)
          std::cout << "CSampleSender: Error opening socket: " << ec.message() << '\n';
      }

      if (attr_.broadcast)
      {
        // set unicast packet TTL
        const asio::ip::unicast::hops ttl(attr_.ttl);
        asio::error_code ec;
        m_socket->set_option(ttl, ec);
        if (ec)
          std::cerr << "CSampleSender: Setting TTL failed: " << ec.message() << '\n';
      }
      else
      {
        // set multicast packet TTL
        {
          const asio::ip::multicast::hops ttl(attr_.ttl);
          asio::error_code ec;
          m_socket->set_option(ttl, ec);
          if (ec)
            std::cerr << "CSampleSender: Setting TTL failed: " << ec.message() << '\n';
        }

        // set loopback option
        {
          const asio::ip::multicast::enable_loopback loopback(attr_.loopback);
          asio::error_code ec;
          m_socket->set_option(loopback, ec);
          if (ec)
            std::cerr << "CSampleSender: Error setting loopback option: " << ec.message() << '\n';
        }
      }

      if (attr_.broadcast)
      {
        asio::error_code ec;
        m_socket->set_option(asio::socket_base::broadcast(true), ec);
        if (ec)
          std::cerr << "CSampleSender: Setting broadcast mode failed: " << ec.message() << '\n';
      }

      // set limit for the data length (maximum size is imposed by the underlying IPv4 protocol)
      // 64*1024 - 20 /* IP header */ - 8 /* UDP header */ - 1 /* don't ask */
      m_socket->set_max_udp_datagram_size(64 * 1024 - 8 - 20 - 1);
    }

    size_t CSampleSender::Send(const std::string& sample_name_, const std::vector<char>& serialized_sample_)
    {
      // ------------------------------------------------
      // emulate old protocol
      // 
      // s1 = size of the sample name
      // s2 = size of the serialized sample payload
      // 
      //  2 Bytes sample name size (unsigned short)
      // s1 Bytes sample name
      // s2 Bytes serialized sample
      // ------------------------------------------------
      const unsigned short s1 = static_cast<unsigned short>(sample_name_.size()) + 1 /*'\0'*/;
      const size_t         s2 = serialized_sample_.size();
      const asio::const_buffer sample_name_size_asio_buffer(&s1, 2);
      const asio::const_buffer sample_name_asio_buffer(sample_name_.c_str(), s1); // we need to use c_str() here to guarantee  trailling \'0'
      const asio::const_buffer serialized_sample_asio_buffer(serialized_sample_.data(), s2);

      const asio::socket_base::message_flags flags(0);
      asio::error_code ec;
      const size_t sent = m_socket->send_to({ sample_name_size_asio_buffer, sample_name_asio_buffer, serialized_sample_asio_buffer }, m_destination_endpoint, flags, ec);
      if (ec)
      {
        std::cout << "CSampleSender::Send failed with: \'" << ec.message() << "\'" << '\n';
        return 0;
      }
      return sent;
    }
  }
}
