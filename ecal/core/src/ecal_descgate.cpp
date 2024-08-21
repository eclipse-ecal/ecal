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
 * @brief  eCAL description gateway class
**/

#include "ecal_descgate.h"

#include <iostream>

namespace
{
  eCAL::Registration::DescQualityFlags GetDataTypeInfoQuality(const eCAL::SDataTypeInformation& data_type_info_, bool is_producer_)
  {
    eCAL::Registration::DescQualityFlags quality = eCAL::Registration::DescQualityFlags::NO_QUALITY;
    if (!data_type_info_.name.empty())
      quality |= eCAL::Registration::DescQualityFlags::TYPENAME_AVAILABLE;
    if (!data_type_info_.encoding.empty())
      quality |= eCAL::Registration::DescQualityFlags::ENCODING_AVAILABLE;
    if (!data_type_info_.descriptor.empty())
      quality |= eCAL::Registration::DescQualityFlags::DESCRIPTION_AVAILABLE;
    if(is_producer_) quality |= eCAL::Registration::DescQualityFlags::INFO_COMES_FROM_PRODUCER;
    return quality;
  }

  eCAL::Registration::SEntityId ConvertToEntityId(const eCAL::Registration::SampleIdentifier& sample_identifier)
  {
    eCAL::Registration::SEntityId id{ sample_identifier.entity_id, sample_identifier.process_id, sample_identifier.host_name};
    return id;
  }


}

namespace eCAL
{
  CDescGate::CDescGate() = default;
  CDescGate::~CDescGate() = default;

  std::set<Registration::STopicId> CDescGate::GetPublisherIDs() const
  {
    return GetTopicIDs(m_publisher_info_map);
  }

  bool CDescGate::GetPublisherInfo(const Registration::STopicId& id_, Registration::SQualityTopicInfo& topic_info_) const
  {
    return GetTopic(id_, m_publisher_info_map, topic_info_);
  }

  Registration::CallbackToken CDescGate::RegisterPublisherEventCallback(const Registration::TopicIDCallbackT& callback_)
  {
    const std::lock_guard<std::mutex> lock(m_publisher_info_map.mtx);

    const Registration::CallbackToken new_token = CreateToken();
    m_publisher_info_map.cb_map[new_token] = callback_;

    return new_token;
  }

  void CDescGate::UnregisterPublisherEventCallback(Registration::CallbackToken token_)
  {
    const std::lock_guard<std::mutex> lock(m_publisher_info_map.mtx);
    m_publisher_info_map.cb_map.erase(token_);
  }

  std::set<Registration::STopicId> CDescGate::GetSubscriberIDs() const
  {
    return GetTopicIDs(m_subscriber_info_map);
  }

  bool CDescGate::GetSubscriberInfo(const Registration::STopicId& id_, Registration::SQualityTopicInfo& topic_info_) const
  {
    return GetTopic(id_, m_subscriber_info_map, topic_info_);
  }

  std::set<Registration::SServiceId> CDescGate::GetServiceIDs() const
  {
    return GetServiceIDs(m_service_info_map);
  }

  bool CDescGate::GetServiceInfo(const Registration::SServiceId& id_, Registration::SQualityServiceInfo& service_info_) const
  {
    return GetService(id_, m_service_info_map, service_info_);
  }

  std::set<Registration::SServiceId> CDescGate::GetClientIDs() const
  {
    return GetServiceIDs(m_client_info_map);
  }

  bool CDescGate::GetClientInfo(const Registration::SServiceId& id_, Registration::SQualityServiceInfo& service_info_) const
  {
    return GetService(id_, m_client_info_map, service_info_);
  }

  std::set<Registration::STopicId> CDescGate::GetTopicIDs(const SQualityTopicIdMap& topic_info_map_)
  {
    std::set<Registration::STopicId> topic_id_set;

    const std::lock_guard<std::mutex> lock(topic_info_map_.mtx);
    for (const auto& topic_map_it : topic_info_map_.id_map)
    {
      topic_id_set.insert(topic_map_it.first);
    }
    return topic_id_set;
  }

