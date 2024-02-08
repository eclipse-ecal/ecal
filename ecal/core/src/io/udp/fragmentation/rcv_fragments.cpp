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


#include "rcv_fragments.h"
#include "msg_type.h"

#include <chrono>
#include <ecal/ecal_log.h>

#include <cstring>
#include <string>
#include <utility>

namespace IO
{
  namespace UDP
  {
    CMsgDefragmentation::CMsgDefragmentation()
      : m_timeout(0.0)
      , m_recv_mode(rcm_waiting)
      , m_message_id(0)
      , m_message_total_num(0)
      , m_message_total_len(0)
      , m_message_curr_num(0)
      , m_message_curr_len(0)
    {
    }

    CMsgDefragmentation::~CMsgDefragmentation() = default;

    int CMsgDefragmentation::ApplyMessage(const struct SUDPMessage& ecal_message_)
    {
      // reset timeout
      m_timeout = std::chrono::duration<double>(0.0);

      // process current packet
      switch (ecal_message_.header.type)
      {
        // new message started
      case msg_type_header:
        OnMessageStart(ecal_message_);
        break;
        // message data package
      case msg_type_content:
        if (m_recv_mode == rcm_reading)
        {
          OnMessageData(ecal_message_);
        }
        break;
      }

      // we have a complete message in the receive buffer
      if (m_recv_mode == rcm_completed)
      {
        // call complete event
        OnMessageCompleted(std::move(m_recv_buffer));
      }

      return(0);
    }

    int CMsgDefragmentation::OnMessageStart(const struct SUDPMessage& ecal_message_)
    {
      // store header info
      m_message_id = ecal_message_.header.id;
      m_message_total_num = ecal_message_.header.num;
      m_message_total_len = ecal_message_.header.len;

      // reset current message states
      m_message_curr_num = 0;
      m_message_curr_len = 0;

      // prepare receive buffer
      m_recv_buffer.reserve(static_cast<size_t>(m_message_total_len));

      // switch to reading mode
      m_recv_mode = rcm_reading;

      return(0);
    }

    int CMsgDefragmentation::OnMessageData(const struct SUDPMessage& ecal_message_)
    {
      // check message id
      if (ecal_message_.header.id != m_message_id)
      {
#ifndef NDEBUG
        // log it
        eCAL::Logging::Log(log_level_debug3, "UDP Sample OnMessageData - WRONG MESSAGE PACKET ID " + std::to_string(ecal_message_.header.id));
#endif
        m_recv_mode = rcm_aborted;
        return(-1);
      }

      // check current packet counter
      if (ecal_message_.header.num != m_message_curr_num)
      {
#ifndef NDEBUG
        // log it
        eCAL::Logging::Log(log_level_debug3, "UDP Sample OnMessageData - WRONG MESSAGE PACKET NUMBER " + std::to_string(ecal_message_.header.num) + " / " + std::to_string(m_message_curr_num));
#endif
        m_recv_mode = rcm_aborted;
        return(-1);
      }

      // check current packet length
      if (ecal_message_.header.len <= 0)
      {
#ifndef NDEBUG
        // log it
        eCAL::Logging::Log(log_level_debug3, "UDP Sample OnMessageData - WRONG MESSAGE PACKET LENGTH " + std::to_string(ecal_message_.header.len));
#endif
        m_recv_mode = rcm_aborted;
        return(-1);
      }

      // copy the message part to the receive message buffer
      m_recv_buffer.resize(m_recv_buffer.size() + static_cast<size_t>(ecal_message_.header.len));
      memcpy(m_recv_buffer.data() + m_recv_buffer.size() - static_cast<size_t>(ecal_message_.header.len), ecal_message_.payload, static_cast<size_t>(ecal_message_.header.len));

      // increase packet counter
      m_message_curr_num++;

      // increase current length
      m_message_curr_len += ecal_message_.header.len;

      // last message packet ? -> switch to completed mode
      if (m_message_curr_num == m_message_total_num) m_recv_mode = rcm_completed;

      return(0);
    }
  }
}
