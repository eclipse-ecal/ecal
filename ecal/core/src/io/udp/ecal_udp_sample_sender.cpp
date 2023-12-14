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
 * @brief  UDP sample sender to send messages of type eCAL::pb::Sample
**/

#include "ecal_udp_sample_sender.h"
#include "io/udp/fragmentation/snd_fragments.h"

#include <ecal/ecal_log.h>

namespace
{
  size_t TransmitToUDP(const void* buf_, const size_t len_, const std::shared_ptr<IO::UDP::CUDPSender>& sample_sender_, const std::string& mcast_address_)
  {
    return (sample_sender_->Send(buf_, len_, mcast_address_.c_str()));
  }
}

namespace eCAL
{
  namespace UDP
  {
    CSampleSender::CSampleSender(const IO::UDP::SSenderAttr& attr_)
    {
      m_udp_sender = std::make_shared<IO::UDP::CUDPSender>(attr_);
    }

    size_t CSampleSender::Send(const std::string& sample_name_, const eCAL::pb::Sample& ecal_sample_, long bandwidth_)
    {
      if (!m_udp_sender) return(0);

      // return value
      size_t sent_sum(0);

      const size_t data_size = IO::UDP::CreateSampleBuffer(sample_name_, ecal_sample_, m_payload);
      if (data_size > 0)
      {
        // and send it
        sent_sum = SendFragmentedMessage(m_payload.data(), data_size, bandwidth_, std::bind(TransmitToUDP, std::placeholders::_1, std::placeholders::_2, m_udp_sender, m_attr.address));

#ifndef NDEBUG
        // log it
        eCAL::Logging::Log(log_level_debug4, "UDP Sample Buffer Sent (" + std::to_string(sent_sum) + " Bytes)");
#endif
      }

      // return bytes sent
      return(sent_sum);
    }
  }
}
