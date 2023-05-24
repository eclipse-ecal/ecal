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
 * @brief  Sender thread for ecal samples
**/

#pragma once

#include <memory>

#include "udp_sender.h"

#ifdef _MSC_VER
#pragma warning(push, 0) // disable proto warnings
#endif
#include <ecal/core/pb/ecal.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

namespace eCAL
{
  class CSampleSender
  {
  public:
    CSampleSender(std::shared_ptr<eCAL::CUDPSender> udp_sender_, const std::string& ipaddr_) :
      m_udp_sender(udp_sender_),
      m_ipaddr(ipaddr_)
    {
    }

    size_t SendSample(const std::string& sample_name_, const eCAL::pb::Sample& ecal_sample_, long bandwidth_);

  private:
    std::shared_ptr<eCAL::CUDPSender> m_udp_sender;
    std::string                       m_ipaddr;

    std::vector<char>                 m_payload;
  };
}
