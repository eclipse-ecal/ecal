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

#include "ecal_descgate.h"

#include <iostream>

namespace
{
  eCAL::Util::DescQualityFlags GetDataTypeInfoQuality(const eCAL::SDataTypeInformation& data_type_info_, bool is_producer_)
  {
    eCAL::Util::DescQualityFlags quality = eCAL::Util::DescQualityFlags::NO_QUALITY;
    if (!data_type_info_.name.empty())
      quality |= eCAL::Util::DescQualityFlags::TYPENAME_AVAILABLE;
    if (!data_type_info_.encoding.empty())
      quality |= eCAL::Util::DescQualityFlags::ENCODING_AVAILABLE;
    if (!data_type_info_.descriptor.empty())
      quality |= eCAL::Util::DescQualityFlags::DESCRIPTION_AVAILABLE;
    if(is_producer_) quality |= eCAL::Util::DescQualityFlags::INFO_COMES_FROM_PRODUCER;
    return quality;
  }
}

namespace eCAL
{
  CDescGate::CDescGate(const std::chrono::milliseconds& exp_timeout_) :
    m_publisher_info_map  (exp_timeout_),
    m_subscriber_info_map (exp_timeout_),
    m_service_info_map    (exp_timeout_),
    m_client_info_map     (exp_timeout_)
  {
  }
  CDescGate::~CDescGate() = default;

  QualityTopicIdMap CDescGate::GetPublishers()
  {
    return GetTopics(m_publisher_info_map);
  }

  QualityTopicIdMap CDescGate::GetSubscribers()
  {
    return GetTopics(m_subscriber_info_map);
  }

  QualityServiceIdMap CDescGate::GetServices()
  {
    return GetServices(m_service_info_map);
  }

  QualityServiceIdMap CDescGate::GetClients()
  {
    return GetServices(m_client_info_map);
  }

  QualityTopicIdMap CDescGate::GetTopics(SQualityTopicIdMap& topic_map_)
  {
    QualityTopicIdMap map;

    const std::lock_guard<std::mutex> lock(topic_map_.mtx);
    topic_map_.map.remove_deprecated();

    for (const auto& topic_map_it : topic_map_.map)
    {
      map.emplace(topic_map_it.first, topic_map_it.second);
    }
    return map;
  }

  QualityServiceIdMap CDescGate::GetServices(SQualityServiceIdMap& service_method_info_map_)
  {
    QualityServiceIdMap map;

    const std::lock_guard<std::mutex> lock(service_method_info_map_.mtx);
    service_method_info_map_.map.remove_deprecated();

    for (const auto& service_method_info_map_it : service_method_info_map_.map)
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

        ApplyServiceDescription(m_service_info_map, sample_.service.sname, method.mname, std::stoull(sample_.service.sid), request_type, response_type, GetDataTypeInfoQuality(request_type, true), GetDataTypeInfoQuality(response_type, true));
      }
    }
    break;
    case bct_unreg_service:
      RemServiceDescription(m_service_info_map, sample_.service.sname, std::stoull(sample_.service.sid));
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

        ApplyServiceDescription(m_client_info_map, sample_.client.sname, method.mname, std::stoull(sample_.client.sid), request_type, response_type, GetDataTypeInfoQuality(request_type, false), GetDataTypeInfoQuality(response_type, false));
      }
      break;
    case bct_unreg_client:
      RemServiceDescription(m_client_info_map, sample_.client.sname, std::stoull(sample_.client.sid));
      break;
    case bct_reg_publisher:
      ApplyTopicDescription(m_publisher_info_map, sample_.topic.tname, std::stoull(sample_.topic.tid), sample_.topic.tdatatype, GetDataTypeInfoQuality(sample_.topic.tdatatype, true));
      break;
    case bct_unreg_publisher:
      RemTopicDescription(m_publisher_info_map, sample_.topic.tname, std::stoull(sample_.topic.tid));
      break;
    case bct_reg_subscriber:
      ApplyTopicDescription(m_subscriber_info_map, sample_.topic.tname, std::stoull(sample_.topic.tid), sample_.topic.tdatatype, GetDataTypeInfoQuality(sample_.topic.tdatatype, false));
      break;
    case bct_unreg_subscriber:
      RemTopicDescription(m_subscriber_info_map, sample_.topic.tname, std::stoull(sample_.topic.tid));
      break;
    default:
    {
      std::cerr << "CDescGate::ApplySample : unknown sample type" << '\n';
    }
    break;
    }
  }

  void CDescGate::ApplyTopicDescription(SQualityTopicIdMap& topic_info_map_,
    const std::string& topic_name_,
    const Util::TopicId& topic_id_,
    const SDataTypeInformation& topic_info_,
    const Util::DescQualityFlags topic_quality_)
  {
    const auto topic_info_key = STopicIdKey{ topic_name_, topic_id_ };

    Util::SQualityTopicInfo topic_quality_info;
    topic_quality_info.id      = topic_id_;
    topic_quality_info.info    = topic_info_;
    topic_quality_info.quality = topic_quality_;

    const std::unique_lock<std::mutex> lock(topic_info_map_.mtx);
    topic_info_map_.map.remove_deprecated();
    topic_info_map_.map[topic_info_key] = topic_quality_info;
  }

  void CDescGate::RemTopicDescription(SQualityTopicIdMap& topic_info_map_, const std::string& topic_name_, const Util::TopicId& topic_id_)
  {
    const std::unique_lock<std::mutex> lock(topic_info_map_.mtx);
    topic_info_map_.map.remove_deprecated();
    topic_info_map_.map.erase(STopicIdKey{ topic_name_, topic_id_ });
  }

  void CDescGate::ApplyServiceDescription(SQualityServiceIdMap& service_method_info_map_,
    const std::string& service_name_,
    const std::string& method_name_,
    const Util::ServiceId& service_id_,
    const SDataTypeInformation& request_type_information_,
    const SDataTypeInformation& response_type_information_,
    const Util::DescQualityFlags request_type_quality_,
    const Util::DescQualityFlags response_type_quality_)
  {
    const auto service_method_info_key = SServiceIdKey{ service_name_, method_name_, service_id_};

    Util::SQualityServiceInfo service_quality_info;
    service_quality_info.id                 = service_id_;
    service_quality_info.info.request_type  = request_type_information_;
    service_quality_info.info.response_type = response_type_information_;
    service_quality_info.request_quality    = request_type_quality_;
    service_quality_info.response_quality   = response_type_quality_;

    const std::lock_guard<std::mutex> lock(service_method_info_map_.mtx);
    service_method_info_map_.map.remove_deprecated();
    service_method_info_map_.map[service_method_info_key] = service_quality_info;
  }

  void CDescGate::RemServiceDescription(SQualityServiceIdMap& service_method_info_map_, const std::string& service_name_, const Util::ServiceId& service_id_)
  {
    std::list<SServiceIdKey> service_method_infos_to_remove;

    const std::lock_guard<std::mutex> lock(service_method_info_map_.mtx);
    service_method_info_map_.map.remove_deprecated();

    for (auto&& service_it : service_method_info_map_.map)
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
      service_method_info_map_.map.erase(service_method_info);
    }
  }
}
