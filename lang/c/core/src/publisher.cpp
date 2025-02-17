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

#include <mutex>
#include <algorithm>

#if ECAL_CORE_PUBLISHER
namespace
{
  void Convert_SDataTypeInformation(eCAL::SDataTypeInformation& data_type_information_, const struct eCAL_SDataTypeInformation* data_type_information_c_)
  {
    data_type_information_.name = data_type_information_c_->name;
    data_type_information_.encoding = data_type_information_c_->encoding;
    data_type_information_.descriptor.assign(data_type_information_c_->descriptor, data_type_information_c_->descriptor_len);
  }

  void Convert_SDataTypeInformation(struct eCAL_SDataTypeInformation* data_type_information_c_, const eCAL::SDataTypeInformation& data_type_information_)
  {
    data_type_information_c_->name = data_type_information_.name.c_str();
    data_type_information_c_->encoding = data_type_information_.encoding.c_str();
    data_type_information_c_->descriptor = data_type_information_.descriptor.data();
    data_type_information_c_->descriptor_len = data_type_information_.descriptor.length();
  }

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

  void Convert_STopicId(struct eCAL_STopicId* topic_id_c_, const eCAL::STopicId& topic_id_)
  {
    topic_id_c_->topic_id.entity_id = topic_id_.topic_id.entity_id;
    topic_id_c_->topic_id.process_id = topic_id_.topic_id.process_id;
    topic_id_c_->topic_id.host_name = topic_id_.topic_id.host_name.c_str();
    topic_id_c_->topic_name = topic_id_.topic_name.c_str();
  }

  void Convert_SPubEventCallbackData(struct eCAL_SPubEventCallbackData* pub_event_callback_data_c_, const eCAL::SPubEventCallbackData& pub_event_callback_data_)
  {
    //TODO: 
  }

  char* Clone_CString(const char* c_string_)
  {
    char* cloned_c_string = NULL;
    if (c_string_ != NULL)
    {
      const auto c_string_len = std::strlen(c_string_);
      cloned_c_string = reinterpret_cast<char*>(std::malloc(c_string_len + 1));
      if (cloned_c_string != NULL)
        std::strcpy(cloned_c_string, c_string_);
    }
    return cloned_c_string;
  }

  void* Clone_CArray(const void* c_array_, size_t c_array_len_)
  {
    void* cloned_c_array = NULL;
    if (c_array_ != NULL)
    {
      cloned_c_array = std::malloc(c_array_len_);
      if (cloned_c_array != NULL)
        std::memcpy(cloned_c_array, c_array_, c_array_len_);
    }
    return cloned_c_array;
  }

  struct eCAL_STopicId* Clone_STopicId(const struct eCAL_STopicId* topic_id_)
  {
    eCAL_STopicId* cloned_topic_id = NULL;
    if (topic_id_ != NULL)
    {
      cloned_topic_id = reinterpret_cast<eCAL_STopicId*>(malloc(sizeof(eCAL_STopicId)));
      if (cloned_topic_id != NULL)
      {
        std::memcpy(cloned_topic_id, topic_id_, sizeof(eCAL_STopicId));
        cloned_topic_id->topic_name = Clone_CString(cloned_topic_id->topic_name);
        cloned_topic_id->topic_id.host_name = Clone_CString(cloned_topic_id->topic_id.host_name);
      }
    }

    return cloned_topic_id;
  }

  struct eCAL_SDataTypeInformation* Clone_SDataTypeInformation(const struct eCAL_SDataTypeInformation* data_type_information_)
  {
    eCAL_SDataTypeInformation* cloned_data_type_information = NULL;
    if (data_type_information_ != NULL)
    {
      cloned_data_type_information = reinterpret_cast<eCAL_SDataTypeInformation*>(malloc(sizeof(eCAL_SDataTypeInformation)));
      if (cloned_data_type_information != NULL)
      {
        std::memcpy(cloned_data_type_information, data_type_information_, sizeof(eCAL_SDataTypeInformation));
        cloned_data_type_information->name = Clone_CString(cloned_data_type_information->name);
        cloned_data_type_information->encoding = Clone_CString(cloned_data_type_information->encoding);
        cloned_data_type_information->descriptor = reinterpret_cast<char*>(Clone_CArray(cloned_data_type_information->descriptor, cloned_data_type_information->descriptor_len));
      }
    }
    return cloned_data_type_information;
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
      struct eCAL_STopicId topic_id;
      struct eCAL_SPubEventCallbackData pub_event_callback_data;
      Convert_STopicId(&topic_id, topic_id_);
      Convert_SPubEventCallbackData(&pub_event_callback_data, pub_event_callback_data_);
      pub_event_callback_(&topic_id, &pub_event_callback_data);
    };

