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

#include "config/builder/udp_attribute_builder.h"
#include <ecal_serialize_sample_payload.h>

#include <functional>
#include <memory>
#include <string>
#include <utility>

namespace eCAL
{
 
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
    return LayerEnabledForPublisherAndSubscriber(m_layer_type, publisher, subscriber);
  }

  CTransportLayerInstance::ConnectionToken CUDPReaderLayer::AddConnection(const PublisherConnectionParameters& publisher, const ReceiveCallbackT& on_data, const ConnectionChangeCallback& on_connection_changed)
  {
    const std::string topic_name = publisher.GetTopicName();

    if (!m_attributes.broadcast)
    {
      auto new_multicast_address = m_mcast_address_tracker.AddTopic(topic_name);
      if (new_multicast_address.has_value())
      {
        m_payload_receiver->AddMultiCastGroup(new_multicast_address.value().c_str());
      }
    }

    auto publisher_callback_handle = m_callback_storage.AddCallback(
      publisher.GetTopicId(),
      publisher.GetDataTypeInformation(),
      on_data);

    auto on_remove_subscription = [this, topic_name, publisher_callback_handle]()
    {
      m_callback_storage.RemoveCallback(publisher_callback_handle);

      if (!m_attributes.broadcast)
      {
        auto removed_multicast_address = m_mcast_address_tracker.RemoveTopic(topic_name);
        if (removed_multicast_address.has_value())
        {
          m_payload_receiver->RemMultiCastGroup(removed_multicast_address.value().c_str());
        }
      }
    };
    return CTransportLayerInstance::ConnectionToken::Make(on_remove_subscription);
  }

  void CUDPReaderLayer::OnUDPMessage(const char* serialized_sample_data_, size_t serialized_sample_size_)
  {
    Payload::Sample ecal_sample;
    if (!DeserializeFromBuffer(serialized_sample_data_, serialized_sample_size_, ecal_sample)) return;

    switch (ecal_sample.cmd_type)
    {
    case bct_set_sample:
    {
      /*
#ifndef NDEBUG
      // check layer
      if (layer_ == eTLayerType::tl_none)
      {
        // log it
        Logging::Log(Logging::log_level_error, ecal_sample.topic_info.topic_name + " : payload received without layer definition !");
      }
#endif
*/

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

      SReceiveCallbackData data;
      data.buffer = payload_addr;
      data.buffer_size = payload_size;
      data.send_clock = ecal_sample.content.clock;
      data.send_timestamp = ecal_sample.content.time;

      m_callback_storage.Invoke(ecal_sample.topic_info.topic_id, data);
    }
    default:
      return;
    }
  }
}
