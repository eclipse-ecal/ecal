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
 * @brief  eCAL description gateway class
**/

#include "ecal_descgate.h"

#include <iostream>

namespace
{
  // TODO: this is not very performant
  // a) this should be done when the information is first retrieved
  // b) we always copy the info around, which is problematic for performance
  eCAL::SDataTypeInformation GetDataTypeInformation(const eCAL::SDataTypeInformation& datatype_info_, const std::string& legacy_type_, const std::string& legacy_desc_)
  {
    eCAL::SDataTypeInformation datatype_info;

    // if the new datatype_info field is set, use it
    if (!datatype_info_.name.empty())
    {
      datatype_info = datatype_info_;
    }
    // otherwise use the old type and desc fields
    else
    {
      datatype_info.name = legacy_type_;
      datatype_info.descriptor = legacy_desc_;
    }

    return datatype_info;
  }

  eCAL::SEntityId ConvertToEntityId(const eCAL::Registration::SampleIdentifier& sample_identifier)
  {
    eCAL::SEntityId id{ sample_identifier.entity_id, sample_identifier.process_id, sample_identifier.host_name};
    return id;
  }

  // we could also think about an update function here
  // e.g. we don't always copy everything, but only write if things have changed. At least this would be beneficial for performance.
  template<typename Service>
  eCAL::ServiceMethodInformationSetT Convert(const Service& service_)
  {
    eCAL::ServiceMethodInformationSetT methods;
    for (const auto& method : service_.methods)
    {
      const eCAL::SDataTypeInformation request_datatype = GetDataTypeInformation(method.request_datatype_information, method.req_type, method.req_desc);
      const eCAL::SDataTypeInformation response_datatype = GetDataTypeInformation(method.response_datatype_information, method.resp_type, method.resp_desc);
      methods.insert(eCAL::SServiceMethodInformation{method.method_name, request_datatype, response_datatype });
    }
    return methods;
  }

  void ApplyTopicDescription(eCAL::CDescGate::STopicIdInfoMap& topic_info_map_,
    const eCAL::CDescGate::STopicEventCallbackMap& topic_callback_map_,
    const eCAL::Registration::SampleIdentifier& topic_id_,
    const std::string& topic_name_,
    const eCAL::SDataTypeInformation& topic_info_)
  {
    const auto topic_info_key = eCAL::STopicId{ ConvertToEntityId(topic_id_), topic_name_ };

    // update topic info
    bool new_topic_info(false);
    {
      const std::unique_lock<std::mutex> lock(topic_info_map_.mtx);
      eCAL::CDescGate::TopicIdInfoMap::iterator topic_info_quality_iter = topic_info_map_.map.find(topic_info_key);
      new_topic_info = topic_info_quality_iter == topic_info_map_.map.end();

      if (new_topic_info)
      {
        std::tie(topic_info_quality_iter, std::ignore) = topic_info_map_.map.emplace(topic_info_key, eCAL::SDataTypeInformation{});
      }

      topic_info_quality_iter->second = topic_info_;
    }

    // notify publisher / subscriber registration callbacks about new entity
    if (new_topic_info)
    {
      const std::unique_lock<std::mutex> lock(topic_callback_map_.mtx);
      for (const auto& callback_iter : topic_callback_map_.map)
      {
        if (callback_iter.second)
        {
          callback_iter.second(topic_info_key, eCAL::Registration::RegistrationEventType::new_entity);
        }
      }
    }
  }

  void RemTopicDescription(eCAL::CDescGate::STopicIdInfoMap& topic_info_map_,
    const eCAL::CDescGate::STopicEventCallbackMap& topic_callback_map_,
    const eCAL::Registration::SampleIdentifier& topic_id_,
    const std::string& topic_name_)
  {
    const auto topic_info_key = eCAL::STopicId{ ConvertToEntityId(topic_id_), topic_name_ };

    // delete topic info
    bool deleted_topic_info(false);
    {
      const std::unique_lock<std::mutex> lock(topic_info_map_.mtx);
      deleted_topic_info = topic_info_map_.map.erase(topic_info_key) > 0;
    }

    // notify publisher / subscriber registration callbacks about deleted entity
    if (deleted_topic_info)
    {
      const std::unique_lock<std::mutex> lock(topic_callback_map_.mtx);
      for (const auto& callback_iter : topic_callback_map_.map)
      {
        if (callback_iter.second)
        {
          callback_iter.second(topic_info_key, eCAL::Registration::RegistrationEventType::deleted_entity);
        }
      }
    }
  }

  template<typename Service>
  void ApplyServiceDescription(eCAL::CDescGate::SServiceIdInfoMap& service_method_info_map_,
    const eCAL::Registration::SampleIdentifier& service_id_,
    const Service& service_)
  {
    const auto service_method_info_key = eCAL::SServiceId{ ConvertToEntityId(service_id_), service_.service_name };

    const std::lock_guard<std::mutex> lock(service_method_info_map_.mtx);
    service_method_info_map_.id_map[service_method_info_key] = Convert(service_);
  }

  template<typename Service>
  void RemServiceDescription(eCAL::CDescGate::SServiceIdInfoMap& service_method_info_map_,
    const eCAL::Registration::SampleIdentifier& service_id_,
    const Service& service_)
  {
    const auto service_method_info_key = eCAL::SServiceId{ ConvertToEntityId(service_id_),  service_.service_name };

    const std::lock_guard<std::mutex> lock(service_method_info_map_.mtx);
    service_method_info_map_.id_map.erase(service_method_info_key);
  }
}

namespace eCAL
{
  CDescGate::CDescGate() = default;
  CDescGate::~CDescGate() = default;

  std::set<STopicId> CDescGate::GetPublisherIDs() const
  {
    return GetTopicIDs(m_publisher_info_map);
  }