  bool CDescGate::GetTopic(const Registration::STopicId& id_, const SQualityTopicIdMap& topic_info_map_, Registration::SQualityTopicInfo& topic_info_)
  {
    const std::lock_guard<std::mutex> lock(topic_info_map_.mtx);
    auto iter = topic_info_map_.id_map.find(id_);
    if (iter == topic_info_map_.id_map.end())
    {
      return false;
    }
    else
    {
      topic_info_ = iter->second;
      return true;
    }
  }

  std::set<Registration::SServiceId> CDescGate::GetServiceIDs(const SQualityServiceIdMap& service_method_info_map_)
  {
    std::set<Registration::SServiceId> service_id_set;

    const std::lock_guard<std::mutex> lock(service_method_info_map_.mtx);
    for (const auto& service_method_info_map_it : service_method_info_map_.id_map)
    {
      service_id_set.insert(service_method_info_map_it.first);
    }
    return service_id_set;
  }

  bool CDescGate::GetService(const Registration::SServiceId& id_, const SQualityServiceIdMap& service_method_info_map_, Registration::SQualityServiceInfo& service_method_info_)
  {
    const std::lock_guard<std::mutex> lock(service_method_info_map_.mtx);
    auto iter = service_method_info_map_.id_map.find(id_);
    if (iter == service_method_info_map_.id_map.end())
    {
      return false;
    }
    else
    {
      service_method_info_ = iter->second;
      return true;
    }
  }

  void CDescGate::ApplySample(const Registration::Sample& sample_, eTLayerType /*layer_*/)
  {
    switch (sample_.cmd_type)
    {
    case bct_none:
    case bct_set_sample:
    case bct_reg_process:
    case bct_unreg_process:
      break;
    case bct_reg_service:
    {
      for (const auto& method : sample_.service.methods)
      {
        SDataTypeInformation request_type{};
        request_type.name       = method.req_type;
        request_type.descriptor = method.req_desc;

        SDataTypeInformation response_type{};
        response_type.name       = method.resp_type;
        response_type.descriptor = method.resp_desc;

        ApplyServiceDescription(m_service_info_map, sample_.identifier, sample_.service.sname, method.mname, request_type, response_type, GetDataTypeInfoQuality(request_type, true), GetDataTypeInfoQuality(response_type, true));
      }
    }
    break;
    case bct_unreg_service:
      RemServiceDescription(m_service_info_map, sample_.identifier, sample_.service.sname);
      break;
    case bct_reg_client:
      for (const auto& method : sample_.client.methods)
      {
        SDataTypeInformation request_type;
        request_type.name       = method.req_type;
        request_type.descriptor = method.req_desc;

        SDataTypeInformation response_type{};
        response_type.name       = method.resp_type;
        response_type.descriptor = method.resp_desc;

        ApplyServiceDescription(m_client_info_map, sample_.identifier, sample_.client.sname, method.mname, request_type, response_type, GetDataTypeInfoQuality(request_type, false), GetDataTypeInfoQuality(response_type, false));
      }
      break;
    case bct_unreg_client:
      RemServiceDescription(m_client_info_map, sample_.identifier, sample_.client.sname);
      break;
    case bct_reg_publisher:
      ApplyTopicDescription(m_publisher_info_map, sample_.identifier, sample_.topic.tname, sample_.topic.tdatatype, GetDataTypeInfoQuality(sample_.topic.tdatatype, true));
      break;
    case bct_unreg_publisher:
      RemTopicDescription(m_publisher_info_map, sample_.identifier, sample_.topic.tname);
      break;
    case bct_reg_subscriber:
      ApplyTopicDescription(m_subscriber_info_map, sample_.identifier, sample_.topic.tname, sample_.topic.tdatatype, GetDataTypeInfoQuality(sample_.topic.tdatatype, false));
      break;
    case bct_unreg_subscriber:
      RemTopicDescription(m_subscriber_info_map, sample_.identifier, sample_.topic.tname);
      break;
    default:
    {
      std::cerr << "CDescGate::ApplySample : unknown sample type" << '\n';
    }
    break;
    }
  }

