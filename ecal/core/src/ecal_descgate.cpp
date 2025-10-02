#include "ecal_descgate.h"

#include <iostream>

namespace
{
  // Keep all helpers with internal linkage in this TU.

  eCAL::SEntityId ConvertToEntityId(const eCAL::Registration::SampleIdentifier& sample_identifier)
  {
    return eCAL::SEntityId{ sample_identifier.entity_id
                          , sample_identifier.process_id
                          , sample_identifier.host_name };
  }

  // This creates a normalized SDataTypeInformation either from the new field
  // or from the legacy type/desc fields.
  eCAL::SDataTypeInformation GetDataTypeInformation(const eCAL::SDataTypeInformation& datatype_info_,
    const std::string& legacy_type_,
    const std::string& legacy_desc_)
  {
    if (!datatype_info_.name.empty())
      return datatype_info_;

    eCAL::SDataTypeInformation fused_datatype_info;
    fused_datatype_info.name = legacy_type_;
    fused_datatype_info.descriptor = legacy_desc_;
    return fused_datatype_info;
  }

  // Convert a "service-like" structure with .methods[] into ServiceMethodInformationSetT.
  template <typename MethodContainer>
  eCAL::ServiceMethodInformationSetT ConvertMethods(const MethodContainer& methods_container)
  {
    eCAL::ServiceMethodInformationSetT methods;
    for (const auto& method : methods_container)
    {
      const auto request_datatype = GetDataTypeInformation(method.request_datatype_information, method.req_type, method.req_desc);
      const auto response_datatype = GetDataTypeInformation(method.response_datatype_information, method.resp_type, method.resp_desc);
      methods.insert(eCAL::SServiceMethodInformation{ method.method_name, request_datatype, response_datatype });
    }
    return methods;
  }

  // Helper to notify all callbacks with a specific event type
  template <class CallbackMap>
  auto MakeNotifyLambda(CallbackMap& callbacks, eCAL::Registration::RegistrationEventType ev)
  {
    return [&callbacks, ev](const auto& tid)
      {
        const std::lock_guard<std::mutex> guard(callbacks.mtx);
        for (const auto& kv : callbacks.map)
          if (kv.second)
            kv.second(tid, ev);
      };
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
    return m_server_infos.GetIDs();
  }

  bool CDescGate::GetServerInfo(const SServiceId& id_, ServiceMethodInformationSetT& service_info_) const
  {
    return m_server_infos.GetInfo(id_, service_info_);
  }

  std::set<SServiceId> CDescGate::GetClientIDs() const
  {
    return m_client_infos.GetIDs();
  }

  bool CDescGate::GetClientInfo(const SServiceId& id_, ServiceMethodInformationSetT& service_info_) const
  {
    return m_client_infos.GetInfo(id_, service_info_);
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
      const auto methods = ConvertMethods(sample_.service.methods);
      m_server_infos.RegisterSample(sample_.identifier, sample_.service.service_name, methods);
      break;
    }

    case bct_unreg_service:
      m_server_infos.UnregisterSample(sample_);
      break;

    case bct_reg_client:
    {
      const auto methods = ConvertMethods(sample_.client.methods);
      m_client_infos.RegisterSample(sample_.identifier, sample_.client.service_name, methods);
      break;
    }

    case bct_unreg_client:
      m_client_infos.UnregisterSample(sample_);
      break;

    case bct_reg_publisher:
      m_publisher_infos.RegisterSample(sample_, MakeNotifyLambda(m_publisher_callback_map, eCAL::Registration::RegistrationEventType::new_entity));
      break;

    case bct_unreg_publisher:
      m_publisher_infos.UnregisterSample(sample_, MakeNotifyLambda(m_publisher_callback_map, eCAL::Registration::RegistrationEventType::deleted_entity));
      break;

    case bct_reg_subscriber:
      m_subscriber_infos.RegisterSample(sample_, MakeNotifyLambda(m_subscriber_callback_map, eCAL::Registration::RegistrationEventType::new_entity));
      break;

    case bct_unreg_subscriber:
      m_subscriber_infos.UnregisterSample(sample_, MakeNotifyLambda(m_subscriber_callback_map, eCAL::Registration::RegistrationEventType::deleted_entity));
      break;

