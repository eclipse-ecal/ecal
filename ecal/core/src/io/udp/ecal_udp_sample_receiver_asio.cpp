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
 * @brief  UDP sample receiver to receive messages of type eCAL::Sample
**/

#include "ecal_udp_sample_receiver_asio.h"
#include "io/udp/ecal_udp_configurations.h"

#ifdef __linux__
#include "linux/socket_os.h"
#endif

#include <array>
#include <iostream>

namespace eCAL
{
  namespace UDP
  {
    CSampleReceiverAsio::CSampleReceiverAsio(const SReceiverAttr& attr_, const HasSampleCallbackT& has_sample_callback_, const ApplySampleCallbackT& apply_sample_callback_) :
      CSampleReceiverBase(attr_, has_sample_callback_, apply_sample_callback_)
    {
      // initialize io context
      m_io_context = std::make_unique<asio::io_context>();
      m_work       = std::make_unique<asio::io_context::work>(*m_io_context);

      // create the socket and set all socket options
      InitializeSocket(attr_);

      // join multicast group
      JoinMultiCastGroup(attr_.address.c_str());

      // run the io context
      m_io_thread = std::thread([this] { m_io_context->run(); });

      // start receiving
      Receive();
    }

    CSampleReceiverAsio::~CSampleReceiverAsio()
    {
      // cancel async socket operations
      asio::error_code ec;
      m_socket->cancel(ec);
      if (ec)
      {
        std::cerr << "CSampleReceiverAsio: Error cancelling socket: " << ec.message() << '\n';
      }

      // stop io context
      m_io_context->stop();
      if (m_io_thread.joinable())
        m_io_thread.join();
    }

    bool CSampleReceiverAsio::AddMultiCastGroup(const char* ipaddr_)
    {
      return JoinMultiCastGroup(ipaddr_);
    }

    bool CSampleReceiverAsio::RemMultiCastGroup(const char* ipaddr_)
    {
      if (!m_broadcast)
      {
        // Leave multicast group
#ifdef __linux__
        if (eCAL::UDP::IsUdpMulticastJoinAllIfEnabled())
        {
          if (!IO::UDP::set_socket_mcast_group_option(m_socket->native_handle(), ipaddr_, MCAST_LEAVE_GROUP))
          {
            return(false);
          }
        }
        else
#endif
        {
          asio::error_code ec;
          m_socket->set_option(asio::ip::multicast::leave_group(asio::ip::make_address(ipaddr_)), ec); // NOLINT(*-unused-return-value)
          if (ec)
          {
            std::cerr << "CUDPReceiverAsio: Unable to leave multicast group: " << ec.message() << '\n';
            return(false);
          }
        }
      }
      return(true);
    }

    void CSampleReceiverAsio::InitializeSocket(const SReceiverAttr& attr_)
    {
      // create socket
      m_socket = std::make_unique<ecaludp::Socket>(*m_io_context, GeteCALDatagramHeader());

      // open socket
      const asio::ip::udp::endpoint listen_endpoint(asio::ip::udp::v4(), static_cast<unsigned short>(attr_.port));
      {
        asio::error_code ec;
        m_socket->open(listen_endpoint.protocol(), ec); // NOLINT(*-unused-return-value)
        if (ec)
        {
          std::cerr << "CSampleReceiverAsio: Unable to open socket: " << ec.message() << '\n';
          return;
        }
      }

      // set socket reuse
      {
        asio::error_code ec;
        m_socket->set_option(asio::ip::udp::socket::reuse_address(true), ec); // NOLINT(*-unused-return-value)
        if (ec)
        {
          std::cerr << "CSampleReceiverAsio: Unable to set reuse-address option: " << ec.message() << '\n';
        }
      }

      // set loopback option
      {
        const asio::ip::multicast::enable_loopback loopback(attr_.loopback);
        asio::error_code ec;
        m_socket->set_option(loopback, ec); // NOLINT(*-unused-return-value)
        if (ec)
        {
          std::cerr << "CSampleReceiverAsio: Unable to enable loopback: " << ec.message() << '\n';
        }
      }

      // set receive buffer size (default = 1 MB)
      {
        int rcvbuf = 1024 * 1024;
        if (attr_.rcvbuf > 0) rcvbuf = attr_.rcvbuf;
        const asio::socket_base::receive_buffer_size recbufsize(rcvbuf);
        asio::error_code ec;
        m_socket->set_option(recbufsize, ec); // NOLINT(*-unused-return-value)
        if (ec)
        {
          std::cerr << "CSampleReceiverAsio: Unable to set receive buffer size: " << ec.message() << '\n';
        }
      }
      // bind socket
      {
        asio::error_code ec;
        m_socket->bind(listen_endpoint, ec); // NOLINT(*-unused-return-value)
        if (ec)
        {
          std::cerr << "CSampleReceiverAsio: Unable to bind socket to " << listen_endpoint.address().to_string() << ":" << listen_endpoint.port() << ": " << ec.message() << '\n';
          return;
        }
      }
    }