  void CDescGate::ApplyTopicDescription(SQualityTopicIdMap& topic_info_map_,
                                        const Registration::SampleIdentifier& topic_id_,
                                        const std::string& topic_name_,
                                        const SDataTypeInformation& topic_info_,
                                        const Registration::DescQualityFlags topic_quality_)
  {
    const auto topic_info_key = Registration::STopicId{ ConvertToEntityId(topic_id_), topic_name_ };

    Registration::SQualityTopicInfo topic_quality_info;
    topic_quality_info.info    = topic_info_;
    topic_quality_info.quality = topic_quality_;

    const std::unique_lock<std::mutex> lock(topic_info_map_.mtx);
    const auto iter = topic_info_map_.id_map.find(topic_info_key);
    topic_info_map_.id_map[topic_info_key] = topic_quality_info;

    if (iter == topic_info_map_.id_map.end())
    {
      // notify publisher / subscriber registration callbacks about new entity
      for (const auto& callback_iter : topic_info_map_.cb_map)
      {
        if (callback_iter.second)
        {
          callback_iter.second(topic_info_key, Registration::RegistrationEventType::new_entity);
        }
      }
    }
  }

  void CDescGate::RemTopicDescription(SQualityTopicIdMap& topic_info_map_,
                                      const Registration::SampleIdentifier& topic_id_,
                                      const std::string& topic_name_)
  {
    const auto topic_info_key = Registration::STopicId{ ConvertToEntityId(topic_id_), topic_name_ };

    const std::unique_lock<std::mutex> lock(topic_info_map_.mtx);
    if (topic_info_map_.id_map.erase(topic_info_key) > 0)
    {
      // notify publisher / subscriber registration callbacks about deleted entity
      for (const auto& callback_iter : topic_info_map_.cb_map)
      {
        if (callback_iter.second)
        {
          callback_iter.second(topic_info_key, Registration::RegistrationEventType::deleted_entity);
        }
      }
    }
  }

  void CDescGate::ApplyServiceDescription(SQualityServiceIdMap& service_method_info_map_,
                                          const Registration::SampleIdentifier& service_id_,
                                          const std::string& service_name_,
                                          const std::string& method_name_,
                                          const SDataTypeInformation& request_type_information_,
                                          const SDataTypeInformation& response_type_information_,
                                          const Registration::DescQualityFlags request_type_quality_,
                                          const Registration::DescQualityFlags response_type_quality_)
  {
    const auto service_method_info_key = Registration::SServiceId{ ConvertToEntityId(service_id_), service_name_, method_name_};

    Registration::SQualityServiceInfo service_quality_info;
    service_quality_info.info.request_type  = request_type_information_;
    service_quality_info.info.response_type = response_type_information_;
    service_quality_info.request_quality    = request_type_quality_;
    service_quality_info.response_quality   = response_type_quality_;

    const std::lock_guard<std::mutex> lock(service_method_info_map_.mtx);
    service_method_info_map_.id_map[service_method_info_key] = service_quality_info;
  }

  void CDescGate::RemServiceDescription(SQualityServiceIdMap& service_method_info_map_,
                                        const Registration::SampleIdentifier& service_id_,
                                        const std::string& service_name_)
  {
    std::list<Registration::SServiceId> service_method_info_keys_to_remove;

    const std::lock_guard<std::mutex> lock(service_method_info_map_.mtx);

    for (auto&& service_it : service_method_info_map_.id_map)
    {
      const auto service_method_info_key = service_it.first;
      if ((service_method_info_key.service_name == service_name_)
        && (service_method_info_key.service_id == ConvertToEntityId(service_id_)))
      {
        service_method_info_keys_to_remove.push_back(service_method_info_key);
      }
    }

    for (const auto& service_method_info_key : service_method_info_keys_to_remove)
    {
      service_method_info_map_.id_map.erase(service_method_info_key);
    }
  }

  Registration::CallbackToken CDescGate::CreateToken()
  {
    const std::lock_guard<std::mutex> lock(m_callback_token_mtx);
    const Registration::CallbackToken token = ++m_callback_token;
    return token;
  }
}