    default:
      std::cerr << "CDescGate::ApplySample : unknown sample type\n";
      break;
    }
  }

  Registration::CallbackToken CDescGate::CreateToken()
  {
    // fetch_add returns old value; add 1 to make tokens start at 1
    return m_callback_token.fetch_add(1, std::memory_order_relaxed) + 1;
  }

  // ---------- CollectedTopicInfo ----------

  void CDescGate::CollectedTopicInfo::RegisterSample(const Registration::Sample& sample_,
    const std::function<void(const STopicId&)>& on_new_topic)
  {
    std::unique_lock<std::mutex> guard(mutex);

    auto it = map.find(sample_.identifier.entity_id);
    if (it != map.end())
    {
      // Update normalized datatype info (v6 should be invariant, but we keep it robust)
      it->second.datatype_info = sample_.topic.datatype_information;
      return;
    }

    const STopicId           topic_id{ ConvertToEntityId(sample_.identifier), sample_.topic.topic_name };
    SDataTypeInformation     datatype = sample_.topic.datatype_information;
    TopicInfo                topic_info{ topic_id, std::move(datatype) };

    map.emplace(sample_.identifier.entity_id, std::move(topic_info));

    // Unlock before invoking callback
    guard.unlock();
    on_new_topic(topic_id);
  }

  void CDescGate::CollectedTopicInfo::UnregisterSample(const Registration::Sample& sample_,
    const std::function<void(const STopicId&)>& on_erased_topic)
  {
    std::unique_lock<std::mutex> guard(mutex);

    auto it = map.find(sample_.identifier.entity_id);
    if (it == map.end())
      return;

    const STopicId erased_id = it->second.id;
    map.erase(it);

    guard.unlock();
    on_erased_topic(erased_id);
  }

  std::set<STopicId> CDescGate::CollectedTopicInfo::GetIDs() const
  {
    const std::lock_guard<std::mutex> guard(mutex);

    std::set<STopicId> topic_ids;
    for (const auto& map_iterator : map)
    {
      topic_ids.insert(map_iterator.second.id);
    }
    return topic_ids;
  }

  bool CDescGate::CollectedTopicInfo::GetInfo(const STopicId& id_, SDataTypeInformation& topic_info_) const
  {
    const std::lock_guard<std::mutex> guard(mutex);

    const auto iterator = map.find(id_.topic_id.entity_id);
    if (iterator == map.end())
      return false;

    topic_info_ = iterator->second.datatype_info;
    return true;
  }

  void CDescGate::CollectedServiceInfo::RegisterSample(const Registration::SampleIdentifier& sample_id_,
    const std::string& service_name_,
    const ServiceMethodInformationSetT& method_info_)
  {
    const std::lock_guard<std::mutex> guard(mutex);

    auto it = map.find(sample_id_.entity_id);
    if (it != map.end())
    {
      // Update existing service entry (e.g., datatype / method info refresh)
      it->second.service_method_information = method_info_;
      return;
    }

    // Insert new service entry
    const auto service_id = eCAL::SServiceId{ ConvertToEntityId(sample_id_), service_name_ };
    ServiceInfo service_info{ service_id, method_info_ };

    map.emplace(sample_id_.entity_id, std::move(service_info));
  }
   
  void CDescGate::CollectedServiceInfo::UnregisterSample(const Registration::Sample& sample_)
  {
    const std::lock_guard<std::mutex> guard(mutex);

    const auto iterator = map.find(sample_.identifier.entity_id);
    if (iterator != map.end())
    {
      map.erase(iterator);
    }
  }

  std::set<SServiceId> CDescGate::CollectedServiceInfo::GetIDs() const
  {
    const std::lock_guard<std::mutex> guard(mutex);

    std::set<SServiceId> service_ids;
    for (const auto& map_iterator : map)
    {
      service_ids.insert(map_iterator.second.id);
    }
    return service_ids;
  }

  bool CDescGate::CollectedServiceInfo::GetInfo(const SServiceId& id_, ServiceMethodInformationSetT& topic_info_) const
  {
    const std::lock_guard<std::mutex> guard(mutex);

    const auto iterator = map.find(id_.service_id.entity_id);

    if (iterator == map.end())
      return false;

    topic_info_ = iterator->second.service_method_information;
    return true;
  }
}
