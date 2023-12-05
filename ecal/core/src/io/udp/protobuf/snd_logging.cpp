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
 * @brief  UDP logging sender to send messages of type eCAL::pb::LogMessage
**/

#include "snd_logging.h"

namespace eCAL
{
  CLoggingSender::CLoggingSender(const SSenderAttr& attr_)
  {
    m_udp_sender = std::make_shared<CUDPSender>(attr_);
  }

  size_t CLoggingSender::Send(const eCAL::pb::LogMessage& ecal_log_message_)
  {
    if (!m_udp_sender) return(0);

    std::string ecal_log_message_s = ecal_log_message_.SerializeAsString();
    if (!ecal_log_message_s.empty())
    {
      return m_udp_sender->Send((void*)ecal_log_message_s.data(), ecal_log_message_s.size());
    }

    return 0;
  }
}
