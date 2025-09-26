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
  eCAL::SEntityId ConvertToEntityId(const eCAL::Registration::SampleIdentifier& sample_identifier)
  {
    eCAL::SEntityId id{ sample_identifier.entity_id, sample_identifier.process_id, sample_identifier.host_name };
    return id;
  }

}

namespace eCAL
{
  CDescGate::CDescGate() = default;
  CDescGate::~CDescGate() = default;

  std::set<STopicId> CDescGate::GetPublisherIDs() const
  {
    return m_publisher_infos.GetIDs();
  }

  bool CDescGate::GetPublisherInfo(const STopicId& id_, SDataTypeInformation& topic_info_) const
  {
    return m_publisher_infos.GetInfo(id_, topic_info_);
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
    return m_subscriber_infos.GetIDs();
  }

  bool CDescGate::GetSubscriberInfo(const STopicId& id_, SDataTypeInformation& topic_info_) const
  {
    return m_subscriber_infos.GetInfo(id_, topic_info_);
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
    return {};
  }

  bool CDescGate::GetServerInfo(const SServiceId& id_, ServiceMethodInformationSetT& service_info_) const
  {
    return {};
  }

  std::set<SServiceId> CDescGate::GetClientIDs() const
  {
    return {};
  }

  bool CDescGate::GetClientInfo(const SServiceId& id_, ServiceMethodInformationSetT& service_info_) const
  {
    return {};
  }