    if (publisher_configuration_ != NULL)
      Convert_Publisher_Configuration(publisher_configuration, publisher_configuration_);

    return new eCAL_Publisher{ new eCAL::CPublisher(topic_name_, data_type_information, pub_event_callback, publisher_configuration) };
  }

  ECALC_API void eCAL_Publisher_Delete(eCAL_Publisher* publisher_)
  {
    delete publisher_->handle;
    delete publisher_;
    publisher_ = NULL;
  }

  ECALC_API int eCAL_Publisher_Send(eCAL_Publisher* publisher_, const void* buffer_, size_t buffer_len_, long long timestamp_)
  {
    return !publisher_->handle->Send(buffer_, buffer_len_, timestamp_);
  }

  ECALC_API int eCAL_Publisher_Send2(eCAL_Publisher* publisher_, struct eCAL_PayloadWriter* payload_writer_, long long timestamp_)
  {
    PayloadWriter payload_writer(payload_writer_);
    return !publisher_->handle->Send(payload_writer, timestamp_);
  }

  ECALC_API size_t eCAL_Publisher_GetSubscriberCount(eCAL_Publisher* publisher_)
  {
    return publisher_->handle->GetSubscriberCount();
  }

  ECALC_API char* eCAL_Publisher_GetTopicName(eCAL_Publisher* publisher_)
  {
    return Clone_CString(publisher_->handle->GetTopicName().c_str());
  }

  ECALC_API struct eCAL_STopicId* eCAL_Publisher_GetTopicId(eCAL_Publisher* publisher_)
  {
    eCAL_STopicId topic_id;
    Convert_STopicId(&topic_id, publisher_->handle->GetTopicId());
    return Clone_STopicId(&topic_id);
  }


  ECALC_API struct eCAL_SDataTypeInformation* eCAL_Publisher_GetDataTypeInformation(eCAL_Publisher* publisher_)
  {
    eCAL_SDataTypeInformation data_type_information;
    Convert_SDataTypeInformation(&data_type_information, publisher_->handle->GetDataTypeInformation());
    return Clone_SDataTypeInformation(&data_type_information);
  }

  ECALC_API void eCAL_STopicId_Free(struct eCAL_STopicId* topic_id_)
  {
    std::free(const_cast<void*>(reinterpret_cast<const void*>(topic_id_->topic_id.host_name)));
    std::free(const_cast<void*>(reinterpret_cast<const void*>(topic_id_->topic_name)));
    std::free(topic_id_);
    topic_id_ = NULL;
  }

  ECALC_API void eCAL_SDataTypeInformation_Free(struct eCAL_SDataTypeInformation* data_type_information_)
  {
    std::free(const_cast<void*>(reinterpret_cast<const void*>(data_type_information_->name)));
    std::free(const_cast<void*>(reinterpret_cast<const void*>(data_type_information_->encoding)));
    std::free(const_cast<void*>(reinterpret_cast<const void*>(data_type_information_->descriptor)));
    data_type_information_ = NULL;
  }

  ECALC_API struct eCAL_Publisher_Configuration eCAL_GetPublisherConfiguration()
  {
    eCAL_Publisher_Configuration publisher_configuration;
    Convert_Publisher_Configuration(&publisher_configuration, eCAL::GetPublisherConfiguration());
    return publisher_configuration;
  }

  ECALC_API void eCAL_Publisher_Configuration_Free(eCAL_Publisher_Configuration* publisher_configuration_)
  {
    std::free(reinterpret_cast<void*>(publisher_configuration_->layer_priority_local));
    std::free(reinterpret_cast<void*>(publisher_configuration_->layer_priority_remote));
  }
}
#endif // ECAL_CORE_PUBLISHER
