/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2019 Continental Corporation
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

#include <ecal/ecal_log.h>
#include <ecal/ecal_config.h>

#include "ecal_globals.h"
#include "ecal_descgate.h"

namespace
{
  eCAL::Util::DescQualityFlags GetClientQuality(const eCAL::SDataTypeInformation& request_info_, const eCAL::SDataTypeInformation& response_info_)
  {
    eCAL::Util::DescQualityFlags quality = eCAL::Util::DescQualityFlags::NO_QUALITY;
    if (!(request_info_.name.empty() && response_info_.name.empty()))
      quality |= eCAL::Util::DescQualityFlags::TYPENAME_AVAILABLE;
    if (!(request_info_.encoding.empty() && response_info_.encoding.empty()))
      quality |= eCAL::Util::DescQualityFlags::ENCODING_AVAILABLE;
    if (!(request_info_.descriptor.empty() && response_info_.descriptor.empty()))
      quality |= eCAL::Util::DescQualityFlags::DESCRIPTION_AVAILABLE;
    return quality;
  }

  eCAL::Util::DescQualityFlags GetServiceQuality(const eCAL::SDataTypeInformation& request_info_, const eCAL::SDataTypeInformation& response_info_)
  {
    eCAL::Util::DescQualityFlags quality = GetClientQuality(request_info_, response_info_);
    quality |= eCAL::Util::DescQualityFlags::INFO_COMES_FROM_PRODUCER;
    return quality;
  }

  eCAL::Util::DescQualityFlags GetSubscriberQuality(const eCAL::SDataTypeInformation& topic_info_)
  {
    eCAL::Util::DescQualityFlags quality = eCAL::Util::DescQualityFlags::NO_QUALITY;
    if (!topic_info_.name.empty())
      quality |= eCAL::Util::DescQualityFlags::TYPENAME_AVAILABLE;
    if (!topic_info_.encoding.empty())
      quality |= eCAL::Util::DescQualityFlags::ENCODING_AVAILABLE;
    if (!topic_info_.descriptor.empty())
      quality |= eCAL::Util::DescQualityFlags::DESCRIPTION_AVAILABLE;
    return quality;
  }

  eCAL::Util::DescQualityFlags GetPublisherQuality(const eCAL::SDataTypeInformation& topic_info_)
  {
    eCAL::Util::DescQualityFlags quality = GetSubscriberQuality(topic_info_);
    quality |= eCAL::Util::DescQualityFlags::INFO_COMES_FROM_PRODUCER;
    return quality;
  }
}

namespace eCAL
{
  CDescGate::CDescGate() :
    m_publisher_info_map  (std::chrono::milliseconds(Config::GetMonitoringTimeoutMs())),
    m_subscriber_info_map (std::chrono::milliseconds(Config::GetMonitoringTimeoutMs())),
    m_service_info_map    (std::chrono::milliseconds(Config::GetMonitoringTimeoutMs())),
    m_client_info_map     (std::chrono::milliseconds(Config::GetMonitoringTimeoutMs()))
  {
  }
  CDescGate::~CDescGate() = default;

  void CDescGate::Create()
  {
#if ECAL_CORE_REGISTRATION
    // utilize registration provider and receiver to get descriptions
    g_registration_provider()->SetCustomApplySampleCallback("descgate", [this](const auto& sample_) {this->ApplySample(sample_, tl_none); });
    g_registration_receiver()->SetCustomApplySampleCallback("descgate", [this](const auto& sample_) {this->ApplySample(sample_, tl_none); });
#endif
  }

  void CDescGate::Destroy()
  {
#if ECAL_CORE_REGISTRATION
    // stop registration provider and receiver utilization to get descriptions
    g_registration_provider()->RemCustomApplySampleCallback("descgate");
    g_registration_receiver()->RemCustomApplySampleCallback("descgate");
#endif
  }

  CDescGate::TopicNameIdMap CDescGate::GetPublisher()
  {
    return GetTopics(m_publisher_info_map);
  }

  CDescGate::TopicNameIdMap CDescGate::GetSubscriber()
  {
    return GetTopics(m_subscriber_info_map);
  }

  CDescGate::ServiceMethodNameIdMap CDescGate::GetServices()
  {
    return GetServices(m_service_info_map);
  }

  CDescGate::ServiceMethodNameIdMap CDescGate::GetClients()
  {
    return GetServices(m_client_info_map);
  }

  CDescGate::TopicNameIdMap CDescGate::GetTopics(const STopicInfoMap& topic_map_)
  {
    TopicNameIdMap map;

    const std::lock_guard<std::mutex> lock(topic_map_.sync);
    topic_map_.map->remove_deprecated();

    for (const auto& topic_map_it : (*topic_map_.map))
    {
      map.emplace(topic_map_it.first, topic_map_it.second);
    }
    return map;
  }

