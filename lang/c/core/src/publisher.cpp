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
 * @file   publisher.cpp
 * @brief  eCAL publisher c interface
**/

#include <ecal/ecal.h>
#include <ecal/v5/ecal_publisher.h>
#include <ecal_c/pubsub/publisher.h>

#include "common.h"

#include <algorithm>

#if ECAL_CORE_PUBLISHER
namespace
{
  void Convert_Publisher_Configuration(eCAL::Publisher::Configuration& publisher_configuration_, const struct eCAL_Publisher_Configuration* publisher_configuration_c_)
  {
    publisher_configuration_.layer.shm.enable = static_cast<bool>(publisher_configuration_c_->layer.shm.enable);
    publisher_configuration_.layer.shm.zero_copy_mode = static_cast<bool>(publisher_configuration_c_->layer.shm.zero_copy_mode);
    publisher_configuration_.layer.shm.acknowledge_timeout_ms = publisher_configuration_c_->layer.shm.acknowledge_timeout_ms;
    publisher_configuration_.layer.shm.memfile_buffer_count = publisher_configuration_c_->layer.shm.memfile_buffer_count;
    publisher_configuration_.layer.shm.memfile_min_size_bytes = publisher_configuration_c_->layer.shm.memfile_min_size_bytes;
    publisher_configuration_.layer.shm.memfile_reserve_percent = publisher_configuration_c_->layer.shm.memfile_reserve_percent;
    publisher_configuration_.layer.udp.enable = static_cast<bool>(publisher_configuration_c_->layer.udp.enable);
    publisher_configuration_.layer.tcp.enable = static_cast<bool>(publisher_configuration_c_->layer.tcp.enable);

    static const std::map<eCAL_TransportLayer_eType, eCAL::TransportLayer::eType> transport_layer_type_map
    {
      {eCAL_TransportLayer_eType_none, eCAL::TransportLayer::eType::none},
      {eCAL_TransportLayer_eType_shm, eCAL::TransportLayer::eType::shm},
      {eCAL_TransportLayer_eType_udp_mc, eCAL::TransportLayer::eType::udp_mc},
      {eCAL_TransportLayer_eType_tcp, eCAL::TransportLayer::eType::tcp}
    };

    publisher_configuration_.layer_priority_local.resize(publisher_configuration_c_->layer_priority_local_length);
    std::transform(publisher_configuration_c_->layer_priority_local,
      publisher_configuration_c_->layer_priority_local + publisher_configuration_c_->layer_priority_local_length,
      publisher_configuration_.layer_priority_local.begin(),
      [](const auto& key) { return transport_layer_type_map.at(key); });

    publisher_configuration_.layer_priority_remote.resize(publisher_configuration_c_->layer_priority_remote_length);
    std::transform(publisher_configuration_c_->layer_priority_remote,
      publisher_configuration_c_->layer_priority_remote + publisher_configuration_c_->layer_priority_remote_length,
      publisher_configuration_.layer_priority_remote.begin(),
      [](const auto& key) { return transport_layer_type_map.at(key); });
    
  }

  void Convert_Publisher_Configuration(struct eCAL_Publisher_Configuration* publisher_configuration_c_, const eCAL::Publisher::Configuration& publisher_configuration_)
  {
    publisher_configuration_c_->layer.shm.enable = static_cast<int>(publisher_configuration_.layer.shm.enable);
    publisher_configuration_c_->layer.shm.zero_copy_mode = static_cast<int>(publisher_configuration_.layer.shm.zero_copy_mode);
    publisher_configuration_c_->layer.shm.acknowledge_timeout_ms = publisher_configuration_.layer.shm.acknowledge_timeout_ms;
    publisher_configuration_c_->layer.shm.memfile_buffer_count = publisher_configuration_.layer.shm.memfile_buffer_count;
    publisher_configuration_c_->layer.shm.memfile_min_size_bytes = publisher_configuration_.layer.shm.memfile_min_size_bytes;
    publisher_configuration_c_->layer.shm.memfile_reserve_percent = publisher_configuration_.layer.shm.memfile_reserve_percent;
    publisher_configuration_c_->layer.udp.enable = static_cast<int>(publisher_configuration_.layer.udp.enable);
    publisher_configuration_c_->layer.tcp.enable = static_cast<int>(publisher_configuration_.layer.tcp.enable);

    static const std::map<eCAL::TransportLayer::eType, eCAL_TransportLayer_eType> transport_layer_type_map
    {
      {eCAL::TransportLayer::eType::none, eCAL_TransportLayer_eType_none},
      {eCAL::TransportLayer::eType::shm, eCAL_TransportLayer_eType_shm},
      {eCAL::TransportLayer::eType::udp_mc, eCAL_TransportLayer_eType_udp_mc},
      {eCAL::TransportLayer::eType::tcp, eCAL_TransportLayer_eType_tcp}
    };

    publisher_configuration_c_->layer_priority_local = reinterpret_cast<eCAL_TransportLayer_eType*>(std::malloc(sizeof(eCAL_TransportLayer_eType) * publisher_configuration_.layer_priority_local.size()));
    if (publisher_configuration_c_->layer_priority_local != NULL)
    {
      std::transform(publisher_configuration_.layer_priority_local.begin(),
        publisher_configuration_.layer_priority_local.end(),
        publisher_configuration_c_->layer_priority_local,
        [](const auto& key) { return transport_layer_type_map.at(key); });
    }

    publisher_configuration_c_->layer_priority_remote = reinterpret_cast<eCAL_TransportLayer_eType*>(std::malloc(sizeof(eCAL_TransportLayer_eType) * publisher_configuration_.layer_priority_remote.size()));
    if (publisher_configuration_c_->layer_priority_remote != NULL)
    {
      std::transform(publisher_configuration_.layer_priority_remote.begin(),
        publisher_configuration_.layer_priority_remote.end(),
        publisher_configuration_c_->layer_priority_remote,
        [](const auto& key) { return transport_layer_type_map.at(key); });
    }
  }

