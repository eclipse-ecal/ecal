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

  bool CDescGate::GetPublisherInfo(const Registration::STopicId& id_, SDataTypeInformation& topic_info_) const
  {
    return GetTopic(id_, m_publisher_info_map, topic_info_);
  }

  Registration::CallbackToken CDescGate::AddPublisherEventCallback(const Registration::TopicIDCallbackT& callback_)
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

  std::set<Registration::STopicId> CDescGate::GetSubscriberIDs() const
  {
    return GetTopicIDs(m_subscriber_info_map);
  }

  bool CDescGate::GetSubscriberInfo(const Registration::STopicId& id_, SDataTypeInformation& topic_info_) const
  {
    return GetTopic(id_, m_subscriber_info_map, topic_info_);
  }

  Registration::CallbackToken CDescGate::AddSubscriberEventCallback(const Registration::TopicIDCallbackT& callback_)
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

  std::set<Registration::SServiceMethodId> CDescGate::GetServiceIDs() const
  {
    return GetServiceIDs(m_service_info_map);
  }

  bool CDescGate::GetServiceInfo(const Registration::SServiceMethodId& id_, SServiceMethodInformation& service_info_) const
  {
    return GetService(id_, m_service_info_map, service_info_);
  }

  std::set<Registration::SServiceMethodId> CDescGate::GetClientIDs() const
  {
    return GetServiceIDs(m_client_info_map);
  }

  bool CDescGate::GetClientInfo(const Registration::SServiceMethodId& id_, SServiceMethodInformation& service_info_) const
  {
    return GetService(id_, m_client_info_map, service_info_);
  }

  std::set<Registration::STopicId> CDescGate::GetTopicIDs(const STopicIdInfoMap& topic_info_map_)
  {
    std::set<Registration::STopicId> topic_id_set;

    const std::lock_guard<std::mutex> lock(topic_info_map_.mtx);
    for (const auto& topic_map_it : topic_info_map_.map)
    {
      topic_id_set.insert(topic_map_it.first);
    }
    return topic_id_set;
  }

  bool CDescGate::GetTopic(const Registration::STopicId& id_, const STopicIdInfoMap& topic_info_map_, SDataTypeInformation& topic_info_)
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

  std::set<Registration::SServiceMethodId> CDescGate::GetServiceIDs(const SServiceIdInfoMap& service_method_info_map_)
  {
    std::set<Registration::SServiceMethodId> service_id_set;

    const std::lock_guard<std::mutex> lock(service_method_info_map_.mtx);
    for (const auto& service_method_info_map_it : service_method_info_map_.id_map)
    {
      service_id_set.insert(service_method_info_map_it.first);
    }
    return service_id_set;
  }

  bool CDescGate::GetService(const Registration::SServiceMethodId& id_, const SServiceIdInfoMap& service_method_info_map_, SServiceMethodInformation& service_method_info_)
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
        const SDataTypeInformation request_datatype  = GetDataTypeInformation(method.req_datatype, method.req_type, method.req_desc);
        const SDataTypeInformation response_datatype = GetDataTypeInformation(method.resp_datatype, method.resp_type, method.resp_desc);

        ApplyServiceDescription(m_service_info_map, sample_.identifier, sample_.service.sname, method.mname, request_datatype, response_datatype);
      }
    }
    break;
    case bct_unreg_service:
      RemServiceDescription(m_service_info_map, sample_.identifier, sample_.service.sname);
      break;
    case bct_reg_client:
      for (const auto& method : sample_.client.methods)
      {
        const SDataTypeInformation request_datatype  = GetDataTypeInformation(method.req_datatype, method.req_type, method.req_desc);
        const SDataTypeInformation response_datatype = GetDataTypeInformation(method.resp_datatype, method.resp_type, method.resp_desc);

        ApplyServiceDescription(m_client_info_map, sample_.identifier, sample_.client.sname, method.mname, request_datatype, response_datatype);
     }
      break;
    case bct_unreg_client:
      RemServiceDescription(m_client_info_map, sample_.identifier, sample_.client.sname);
      break;
    case bct_reg_publisher:
      ApplyTopicDescription(m_publisher_info_map, m_publisher_callback_map, sample_.identifier, sample_.topic.tname, sample_.topic.tdatatype);
      break;
    case bct_unreg_publisher:
      RemTopicDescription(m_publisher_info_map, m_publisher_callback_map, sample_.identifier, sample_.topic.tname);
      break;
    case bct_reg_subscriber:
      ApplyTopicDescription(m_subscriber_info_map, m_subscriber_callback_map, sample_.identifier, sample_.topic.tname, sample_.topic.tdatatype);
      break;
    case bct_unreg_subscriber:
      RemTopicDescription(m_subscriber_info_map, m_subscriber_callback_map, sample_.identifier, sample_.topic.tname);
      break;
    default:
    {
      std::cerr << "CDescGate::ApplySample : unknown sample type" << '\n';
    }
    break;
    }
  }

  void CDescGate::ApplyTopicDescription(STopicIdInfoMap& topic_info_map_,
                                        const STopicIdCallbackMap& topic_callback_map_,
                                        const Registration::SampleIdentifier& topic_id_,
                                        const std::string& topic_name_,
                                        const SDataTypeInformation& topic_info_)
  {
    const auto topic_info_key = Registration::STopicId{ ConvertToEntityId(topic_id_), topic_name_ };

    // update topic info
    bool new_topic_info(false);
    {
      const std::unique_lock<std::mutex> lock(topic_info_map_.mtx);
      TopicIdInfoMap::iterator topic_info_quality_iter = topic_info_map_.map.find(topic_info_key);
      new_topic_info = topic_info_quality_iter == topic_info_map_.map.end();

      if (new_topic_info)
      {
        std::tie(topic_info_quality_iter, std::ignore) = topic_info_map_.map.emplace(topic_info_key, SDataTypeInformation{});
      }

      topic_info_quality_iter->second = topic_info_;
    }

    // notify publisher / subscriber registration callbacks about new entity
    if(new_topic_info)
    {
      const std::unique_lock<std::mutex> lock(topic_callback_map_.mtx);
      for (const auto& callback_iter : topic_callback_map_.map)
      {
        if (callback_iter.second)
        {
          callback_iter.second(topic_info_key, Registration::RegistrationEventType::new_entity);
        }
      }
    }
  }

  void CDescGate::RemTopicDescription(STopicIdInfoMap& topic_info_map_,
                                      const STopicIdCallbackMap& topic_callback_map_,
                                      const Registration::SampleIdentifier& topic_id_,
                                      const std::string& topic_name_)
  {
    const auto topic_info_key = Registration::STopicId{ ConvertToEntityId(topic_id_), topic_name_ };

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
          callback_iter.second(topic_info_key, Registration::RegistrationEventType::deleted_entity);
        }
      }
    }
  }

  void CDescGate::ApplyServiceDescription(SServiceIdInfoMap& service_method_info_map_,
                                          const Registration::SampleIdentifier& service_id_,
                                          const std::string& service_name_,
                                          const std::string& method_name_,
                                          const SDataTypeInformation& request_type_information_,
                                          const SDataTypeInformation& response_type_information_)
  {
    const auto service_method_info_key = Registration::SServiceMethodId{ ConvertToEntityId(service_id_), service_name_, method_name_};

    SServiceMethodInformation service_info;
    service_info.request_type  = request_type_information_;
    service_info.response_type = response_type_information_;

    const std::lock_guard<std::mutex> lock(service_method_info_map_.mtx);
    service_method_info_map_.id_map[service_method_info_key] = service_info;
  }

  void CDescGate::RemServiceDescription(SServiceIdInfoMap& service_method_info_map_,
                                        const Registration::SampleIdentifier& service_id_,
                                        const std::string& service_name_)
  {
    std::list<Registration::SServiceMethodId> service_method_info_keys_to_remove;

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

  SDataTypeInformation CDescGate::GetDataTypeInformation(const SDataTypeInformation& datatype_info_, const std::string& legacy_type_, const std::string& legacy_desc_)
  {
    SDataTypeInformation datatype_info;

    // if the new datatype_info field is set, use it
    if (!datatype_info_.name.empty())
    {
      datatype_info = datatype_info_;
    }
    // otherwise use the old type and desc fields
    else
    {
      datatype_info.name       = legacy_type_;
      datatype_info.descriptor = legacy_desc_;
    }

    return datatype_info;
  }

  Registration::CallbackToken CDescGate::CreateToken()
  {
    // Atomically increment m_callback_token using fetch_add to ensure thread safety.
    // fetch_add returns the value before increment, so we add 1 to get the new token value.
    // memory_order_relaxed is used to optimize performance without additional synchronization.
    return m_callback_token.fetch_add(1, std::memory_order_relaxed) + 1;
  }
}
