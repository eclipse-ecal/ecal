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
 * @brief  udp multicast reader and layer
**/

#include <ecal/ecal_config.h>

#include "ecal_reader_udp.h"
#include "ecal_global_accessors.h"

#include "io/udp/ecal_udp_configurations.h"
#include "pubsub/ecal_subgate.h"

#include <functional>
#include <memory>
#include <string>
#include <utility>

namespace eCAL
{
  ////////////////
  // LAYER
  ////////////////
  CUDPReaderLayer::CUDPReaderLayer() : 
                   m_started(false),
                   m_local_mode(false)
  {}

  CUDPReaderLayer::~CUDPReaderLayer() = default;

  void CUDPReaderLayer::Initialize()
  {
  }

  void CUDPReaderLayer::AddSubscription(const std::string& /*host_name_*/, const std::string& topic_name_, const std::string& /*topic_id_*/)
  {
    if (!m_started)
    {
      // set local mode
      m_local_mode = UDP::IsBroadcast();

      // set network attributes
      eCAL::UDP::SReceiverAttr attr;
      attr.address   = UDP::GetPayloadAddress();
      attr.port      = UDP::GetPayloadPort();
      attr.broadcast = UDP::IsBroadcast();
      attr.loopback  = true;
      attr.rcvbuf    = UDP::GetReceiveBufferSize();

      // start payload sample receiver
      m_payload_receiver = std::make_shared<UDP::CSampleReceiver>(attr, std::bind(&CUDPReaderLayer::HasSample, this, std::placeholders::_1), std::bind(&CUDPReaderLayer::ApplySample, this, std::placeholders::_1, std::placeholders::_2));

      m_started = true;
    }

    // we use udp broadcast in local mode
    if (m_local_mode) return;

    // add topic name based multicast address
    const std::string mcast_address = UDP::GetTopicPayloadAddress(topic_name_);
    if (m_topic_name_mcast_map.find(mcast_address) == m_topic_name_mcast_map.end())
    {
      m_topic_name_mcast_map.emplace(std::pair<std::string, int>(mcast_address, 0));
      m_payload_receiver->AddMultiCastGroup(mcast_address.c_str());
    }
    m_topic_name_mcast_map[mcast_address]++;
  }

  void CUDPReaderLayer::RemSubscription(const std::string& /*host_name_*/, const std::string& topic_name_, const std::string& /*topic_id_*/)
  {
    // we use udp broadcast in local mode
    if (m_local_mode) return;

    const std::string mcast_address = UDP::GetTopicPayloadAddress(topic_name_);
    if (m_topic_name_mcast_map.find(mcast_address) == m_topic_name_mcast_map.end())
    {
      // this should never happen
    }
    else
    {
      m_topic_name_mcast_map[mcast_address]--;
      if (m_topic_name_mcast_map[mcast_address] == 0)
      {
        m_payload_receiver->RemMultiCastGroup(mcast_address.c_str());
        m_topic_name_mcast_map.erase(mcast_address);
      }
    }
  }
  bool CUDPReaderLayer::HasSample(const std::string& sample_name_)
  {
    if (g_subgate() == nullptr) return(false);
    return(g_subgate()->HasSample(sample_name_));
  }

  bool CUDPReaderLayer::ApplySample(const char* serialized_sample_data_, size_t serialized_sample_size_)
  {
    if (g_subgate() == nullptr) return false;
    return g_subgate()->ApplySample(serialized_sample_data_, serialized_sample_size_, tl_ecal_udp);
  }
}
