/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2024 Continental Corporation
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
 * @brief  eCAL registration provider
 *
 * All process internal publisher/subscriber, server/clients register here with all their attributes.
 *
 * These information will be send cyclic (registration refresh) via UDP to external eCAL processes.
 *
**/

#include "registration/udp/ecal_registration_sender_udp.h"

#include "serialization/ecal_serialize_sample_registration.h"
#include "io/udp/ecal_udp_configurations.h"
#include <ecal/ecal_config.h>

namespace
{
  using namespace eCAL;
  UDP::SSenderAttr CreateAttributes()
  {
    eCAL::UDP::SSenderAttr attr;
    attr.address = UDP::GetRegistrationAddress();
    attr.port = UDP::GetRegistrationPort();
    attr.ttl = UDP::GetMulticastTtl();
    attr.broadcast = UDP::IsBroadcast();
    attr.loopback = true;
    attr.sndbuf = UDP::GetSendBufferSize();
    return attr;
  }

}

namespace eCAL
{
  CRegistrationSenderUDP::CRegistrationSenderUDP()
    : m_reg_sample_snd(CreateAttributes())
  {
  }

  CRegistrationSenderUDP::~CRegistrationSenderUDP() = default;

  bool CRegistrationSenderUDP::SendSample(const Registration::Sample& sample_)
  {
    // serialize single sample
    if (SerializeToBuffer(sample_, m_sample_buffer))
    {
      // send single sample over udp
      return m_reg_sample_snd.Send("reg_sample", m_sample_buffer) != 0;
    }
    return false;
  }

  bool CRegistrationSenderUDP::SendSampleList(const Registration::SampleList& sample_list)
  {
    bool return_value{ true };
    for (const auto& sample : sample_list.samples)
    {
      return_value &= SendSample(sample);
    }
    return return_value;
  }
}