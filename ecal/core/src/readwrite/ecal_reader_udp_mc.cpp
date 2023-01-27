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
 * @brief  udp multicast reader and layer
**/

#include "readwrite/ecal_reader_udp_mc.h"

#include "ecal_global_accessors.h"
#include "pubsub/ecal_subgate.h"

#include "io/udp_configurations.h"

namespace eCAL
{
  ////////////////
  // READER
  ////////////////
  bool CDataReaderUDP::HasSample(const std::string& sample_name_)
  {
    if (!g_subgate()) return(false);
    return(g_subgate()->HasSample(sample_name_));
  }

  size_t CDataReaderUDP::ApplySample(const eCAL::pb::Sample& ecal_sample_, eCAL::pb::eTLayerType layer_)
  {
    if (!g_subgate()) return 0;
    return g_subgate()->ApplySample(ecal_sample_, layer_);
  }

  ////////////////
  // LAYER
  ////////////////
  CUDPReaderLayer::CUDPReaderLayer() : 
                   started(false)
  {};

  CUDPReaderLayer::~CUDPReaderLayer()
  {
    thread.Stop();
  };

  void CUDPReaderLayer::Initialize()
  {
    SReceiverAttr attr;
    attr.ipaddr = Config::GetUdpMulticastGroup();
    attr.port = Config::GetUdpMulticastPort() + NET_UDP_MULTICAST_PORT_SAMPLE_OFF;
    attr.unicast = false;
    attr.loopback = true;
    attr.rcvbuf = Config::GetUdpMulticastRcvBufSizeBytes();
    rcv.Create(attr);
  }

  void CUDPReaderLayer::AddSubscription(const std::string& /*host_name_*/, const std::string& topic_name_, const std::string& /*topic_id_*/, QOS::SReaderQOS /*qos_*/)
  {
    if (!started)
    {
      thread.Start(0, std::bind(&CDataReaderUDP::Receive, &reader, &rcv));
      started = true;
    }
    // add topic name based multicast address
    std::string mcast_address = UDP::GetTopicMulticastAddress(topic_name_);
    if (topic_name_mcast_map.find(mcast_address) == topic_name_mcast_map.end())
    {
      topic_name_mcast_map.emplace(std::pair<std::string, int>(mcast_address, 0));
      rcv.AddMultiCastGroup(mcast_address.c_str());
    }
    topic_name_mcast_map[mcast_address]++;
  }

  void CUDPReaderLayer::RemSubscription(const std::string& /*host_name_*/, const std::string& topic_name_, const std::string& /*topic_id_*/)
  {
    std::string mcast_address = UDP::GetTopicMulticastAddress(topic_name_);
    if (topic_name_mcast_map.find(mcast_address) == topic_name_mcast_map.end())
    {
      // this should never happen
    }
    else
    {
      topic_name_mcast_map[mcast_address]--;
      if (topic_name_mcast_map[mcast_address] == 0)
      {
        rcv.RemMultiCastGroup(mcast_address.c_str());
        topic_name_mcast_map.erase(mcast_address);
      }
    }
  }
};
