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
 * @brief  UDP logging receiver to receive messages of type eCAL::pb::LogMessage
**/

#include <ecal/ecal.h>
#include <ecal/ecal_config.h>

#include "ecal_def.h"
#include "io/udp/msg_type.h"
#include "io/udp/udp_configurations.h"

#include "rcv_logging.h"

namespace
{
  bool IsLocalHost(const eCAL::pb::LogMessage &ecal_message_)
  {
    const std::string host_name = ecal_message_.hname();
    if (host_name.empty()) return (false);
    if (host_name == eCAL::Process::GetHostName()) return (true);
    return (false);
  }
}

namespace eCAL
{
  static bool IsLocalHost(const eCAL::pb::LogMessage& ecal_message_)
  {
    const std::string host_name = ecal_message_.hname();
    if (host_name.empty())                   return(false);
    if (host_name == Process::GetHostName()) return(true);
    return(false);
  }

  CUDPLoggingReceiver::CUDPLoggingReceiver(LogMessageCallbackT log_cb_) :
    m_network_mode(false), m_log_message_callback(log_cb_)
  {
    // set network attributes
    SReceiverAttr attr;
    attr.address   = UDP::GetLoggingAddress();
    attr.port      = UDP::GetLoggingPort();
    attr.broadcast = !Config::IsNetworkEnabled();
    attr.loopback  = true;
    attr.rcvbuf    = Config::GetUdpMulticastRcvBufSizeBytes();

    // create udp logging receiver
    m_udp_receiver.Create(attr);

    // start logging receiver thread
    m_receive_thread = std::thread(&CUDPLoggingReceiver::ReceiveThread, this);

    // allocate receive buffer
    m_msg_buffer.resize(MSG_BUFFER_SIZE);
  }

  CUDPLoggingReceiver::~CUDPLoggingReceiver()
  {
    m_receive_thread_stop.store(true, std::memory_order_release);
    m_receive_thread.join();
  }

  void CUDPLoggingReceiver::SetNetworkMode(bool network_mode_)
  {
    m_network_mode = network_mode_;
  }

  void CUDPLoggingReceiver::ReceiveThread()
  {
    while (!m_receive_thread_stop.load(std::memory_order_acquire))
    {
      // wait for any incoming message
      const size_t recv_len = m_udp_receiver.Receive(m_msg_buffer.data(), m_msg_buffer.size(), CMN_UDP_RECEIVE_THREAD_CYCLE_TIME_MS);
      if (recv_len > 0)
      {
        m_log_message.Clear();
        if (m_log_message.ParseFromArray(m_msg_buffer.data(), static_cast<int>(recv_len)))
        {
          if (IsLocalHost(m_log_message) || m_network_mode)
          {
            m_log_message_callback(m_log_message);
          }
        }
      }
    }
  }
}