  void Convert_SPubEventCallbackData(struct eCAL_SPubEventCallbackData* pub_event_callback_data_c_, const eCAL::SPubEventCallbackData& pub_event_callback_data_)
  {
    static const std::map<eCAL::ePublisherEvent, eCAL_ePublisherEvent> publisher_event_map
    {
        {eCAL::ePublisherEvent::none, eCAL_ePublisherEvent_none},
        {eCAL::ePublisherEvent::connected, eCAL_ePublisherEvent_connected},
        {eCAL::ePublisherEvent::disconnected, eCAL_ePublisherEvent_disconnected},
        {eCAL::ePublisherEvent::dropped, eCAL_ePublisherEvent_dropped}
    };

    pub_event_callback_data_c_->event_type = publisher_event_map.at(pub_event_callback_data_.event_type);
    pub_event_callback_data_c_->event_time = pub_event_callback_data_.event_time;
    Convert_SDataTypeInformation(&pub_event_callback_data_c_->subscriber_datatype, pub_event_callback_data_.subscriber_datatype);
  }

  void Assign_SPubEventCallbackData(struct eCAL_SPubEventCallbackData* pub_event_callback_data_c_, const eCAL::SPubEventCallbackData& pub_event_callback_data_)
  {
    static const std::map<eCAL::ePublisherEvent, eCAL_ePublisherEvent> publisher_event_map
    {
        {eCAL::ePublisherEvent::none, eCAL_ePublisherEvent_none},
        {eCAL::ePublisherEvent::connected, eCAL_ePublisherEvent_connected},
        {eCAL::ePublisherEvent::disconnected, eCAL_ePublisherEvent_disconnected},
        {eCAL::ePublisherEvent::dropped, eCAL_ePublisherEvent_dropped}
    };

    pub_event_callback_data_c_->event_type = publisher_event_map.at(pub_event_callback_data_.event_type);
    pub_event_callback_data_c_->event_time = pub_event_callback_data_.event_time;
    Assign_SDataTypeInformation(&pub_event_callback_data_c_->subscriber_datatype, pub_event_callback_data_.subscriber_datatype);
  }

  void eCAL_SPubEventCallbackData_Free(eCAL_SPubEventCallbackData* pub_event_callback_data_)
  {
    std::free(reinterpret_cast<void*>(pub_event_callback_data_));
  }

  class PayloadWriter : public eCAL::CPayloadWriter
  {
  public:
    PayloadWriter(const struct eCAL_PayloadWriter* payload_writer_) : m_payload_writer(payload_writer_)
    {
    }

    bool WriteFull(void* buffer_, size_t size_) final
    {
      return !m_payload_writer->WriteFull(buffer_, size_);
    }

    bool WriteModified(void* buffer_, size_t size_) final
    {
      if (m_payload_writer->WriteModified != NULL)
        return !m_payload_writer->WriteModified(buffer_, size_);
      else
        return WriteFull(buffer_, size_);
    }

    size_t GetSize() final
    {
      return m_payload_writer->GetSize();
    }

  private:
    const struct eCAL_PayloadWriter* m_payload_writer;
  };
}

struct eCAL_Publisher
{
  eCAL::CPublisher* handle;
};

