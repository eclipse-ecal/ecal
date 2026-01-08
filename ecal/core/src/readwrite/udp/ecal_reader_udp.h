/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
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

#pragma once

#include "io/udp/ecal_udp_sample_receiver.h"
#include "readwrite/ecal_reader_layer.h"
#include "config/attributes/reader_udp_attributes.h"

#include "io/udp/ecal_udp_configurations.h"

#include <cassert>
#include <cstddef>
#include <optional>
#include <map>
#include <memory>
#include <string>

namespace eCAL
{
  ////////////////
  // LAYER
  ////////////////
  class CCallbackStorage
  {
  public:
    using Handle = EntityIdT;

    Handle AddCallback(const STopicId& id_, const SDataTypeInformation& data_type_info_, const ReceiveCallbackT& data_callback_)
    {
      m_receive_callbacks[id_.topic_id.entity_id] = { id_, data_type_info_, data_callback_ };
      return id_.topic_id.entity_id;
    }

    void RemoveCallback(Handle handle)
    {
      m_receive_callbacks.erase(handle);
    }

    void Invoke(EntityIdT id, const SReceiveCallbackData& data)
    {
      auto it = m_receive_callbacks.find(id);
      if (it != m_receive_callbacks.end())
      {
        const auto& publisher_id = it->second.id;
        const auto& publisher_data_type_info = it->second.data_type_info;
        const auto& data_callback = it->second.data_callback;
        data_callback(publisher_id, publisher_data_type_info, data);
      }
    }

  private:
    struct PublisherReceiveCallbackData
    {
      STopicId                    id;
      SDataTypeInformation        data_type_info;
      ReceiveCallbackT            data_callback;
    };
    std::map<EntityIdT, PublisherReceiveCallbackData> m_receive_callbacks;
  };

  class MulticastAddressTracker
  {
  public:
    std::optional<std::string> AddTopic(const std::string& topic_name)
    {
      const std::string mcast_address = UDP::GetTopicPayloadAddress(topic_name);
      if (m_topic_name_mcast_map.find(mcast_address) == m_topic_name_mcast_map.end())
      {
        m_topic_name_mcast_map.emplace(std::pair<std::string, int>(mcast_address, 0));
        m_topic_name_mcast_map[mcast_address]++;
        return mcast_address;
      }
      m_topic_name_mcast_map[mcast_address]++;
      return std::nullopt;
    }

    std::optional<std::string> RemoveTopic(const std::string& topic_name)
    {
      const std::string mcast_address = UDP::GetTopicPayloadAddress(topic_name);
      auto it = m_topic_name_mcast_map.find(mcast_address);
      assert(it != m_topic_name_mcast_map.end());

      m_topic_name_mcast_map[mcast_address]--;
      if (m_topic_name_mcast_map[mcast_address] == 0)
      {
        m_topic_name_mcast_map.erase(mcast_address);
        return mcast_address;
      }
      return std::nullopt;
    }

  private:
    using McastAddress = std::string;
    using NumberParticipants = int;
    std::map<McastAddress, NumberParticipants> m_topic_name_mcast_map;
  };

  class CUDPReaderLayer : CTransportLayerInstance
  {
  public:
    CUDPReaderLayer(const eCAL::eCALReader::UDP::SAttributes& attr_);
    ~CUDPReaderLayer() override = default;

    bool AcceptsConnection(const PublisherConnectionParameters& publisher, const SubscriberConnectionParameters& subscriber) const override;
    CTransportLayerInstance::ConnectionToken AddConnection(const PublisherConnectionParameters& publisher, const ReceiveCallbackT& on_data, const ConnectionChangeCallback& on_connection_changed) override;

  private:
    void OnUDPMessage(const char* serialized_sample_data_, size_t serialized_sample_size_);

    // global, per process UDP attributes
    eCAL::eCALReader::UDP::SAttributes     m_attributes;
    std::unique_ptr<UDP::CSampleReceiver>  m_payload_receiver;
    MulticastAddressTracker                m_mcast_address_tracker;
    CCallbackStorage                       m_callback_storage;
  };

}
