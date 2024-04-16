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

#include "ecal_udp_sample_receiver.h"
#include "io/udp/ecal_udp_configurations.h"

namespace eCAL
{
  namespace UDP
  {
    CSampleReceiver::CSampleReceiver(const SReceiverAttr& attr_, const HasSampleCallbackT& has_sample_callback_, const ApplySampleCallbackT& apply_sample_callback_)
    {
#ifdef ECAL_CORE_NPCAP_SUPPORT
      if (eCAL::UDP::IsNpcapEnabled())
      {
        m_receiver_npcap = std::make_shared<CSampleReceiverNpcap>(attr_, has_sample_callback_, apply_sample_callback_);
      }
      else
#endif
      {
        m_receiver_asio = std::make_shared<CSampleReceiverAsio>(attr_, has_sample_callback_, apply_sample_callback_);
      }
    }

    bool CSampleReceiver::AddMultiCastGroup(const char* ipaddr_)
    {
      if (m_receiver_asio)  return m_receiver_asio->AddMultiCastGroup(ipaddr_);

#ifdef ECAL_CORE_NPCAP_SUPPORT
      if (m_receiver_npcap) return m_receiver_npcap->AddMultiCastGroup(ipaddr_);
#endif

      return false;
    }

    bool CSampleReceiver::RemMultiCastGroup(const char* ipaddr_)
    {
      if (m_receiver_asio)  return m_receiver_asio->RemMultiCastGroup(ipaddr_);

#ifdef ECAL_CORE_NPCAP_SUPPORT
      if (m_receiver_npcap) return m_receiver_npcap->RemMultiCastGroup(ipaddr_);
#endif
      return false;
    }
  }
}