  template <class CallbackMap>
  auto create_notify_lambda(CallbackMap& callbacks, eCAL::Registration::RegistrationEventType ev) {
    return [&callbacks, ev](const auto& tid) {
      std::lock_guard<std::mutex> guard(callbacks.mtx);
      for (const auto& kv : callbacks.map)
        if (kv.second) kv.second(tid, ev);
      };
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
      break;
    case bct_unreg_service:
      break;
    case bct_reg_client:
      break;
    case bct_unreg_client:
      break;
    case bct_reg_publisher:
      m_publisher_infos.RegisterSample(sample_, create_notify_lambda(m_publisher_callback_map, eCAL::Registration::RegistrationEventType::new_entity));
      break;
    case bct_unreg_publisher:
      m_publisher_infos.UnregisterSample(sample_, create_notify_lambda(m_publisher_callback_map, eCAL::Registration::RegistrationEventType::deleted_entity));
      break;
    case bct_reg_subscriber:
      m_subscriber_infos.RegisterSample(sample_, create_notify_lambda(m_subscriber_callback_map, eCAL::Registration::RegistrationEventType::new_entity));
      break;
    case bct_unreg_subscriber:
      m_subscriber_infos.UnregisterSample(sample_, create_notify_lambda(m_subscriber_callback_map, eCAL::Registration::RegistrationEventType::deleted_entity));
      break;
    default:
    {
      std::cerr << "CDescGateOld::ApplySample : unknown sample type" << '\n';
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

  inline void CDescGate::CollectedTopicInfo::RegisterSample(const Registration::Sample& sample_, std::function<void(const STopicId&)> on_new_topic)
  {
    std::unique_lock<std::mutex> guard(mutex);

    auto iterator = map.find(sample_.identifier.entity_id);

    if (iterator != map.end())
    {
      // See if we need to update dataype information.
      // In theory, this should be an invariant, at least with v6.
      // Anyways, only update if changed
//      if (iterator->second.datatype_info != sample_.topic.datatype_information)
      {
        iterator->second.datatype_info = sample_.topic.datatype_information;
      }
    }
    else
    {
      auto topic_id = eCAL::STopicId{ ConvertToEntityId(sample_.identifier), sample_.topic.topic_name};
      SDataTypeInformation datatype_information = sample_.topic.datatype_information;
      auto topic_info = TopicInfo{ topic_id, std::move(datatype_information) };
      
      map.emplace(sample_.identifier.entity_id, topic_info );

      guard.unlock();

      on_new_topic(topic_id);
    }
  }

  inline void CDescGate::CollectedTopicInfo::UnregisterSample(const Registration::Sample& sample_, std::function<void(const STopicId&)> on_erased_topic)
  {
    std::unique_lock<std::mutex> guard(mutex); 

    auto iterator = map.find(sample_.identifier.entity_id);
    if (iterator != map.end())
    {
      // First erase and then call the callback.
      // Should think about if it makes sense to already release the mutex
      STopicId erased_id = iterator->second.id;
      map.erase(iterator);
      
      guard.unlock();
      on_erased_topic(erased_id);
    }
  }

  inline std::set<STopicId> CDescGate::CollectedTopicInfo::GetIDs() const
  {
    std::lock_guard<std::mutex> guard(mutex);

    std::set<STopicId> topic_ids;
    for (const auto& map_iterator : map)
    {
      topic_ids.insert(map_iterator.second.id);
    }
    return topic_ids;
  }

  inline bool CDescGate::CollectedTopicInfo::GetInfo(const STopicId& id_, SDataTypeInformation& topic_info_) const
  {
    std::lock_guard<std::mutex> guard(mutex);

    auto iterator = map.find(id_.topic_id.entity_id);

    if (iterator == map.end())
      return false;

    topic_info_ = iterator->second.datatype_info;
    return true;
  }
}


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
      methods.insert(eCAL::SServiceMethodInformation{ method.method_name, request_datatype, response_datatype });
    }
    return methods;
  }

  void ApplyTopicDescription(eCAL::CDescGateOld::STopicIdInfoMap& topic_info_map_,
    const eCAL::CDescGateOld::STopicEventCallbackMap& topic_callback_map_,
    const eCAL::Registration::SampleIdentifier& topic_id_,
    const std::string& topic_name_,
    const eCAL::SDataTypeInformation& topic_info_)
  {
    const auto topic_info_key = eCAL::STopicId{ ConvertToEntityId(topic_id_), topic_name_ };

    // update topic info
    bool new_topic_info(false);
    {
      const std::unique_lock<std::mutex> lock(topic_info_map_.mtx);
      eCAL::CDescGateOld::TopicIdInfoMap::iterator topic_info_quality_iter = topic_info_map_.map.find(topic_info_key);
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

  void RemTopicDescription(eCAL::CDescGateOld::STopicIdInfoMap& topic_info_map_,
    const eCAL::CDescGateOld::STopicEventCallbackMap& topic_callback_map_,
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
  void ApplyServiceDescription(eCAL::CDescGateOld::SServiceIdInfoMap& service_method_info_map_,
    const eCAL::Registration::SampleIdentifier& service_id_,
    const Service& service_)
  {
    const auto service_method_info_key = eCAL::SServiceId{ ConvertToEntityId(service_id_), service_.service_name };

    const std::lock_guard<std::mutex> lock(service_method_info_map_.mtx);
    service_method_info_map_.id_map[service_method_info_key] = Convert(service_);
  }

  template<typename Service>
  void RemServiceDescription(eCAL::CDescGateOld::SServiceIdInfoMap& service_method_info_map_,
    const eCAL::Registration::SampleIdentifier& service_id_,
    const Service& service_)
  {
    const auto service_method_info_key = eCAL::SServiceId{ ConvertToEntityId(service_id_),  service_.service_name };

    const std::lock_guard<std::mutex> lock(service_method_info_map_.mtx);
    service_method_info_map_.id_map.erase(service_method_info_key);
  }
}


namespace eCAL{
  CDescGateOld::CDescGateOld() = default;
  CDescGateOld::~CDescGateOld() = default;

  std::set<STopicId> CDescGateOld::GetPublisherIDs() const
  {
    return GetTopicIDs(m_publisher_info_map);
  }

  bool CDescGateOld::GetPublisherInfo(const STopicId& id_, SDataTypeInformation& topic_info_) const
  {
    return GetTopic(id_, m_publisher_info_map, topic_info_);
  }

  Registration::CallbackToken CDescGateOld::AddPublisherEventCallback(const Registration::TopicEventCallbackT& callback_)
  {
    const std::lock_guard<std::mutex> lock(m_publisher_callback_map.mtx);

    const Registration::CallbackToken new_token = CreateToken();
    m_publisher_callback_map.map[new_token] = callback_;

    return new_token;
  }

  void CDescGateOld::RemPublisherEventCallback(Registration::CallbackToken token_)
  {
    const std::lock_guard<std::mutex> lock(m_publisher_callback_map.mtx);
    m_publisher_callback_map.map.erase(token_);
  }

  std::set<STopicId> CDescGateOld::GetSubscriberIDs() const
  {
    return GetTopicIDs(m_subscriber_info_map);
  }

  bool CDescGateOld::GetSubscriberInfo(const STopicId& id_, SDataTypeInformation& topic_info_) const
  {
    return GetTopic(id_, m_subscriber_info_map, topic_info_);
  }

  Registration::CallbackToken CDescGateOld::AddSubscriberEventCallback(const Registration::TopicEventCallbackT& callback_)
  {
    const std::lock_guard<std::mutex> lock(m_subscriber_callback_map.mtx);

    const Registration::CallbackToken new_token = CreateToken();
    m_subscriber_callback_map.map[new_token] = callback_;

    return new_token;
  }

  void CDescGateOld::RemSubscriberEventCallback(Registration::CallbackToken token_)
  {
    const std::lock_guard<std::mutex> lock(m_subscriber_callback_map.mtx);
    m_subscriber_callback_map.map.erase(token_);
  }

  std::set<SServiceId> CDescGateOld::GetServerIDs() const
  {
    return GetServiceIDs(m_service_info_map);
  }

  bool CDescGateOld::GetServerInfo(const SServiceId& id_, ServiceMethodInformationSetT& service_info_) const
  {
    return GetService(id_, m_service_info_map, service_info_);
  }

  std::set<SServiceId> CDescGateOld::GetClientIDs() const
  {
    return GetServiceIDs(m_client_info_map);
  }

  bool CDescGateOld::GetClientInfo(const SServiceId& id_, ServiceMethodInformationSetT& service_info_) const
  {
    return GetService(id_, m_client_info_map, service_info_);
  }

  std::set<STopicId> CDescGateOld::GetTopicIDs(const STopicIdInfoMap& topic_info_map_)
  {
    std::set<STopicId> topic_id_set;

    const std::lock_guard<std::mutex> lock(topic_info_map_.mtx);
    for (const auto& topic_map_it : topic_info_map_.map)
    {
      topic_id_set.insert(topic_map_it.first);
    }
    return topic_id_set;
  }

  bool CDescGateOld::GetTopic(const STopicId& id_, const STopicIdInfoMap& topic_info_map_, SDataTypeInformation& topic_info_)
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

  std::set<SServiceId> CDescGateOld::GetServiceIDs(const SServiceIdInfoMap& service_method_info_map_)
  {
    std::set<SServiceId> service_id_set;

    const std::lock_guard<std::mutex> lock(service_method_info_map_.mtx);
    for (const auto& service_method_info_map_it : service_method_info_map_.id_map)
    {
      service_id_set.insert(service_method_info_map_it.first);
    }
    return service_id_set;
  }

  bool CDescGateOld::GetService(const SServiceId& id_, const SServiceIdInfoMap& service_method_info_map_, ServiceMethodInformationSetT& service_method_info_)
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

  void CDescGateOld::ApplySample(const Registration::Sample& sample_, eTLayerType /*layer_*/)
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
      std::cerr << "CDescGateOld::ApplySample : unknown sample type" << '\n';
    }
    break;
    }
  }

  Registration::CallbackToken CDescGateOld::CreateToken()
  {
    // Atomically increment m_callback_token using fetch_add to ensure thread safety.
    // fetch_add returns the value before increment, so we add 1 to get the new token value.
    // memory_order_relaxed is used to optimize performance without additional synchronization.
    return m_callback_token.fetch_add(1, std::memory_order_relaxed) + 1;
  }




}