  CDescGate::ServiceMethodNameIdMap CDescGate::GetServices(const SServiceMethodInfoMap& service_method_info_map_)
  {
    ServiceMethodNameIdMap map;

    const std::lock_guard<std::mutex> lock(service_method_info_map_.sync);
    service_method_info_map_.map->remove_deprecated();

    for (const auto& service_method_info_map_it : (*service_method_info_map_.map))
    {
      map.emplace(service_method_info_map_it.first, service_method_info_map_it.second);
    }
    return map;
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

        ApplyServiceDescription(m_service_info_map, sample_.service.sname, sample_.service.sid, method.mname, request_type, response_type, GetServiceQuality(request_type, response_type));
      }
    }
    break;
    case bct_unreg_service:
      RemServiceDescription(m_service_info_map, sample_.service.sname, sample_.service.sid);
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

        ApplyServiceDescription(m_client_info_map, sample_.client.sname, sample_.client.sid, method.mname, request_type, response_type, GetClientQuality(request_type, response_type));
      }
      break;
    case bct_unreg_client:
      RemServiceDescription(m_client_info_map, sample_.client.sname, sample_.client.sid);
      break;
    case bct_reg_publisher:
      ApplyTopicDescription(m_publisher_info_map, sample_.topic.tname, sample_.topic.tid, sample_.topic.tdatatype, GetPublisherQuality(sample_.topic.tdatatype));
      break;
    case bct_unreg_publisher:
      RemTopicDescription(m_publisher_info_map, sample_.topic.tname, sample_.topic.tid);
      break;
    case bct_reg_subscriber:
      ApplyTopicDescription(m_subscriber_info_map, sample_.topic.tname, sample_.topic.tid, sample_.topic.tdatatype, GetSubscriberQuality(sample_.topic.tdatatype));
      break;
    case bct_unreg_subscriber:
      RemTopicDescription(m_subscriber_info_map, sample_.topic.tname, sample_.topic.tid);
      break;
    default:
    {
      Logging::Log(log_level_debug1, "CDescGate::ApplySample : unknown sample type");
    }
    break;
    }
  }

  void CDescGate::ApplyTopicDescription(STopicInfoMap& topic_info_map_,
    const std::string& topic_name_,
    const std::string& topic_id_,
    const SDataTypeInformation& topic_info_,
    Util::DescQualityFlags topic_quality_)
  {
    const auto topic_info_key = STopicNameId{ topic_name_, topic_id_ };

    Util::SQualityDataTypeInformation topic_quality_info;
    topic_quality_info.id      = topic_id_;
    topic_quality_info.info    = topic_info_;
    topic_quality_info.quality = topic_quality_;

    const std::unique_lock<std::mutex> lock(topic_info_map_.sync);
    topic_info_map_.map->remove_deprecated();

    const auto topic_info_it = topic_info_map_.map->find(topic_info_key);

    (*topic_info_map_.map)[topic_info_key] = topic_quality_info;
  }

  void CDescGate::RemTopicDescription(STopicInfoMap& topic_info_map_, const std::string& topic_name_, const std::string& topic_id_)
  {
    const std::unique_lock<std::mutex> lock(topic_info_map_.sync);
    topic_info_map_.map->remove_deprecated();
    topic_info_map_.map->erase(STopicNameId{ topic_name_, topic_id_ });
  }

  void CDescGate::ApplyServiceDescription(SServiceMethodInfoMap& service_method_info_map_,
    const std::string& service_name_,
    const std::string& service_id_,
    const std::string& method_name_,
    const SDataTypeInformation& request_type_information_,
    const SDataTypeInformation& response_type_information_,
    const Util::DescQualityFlags service_quality_)
  {
    const auto service_method_info_key = SServiceMethodNameId{ service_name_, service_id_, method_name_ };

    Util::SQualityServiceMethodInformation service_quality_info;
    service_quality_info.id                 = service_id_;
    service_quality_info.info.request_type  = request_type_information_;
    service_quality_info.info.response_type = response_type_information_;
    service_quality_info.quality            = service_quality_;

    const std::lock_guard<std::mutex> lock(service_method_info_map_.sync);
    service_method_info_map_.map->remove_deprecated();
    (*service_method_info_map_.map)[service_method_info_key] = service_quality_info;
  }

  void CDescGate::RemServiceDescription(SServiceMethodInfoMap& service_method_info_map_, const std::string& service_name_, const std::string& service_id_)
  {
    std::list<SServiceMethodNameId> service_method_infos_to_remove;

    const std::lock_guard<std::mutex> lock(service_method_info_map_.sync);
    service_method_info_map_.map->remove_deprecated();

    for (auto&& service_it : *service_method_info_map_.map)
    {
      const auto service_method_info = service_it.first;
      if ((service_method_info.service_name == service_name_)
        && (service_method_info.service_id == service_id_))
      {
        service_method_infos_to_remove.push_back(service_method_info);
      }
    }

    for (const auto& service_method_info : service_method_infos_to_remove)
    {
      (*service_method_info_map_.map).erase(service_method_info);
    }
  }
}