extern "C"
{
  ECALC_API eCAL_Publisher* eCAL_Publisher_New(const char* topic_name_, const struct eCAL_SDataTypeInformation* data_type_information_, const struct eCAL_Publisher_Configuration* publisher_configuration_)
  {
    eCAL::SDataTypeInformation data_type_information;
    eCAL::Publisher::Configuration publisher_configuration = eCAL::GetPublisherConfiguration();

    if (data_type_information_ != NULL)
      Convert_SDataTypeInformation(data_type_information, data_type_information_);
    if (publisher_configuration_ != NULL)
      Convert_Publisher_Configuration(publisher_configuration, publisher_configuration_);

    return new eCAL_Publisher{ new eCAL::CPublisher(topic_name_, data_type_information, publisher_configuration) };
  }

  ECALC_API eCAL_Publisher* eCAL_Publisher_New2(const char* topic_name_, const struct eCAL_SDataTypeInformation* data_type_information_, const eCAL_PubEventCallbackT pub_event_callback_, const struct eCAL_Publisher_Configuration* publisher_configuration_)
  {
    eCAL::SDataTypeInformation data_type_information;
    eCAL::Publisher::Configuration publisher_configuration = eCAL::GetPublisherConfiguration();

    Convert_SDataTypeInformation(data_type_information, data_type_information_);

    const auto pub_event_callback = [pub_event_callback_](const eCAL::STopicId& topic_id_, const eCAL::SPubEventCallbackData& pub_event_callback_data_)
    {
      struct eCAL_STopicId topic_id_c;
      struct eCAL_SPubEventCallbackData pub_event_callback_data_c;

      Assign_STopicId(&topic_id_c, topic_id_);
      Assign_SPubEventCallbackData(&pub_event_callback_data_c, pub_event_callback_data_);
      pub_event_callback_(&topic_id_c, &pub_event_callback_data_c);
    };

    if (publisher_configuration_ != NULL)
      Convert_Publisher_Configuration(publisher_configuration, publisher_configuration_);

    return new eCAL_Publisher{ new eCAL::CPublisher(topic_name_, data_type_information, pub_event_callback, publisher_configuration) };
  }

  ECALC_API void eCAL_Publisher_Delete(eCAL_Publisher* publisher_)
  {
    delete publisher_->handle;
    delete publisher_;
  }

  ECALC_API int eCAL_Publisher_Send(eCAL_Publisher* publisher_, const void* buffer_, size_t buffer_len_, long long timestamp_)
  {
    return static_cast<int>(!publisher_->handle->Send(buffer_, buffer_len_, timestamp_));
  }

  ECALC_API int eCAL_Publisher_Send2(eCAL_Publisher* publisher_, struct eCAL_PayloadWriter* payload_writer_, long long timestamp_)
  {
    PayloadWriter payload_writer(payload_writer_);
    return static_cast<int>(!publisher_->handle->Send(payload_writer, timestamp_));
  }

  ECALC_API size_t eCAL_Publisher_GetSubscriberCount(eCAL_Publisher* publisher_)
  {
    return publisher_->handle->GetSubscriberCount();
  }

  ECALC_API const char* eCAL_Publisher_GetTopicName(eCAL_Publisher* publisher_)
  {
    return publisher_->handle->GetTopicName().c_str();
  }

  ECALC_API struct eCAL_STopicId* eCAL_Publisher_GetTopicId(eCAL_Publisher* publisher_)
  {
    auto* topic_id = reinterpret_cast<eCAL_STopicId*>(std::malloc(sizeof(eCAL_STopicId)));
    if (topic_id != NULL)
      Convert_STopicId(topic_id, publisher_->handle->GetTopicId());
    return topic_id;
  }

  ECALC_API const struct eCAL_STopicId* eCAL_Publisher_GetTopicId_NoMalloc(eCAL_Publisher* publisher_, void* buffer, size_t* buffer_length_)
  {
    const auto topic_id = publisher_->handle->GetTopicId();
    const auto allocation_size = aligned_size(sizeof(eCAL_STopicId)) + ExtSize_STopicId(topic_id);
    
    struct eCAL_STopicId* topic_id_c = NULL;
    
    if (buffer_length_ == NULL)
    {
      topic_id_c = reinterpret_cast<struct eCAL_STopicId*>(std::malloc(allocation_size));
      if (topic_id_c != NULL)
      {
        auto* ptr = reinterpret_cast<char*>(topic_id_c) + aligned_size(sizeof(eCAL_STopicId));
        Convert_STopicId(topic_id_c, topic_id, &ptr);
      }
    }
    else
    {
      if (*buffer_length_ >= allocation_size)
      {
        topic_id_c = reinterpret_cast<struct eCAL_STopicId*>(buffer);
        auto* ptr = reinterpret_cast<char*>(topic_id_c) + aligned_size(sizeof(eCAL_STopicId));
        Convert_STopicId(topic_id_c, topic_id, &ptr);
      }
      else
        *buffer_length_ = allocation_size;
    }

    return topic_id_c;
  }

  ECALC_API struct eCAL_SDataTypeInformation* eCAL_Publisher_GetDataTypeInformation(eCAL_Publisher* publisher_)
  {
    auto* data_type_information = reinterpret_cast<eCAL_SDataTypeInformation*>(std::malloc(sizeof(eCAL_SDataTypeInformation)));
    if (data_type_information != NULL)
      Convert_SDataTypeInformation(data_type_information, publisher_->handle->GetDataTypeInformation());
    return data_type_information;
  }
}
#endif // ECAL_CORE_PUBLISHER
