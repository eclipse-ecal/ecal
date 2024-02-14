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

#pragma once

#include "ecal_def.h"

#include <chrono>
#include <cstdint>
#include <vector>

namespace IO
{
  namespace UDP
  {
    class CMsgDefragmentation
    {
    public:
      CMsgDefragmentation();
      virtual ~CMsgDefragmentation();

      int ApplyMessage(const struct SUDPMessage& ecal_message_);

      bool HasFinished() { return((m_recv_mode == rcm_aborted) || (m_recv_mode == rcm_completed)); };
      bool HasTimedOut(const std::chrono::duration<double>& diff_time_) { m_timeout += diff_time_; return(m_timeout >= std::chrono::milliseconds(NET_UDP_RECBUFFER_TIMEOUT)); };

      int32_t GetMessageTotalLength() const   { return(m_message_total_len); };
      int32_t GetMessageCurrentLength() const { return(m_message_curr_len); };

      virtual int OnMessageCompleted(std::vector<char>&& msg_buffer_) = 0;

    protected:
      int OnMessageStart(const struct SUDPMessage& ecal_message_);
      int OnMessageData(const struct SUDPMessage& ecal_message_);

      enum eReceiveMode
      {
        rcm_waiting = 1,
        rcm_reading,
        rcm_aborted,
        rcm_completed
      };

      std::chrono::duration<double> m_timeout;
      std::vector<char>             m_recv_buffer;
      eReceiveMode                  m_recv_mode;

      int32_t                       m_message_id;
      int32_t                       m_message_total_num;
      int32_t                       m_message_total_len;

      int32_t                       m_message_curr_num;
      int32_t                       m_message_curr_len;
    };
  }
}