  bool CDescGate::GetPublisherInfo(const STopicId& id_, SDataTypeInformation& topic_info_) const
  {
    return GetTopic(id_, m_publisher_info_map, topic_info_);
  }

  Registration::CallbackToken CDescGate::AddPublisherEventCallback(const Registration::TopicEventCallbackT& callback_)
  {
    const std::lock_guard<std::mutex> lock(m_publisher_callback_map.mtx);

    const Registration::CallbackToken new_token = CreateToken();
    m_publisher_callback_map.map[new_token] = callback_;

    return new_token;
  }

  void CDescGate::RemPublisherEventCallback(Registration::CallbackToken token_)
  {
    const std::lock_guard<std::mutex> lock(m_publisher_callback_map.mtx);
    m_publisher_callback_map.map.erase(token_);
  }

  std::set<STopicId> CDescGate::GetSubscriberIDs() const
  {
    return GetTopicIDs(m_subscriber_info_map);
  }

  bool CDescGate::GetSubscriberInfo(const STopicId& id_, SDataTypeInformation& topic_info_) const
  {
    return GetTopic(id_, m_subscriber_info_map, topic_info_);
  }

  Registration::CallbackToken CDescGate::AddSubscriberEventCallback(const Registration::TopicEventCallbackT& callback_)
  {
    const std::lock_guard<std::mutex> lock(m_subscriber_callback_map.mtx);

    const Registration::CallbackToken new_token = CreateToken();
    m_subscriber_callback_map.map[new_token] = callback_;

    return new_token;
  }

  void CDescGate::RemSubscriberEventCallback(Registration::CallbackToken token_)
  {
    const std::lock_guard<std::mutex> lock(m_subscriber_callback_map.mtx);
    m_subscriber_callback_map.map.erase(token_);
  }

  std::set<SServiceId> CDescGate::GetServerIDs() const
  {
    return GetServiceIDs(m_service_info_map);
  }

  bool CDescGate::GetServerInfo(const SServiceId& id_, ServiceMethodInformationSetT& service_info_) const
  {
    return GetService(id_, m_service_info_map, service_info_);
  }

  std::set<SServiceId> CDescGate::GetClientIDs() const
  {
    return GetServiceIDs(m_client_info_map);
  }

  bool CDescGate::GetClientInfo(const SServiceId& id_, ServiceMethodInformationSetT& service_info_) const
  {
    return GetService(id_, m_client_info_map, service_info_);
  }

  std::set<STopicId> CDescGate::GetTopicIDs(const STopicIdInfoMap& topic_info_map_)
  {
    std::set<STopicId> topic_id_set;

    const std::lock_guard<std::mutex> lock(topic_info_map_.mtx);
    for (const auto& topic_map_it : topic_info_map_.map)
    {
      topic_id_set.insert(topic_map_it.first);
    }
    return topic_id_set;
  }

  bool CDescGate::GetTopic(const STopicId& id_, const STopicIdInfoMap& topic_info_map_, SDataTypeInformation& topic_info_)
  {
    const std::lock_guard<std::mutex> lock(topic_info_map_.mtx);
    auto iter = topic_info_map_.map.find(id_);
    if (iter == topic_info_map_.map.end())
    {
      return false;
    }
    else
    {
      topic_info_ = iter->second;
      return true;
    }
  }

  std::set<SServiceId> CDescGate::GetServiceIDs(const SServiceIdInfoMap& service_method_info_map_)
  {
    std::set<SServiceId> service_id_set;

    const std::lock_guard<std::mutex> lock(service_method_info_map_.mtx);
    for (const auto& service_method_info_map_it : service_method_info_map_.id_map)
    {
      service_id_set.insert(service_method_info_map_it.first);
    }
    return service_id_set;
  }

  bool CDescGate::GetService(const SServiceId& id_, const SServiceIdInfoMap& service_method_info_map_, ServiceMethodInformationSetT& service_method_info_)
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
      ApplyServiceDescription(m_service_info_map, sample_.identifier, sample_.service);
      break;
    case bct_unreg_service:
      RemServiceDescription(m_service_info_map, sample_.identifier, sample_.service);
      break;
    case bct_reg_client:
      ApplyServiceDescription(m_client_info_map, sample_.identifier, sample_.client);
      break;
    case bct_unreg_client:
      RemServiceDescription(m_client_info_map, sample_.identifier, sample_.client);
      break;
    case bct_reg_publisher:
      ApplyTopicDescription(m_publisher_info_map, m_publisher_callback_map, sample_.identifier, sample_.topic.topic_name, sample_.topic.datatype_information);
      break;
    case bct_unreg_publisher:
      RemTopicDescription(m_publisher_info_map, m_publisher_callback_map, sample_.identifier, sample_.topic.topic_name);
      break;
    case bct_reg_subscriber:
      ApplyTopicDescription(m_subscriber_info_map, m_subscriber_callback_map, sample_.identifier, sample_.topic.topic_name, sample_.topic.datatype_information);
      break;
    case bct_unreg_subscriber:
      RemTopicDescription(m_subscriber_info_map, m_subscriber_callback_map, sample_.identifier, sample_.topic.topic_name);
      break;
    default:
    {
      std::cerr << "CDescGate::ApplySample : unknown sample type" << '\n';
    }
    break;
    }
  }

  Registration::CallbackToken CDescGate::CreateToken()
  {
    // Atomically increment m_callback_token using fetch_add to ensure thread safety.
    // fetch_add returns the value before increment, so we add 1 to get the new token value.
    // memory_order_relaxed is used to optimize performance without additional synchronization.
    return m_callback_token.fetch_add(1, std::memory_order_relaxed) + 1;
  }
}
