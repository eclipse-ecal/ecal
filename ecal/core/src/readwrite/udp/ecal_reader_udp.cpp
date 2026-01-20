/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
 * Copyright 2025 AUMOVIO and subsidiaries. All rights reserved.
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

#include <ecal/config.h>

#include "ecal_reader_udp.h"
#include "ecal_global_accessors.h"

#include "io/udp/ecal_udp_configurations.h"
#include "config/builder/udp_attribute_builder.h"
#include <ecal_serialize_sample_payload.h>

#include <functional>
#include <memory>
#include <string>
#include <utility>

namespace eCAL
{

  void CUDPReaderLayer::AddSubscription(const std::string& /*host_name_*/, const std::string& topic_name_, const EntityIdT& /*topic_id_*/)
  {
    if (!m_started)
    {      
      // start payload sample receiver

      m_started = true;
    }

    // we use udp broadcast in local mode
    if (m_attributes.broadcast) return;

    // add topic name based multicast address
    const std::string mcast_address = UDP::GetTopicPayloadAddress(topic_name_);
    if (m_topic_name_mcast_map.find(mcast_address) == m_topic_name_mcast_map.end())
    {
      m_topic_name_mcast_map.emplace(std::pair<std::string, int>(mcast_address, 0));
      m_payload_receiver->AddMultiCastGroup(mcast_address.c_str());
    }
    m_topic_name_mcast_map[mcast_address]++;
  }

  void CUDPReaderLayer::RemSubscription(const std::string& /*host_name_*/, const std::string& topic_name_, const EntityIdT& /*topic_id_*/)
  {
    // we use udp broadcast in local mode
    if (m_attributes.broadcast) return;

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
  

  CUDPReaderLayer::CUDPReaderLayer(const eCAL::eCALReader::UDP::SAttributes& attr_)
    : CTransportLayerInstance(LayerType::UDP)
    , m_attributes(attr_)
  {
    m_payload_receiver = std::make_unique<UDP::CSampleReceiver>(
      eCALReader::UDP::ConvertToIOUDPReceiverAttributes(m_attributes),
      [](const std::string& sample_name_) {return true; },
      [this](const char* serialized_sample_data_, size_t serialized_sample_size_) { return OnUDPMessage(serialized_sample_data_, serialized_sample_size_); }
    );

  }

  bool CUDPReaderLayer::AcceptsConnection(const PublisherConnectionParameters& publisher, const SubscriberConnectionParameters& subscriber) const
  {
    return false;
  }

  CTransportLayerInstance::ConnectionToken CUDPReaderLayer::AddConnection(const PublisherConnectionParameters& publisher, const ReceiveCallbackT& on_data, const ConnectionChangeCallback& on_connection_changed)
  {





    auto on_remove_subscription = [this, publisher](const std::string& /*host_name_*/, const std::string& topic_name_, const EntityIdT& /*topic_id_*/)
    {
      
    };
    return CTransportLayerInstance::ConnectionToken::Make(on_remove_subscription);
  }


  void CUDPReaderLayer::OnUDPMessage(const char* serialized_sample_data_, size_t serialized_sample_size_)
  {
    Payload::Sample ecal_sample;
    if (!DeserializeFromBuffer(serialized_sample_data_, serialized_sample_size_, ecal_sample)) return false;

    size_t applied_size(0);
    switch (ecal_sample.cmd_type)
    {
    case bct_set_sample:
    {
#ifndef NDEBUG
      // check layer
      if (layer_ == eTLayerType::tl_none)
      {
        // log it
        Logging::Log(Logging::log_level_error, ecal_sample.topic_info.topic_name + " : payload received without layer definition !");
      }
#endif

      // extract payload
      const char* payload_addr = nullptr;
      size_t      payload_size = 0;
      switch (ecal_sample.content.payload.type)
      {
      case eCAL::Payload::pl_raw:
        payload_addr = ecal_sample.content.payload.raw_addr;
        payload_size = ecal_sample.content.payload.raw_size;
        break;
      case eCAL::Payload::pl_vec:
        payload_addr = ecal_sample.content.payload.vec.data();
        payload_size = ecal_sample.content.payload.vec.size();
        break;
      default:
        break;
      }

    // Find correct callback
    auto publisher_callback_information = m_receive_callbacks.find(ecal_sample.topic_info.topic_id);
    if (publisher_callback_information != m_receive_callbacks.end())
    {
      const auto& callback_data = publisher_callback_information->second;
      SReceiveCallbackData data;
      data.buffer = payload_addr;
      data.buffer_size = payload_size;
      data.send_clock = ecal_sample.content.clock;
      data.send_timestamp = ecal_sample.content.time;
      // id & hash currently unused
      // Call data callback
      callback_data.data_callback(callback_data.id, callback_data.data_type_info, data);
    }

  }
}
