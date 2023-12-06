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

#include <ecal/ecal_process.h>

#include "ecal_udp_logging_receiver.h"
#include "io/udp/fragmentation/msg_type.h"

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
  namespace UDP
  {
    CLoggingReceiver::CLoggingReceiver(const IO::UDP::SReceiverAttr& attr_, LogMessageCallbackT log_message_callback_) :
      m_network_mode(!attr_.broadcast), m_log_message_callback(log_message_callback_)
    {
      // create udp receiver
      m_udp_receiver.Create(attr_);

      // allocate receive buffer
      m_msg_buffer.resize(MSG_BUFFER_SIZE);

      // start receiver thread
      m_udp_receiver_thread = std::make_shared<CCallbackThread>(std::bind(&CLoggingReceiver::ReceiveThread, this));
      m_udp_receiver_thread->start(std::chrono::milliseconds(0));
    }

    CLoggingReceiver::~CLoggingReceiver()
    {
      m_udp_receiver_thread->stop();
    }

    void CLoggingReceiver::ReceiveThread()
    {
      // wait for any incoming message
      const size_t recv_len = m_udp_receiver.Receive(m_msg_buffer.data(), m_msg_buffer.size(), CMN_UDP_RECEIVE_THREAD_CYCLE_TIME_MS);
      if (recv_len > 0)
      {
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