    bool CSampleReceiverAsio::JoinMultiCastGroup(const char* ipaddr_)
    {
      if (!m_broadcast)
      {
#ifdef __linux__
        // TODO: NPCAP - How to implement this ? native_handle() not provided
        if (eCAL::UDP::IsUdpMulticastJoinAllIfEnabled())
        {
          if (!IO::UDP::set_socket_mcast_group_option(m_socket->native_handle(), ipaddr_, MCAST_JOIN_GROUP))
          {
            return(false);
          }
        }
        else
#endif
        {
          asio::error_code ec;
          m_socket->set_option(asio::ip::multicast::join_group(asio::ip::make_address(ipaddr_)), ec); // NOLINT(*-unused-return-value)
          if (ec)
          {
            std::cerr << "CUDPReceiverAsio: Unable to join multicast group: " << ec.message() << '\n';
            return(false);
          }
        }
      }
      return(true);
    }

    void CSampleReceiverAsio::Receive()
    {
      m_socket->async_receive_from(m_sender_endpoint,
        [this](const std::shared_ptr<ecaludp::OwningBuffer>& buffer, asio::error_code ec)
        {
          // triggered by m_socket->cancel in destructor
          if (ec == asio::error::operation_aborted)
          {
            asio::error_code ec_close_op;
            m_socket->close(ec_close_op);
            if (ec_close_op)
            {
              std::cerr << "CSampleReceiverAsio: Error closing socket: " << ec_close_op.message() << '\n';
            }

            return;
          }

          if (ec)
          {
            std::cerr << "CSampleReceiverAsio: Error receiving: " << ec.message() << '\n';
            return;
          }

          // extract data from the buffer
          const char* receive_buffer = static_cast<const char*>(buffer->data());
          bool processed = true;

          // read sample_name size
          unsigned short sample_name_size = 0;
          memcpy(&sample_name_size, receive_buffer, 2);

          // check for damaged data
          if (sample_name_size > buffer->size())
          {
            std::cerr << "CSampleReceiverAsio: Received damaged data. Wrong sample name size." << '\n';
            processed = false;
          }
          else
          {
            // read sample_name
            const std::string sample_name(receive_buffer + sizeof(sample_name_size));

            // calculate payload offset
            auto payload_offset = sizeof(sample_name_size) + sample_name_size;

            // check for damaged data
            if (payload_offset > buffer->size())
            {
              std::cerr << "CSampleReceiverAsio: Received damaged data. Wrong payload buffer offset." << '\n';
              processed = false;
            }
            else if (m_has_sample_callback(sample_name)) // if we are interested in the sample payload
            {
              // extract payload and its size
              const char* payload_buffer = receive_buffer + payload_offset;
              auto payload_buffer_size = buffer->size() - payload_offset;

              // apply the sample payload
              m_apply_sample_callback(payload_buffer, payload_buffer_size);
            }
          }

          // recursively call Receive() to continue listening for data
          if (processed)
          {
            this->Receive();
          }
        });
    }
  }
}
