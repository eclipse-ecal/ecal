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

#include <algorithm>

namespace
{
  // TODO: remove me with new CDescGate
  eCAL::CDescGate::QualityFlags GetServiceMethodQuality(const eCAL::SDataTypeInformation& request_info_, const eCAL::SDataTypeInformation& response_info_)
  {
    eCAL::CDescGate::QualityFlags quality = eCAL::CDescGate::QualityFlags::NO_QUALITY;
    if (!(request_info_.name.empty() && response_info_.name.empty()))
      quality |= eCAL::CDescGate::QualityFlags::TYPE_AVAILABLE;
    if (!(request_info_.descriptor.empty() && response_info_.descriptor.empty()))
      quality |= eCAL::CDescGate::QualityFlags::DESCRIPTION_AVAILABLE;
    quality |= eCAL::CDescGate::QualityFlags::INFO_COMES_FROM_PRODUCER;

    return quality;
  }

  // TODO: remove me with new CDescGate
  eCAL::CDescGate::QualityFlags GetClientMethodQuality(const eCAL::SDataTypeInformation& request_info_, const eCAL::SDataTypeInformation& response_info_)
  {
    eCAL::CDescGate::QualityFlags quality = eCAL::CDescGate::QualityFlags::NO_QUALITY;
    if (!(request_info_.name.empty() && response_info_.name.empty()))
      quality |= eCAL::CDescGate::QualityFlags::TYPE_AVAILABLE;
    if (!(request_info_.descriptor.empty() && response_info_.descriptor.empty()))
      quality |= eCAL::CDescGate::QualityFlags::DESCRIPTION_AVAILABLE;

    return quality;
  }

  // TODO: remove me with new CDescGate
  eCAL::CDescGate::QualityFlags GetPublisherQuality(const eCAL::SDataTypeInformation& topic_info_)
  {
    eCAL::CDescGate::QualityFlags quality = eCAL::CDescGate::QualityFlags::NO_QUALITY;
    if (!topic_info_.name.empty() || !topic_info_.encoding.empty())
      quality |= eCAL::CDescGate::QualityFlags::TYPE_AVAILABLE;
    if (!topic_info_.descriptor.empty())
      quality |= eCAL::CDescGate::QualityFlags::DESCRIPTION_AVAILABLE;
    quality |= eCAL::CDescGate::QualityFlags::INFO_COMES_FROM_CORRECT_ENTITY;
    quality |= eCAL::CDescGate::QualityFlags::INFO_COMES_FROM_PRODUCER;

    return quality;
  }

  // TODO: remove me with new CDescGate
  eCAL::CDescGate::QualityFlags GetSubscriberQuality(const eCAL::SDataTypeInformation& topic_info_)
  {
    eCAL::CDescGate::QualityFlags quality = eCAL::CDescGate::QualityFlags::NO_QUALITY;
    if (!topic_info_.name.empty() || !topic_info_.encoding.empty())
      quality |= eCAL::CDescGate::QualityFlags::TYPE_AVAILABLE;
    if (!topic_info_.descriptor.empty())
      quality |= eCAL::CDescGate::QualityFlags::DESCRIPTION_AVAILABLE;
    quality |= eCAL::CDescGate::QualityFlags::INFO_COMES_FROM_CORRECT_ENTITY;

    return quality;
  }
}

namespace eCAL
{
  CDescGate::CDescGate() :
    m_topic_info_map(std::chrono::milliseconds(Config::GetMonitoringTimeoutMs())),
    m_service_info_map(std::chrono::milliseconds(Config::GetMonitoringTimeoutMs())),
    m_client_info_map(std::chrono::milliseconds(Config::GetMonitoringTimeoutMs()))
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

  void CDescGate::GetTopics(std::unordered_map<std::string, SDataTypeInformation>& topic_info_map_)
  {
    std::unordered_map<std::string, SDataTypeInformation> map;

    const std::lock_guard<std::mutex> lock(m_topic_info_map.sync);
    m_topic_info_map.map->remove_deprecated();
    map.reserve(m_topic_info_map.map->size());

    for (const auto& topic_info : (*m_topic_info_map.map))
    {
      map.emplace(topic_info.first, topic_info.second.info);
    }
    topic_info_map_.swap(map);
  }

  void CDescGate::GetTopicNames(std::vector<std::string>& topic_names_)
  {
    topic_names_.clear();

    const std::lock_guard<std::mutex> lock(m_topic_info_map.sync);
    m_topic_info_map.map->remove_deprecated();
    topic_names_.reserve(m_topic_info_map.map->size());

    for (const auto& topic_info : (*m_topic_info_map.map))
    {
      topic_names_.emplace_back(topic_info.first);
    }
  }

  bool CDescGate::GetDataTypeInformation(const std::string& topic_name_, SDataTypeInformation& topic_info_)
  {
    if (topic_name_.empty()) return(false);

    const std::lock_guard<std::mutex> lock(m_topic_info_map.sync);
    m_topic_info_map.map->remove_deprecated();
    const auto topic_info_it = m_topic_info_map.map->find(topic_name_);

    if (topic_info_it == m_topic_info_map.map->end()) return(false);
    topic_info_ = (*topic_info_it).second.info;
    return(true);
  }

  void CDescGate::GetServices(std::map<std::tuple<std::string, std::string>, SServiceMethodInformation>& service_info_map_)
  {
    GetServices(service_info_map_, m_service_info_map);
  }

  void CDescGate::GetServiceMethodNames(std::vector<std::tuple<std::string, std::string>>& service_method_names_)
  {
    GetServiceMethodNames(service_method_names_, m_service_info_map);
  }

  bool CDescGate::GetServiceTypeNames(const std::string& service_name_, const std::string& method_name_, std::string& req_type_name_, std::string& resp_type_name_)
  {
    return GetServiceTypeNames(service_name_, method_name_, req_type_name_, resp_type_name_, m_service_info_map);
  }

  bool CDescGate::GetServiceDescription(const std::string& service_name_, const std::string& method_name_, std::string& req_type_desc_, std::string& resp_type_desc_)
  {
    return GetServiceDescription(service_name_, method_name_, req_type_desc_, resp_type_desc_, m_service_info_map);
  }

  void CDescGate::GetClients(std::map<std::tuple<std::string, std::string>, SServiceMethodInformation>& service_info_map_)
  {
    GetServices(service_info_map_, m_client_info_map);
  }

  void CDescGate::GetClientMethodNames(std::vector<std::tuple<std::string, std::string>>& client_method_names_)
  {
    GetServiceMethodNames(client_method_names_, m_client_info_map);
  }

  bool CDescGate::GetClientTypeNames(const std::string& service_name_, const std::string& method_name_, std::string& req_type_name_, std::string& resp_type_name_)
  {
    return GetServiceTypeNames(service_name_, method_name_, req_type_name_, resp_type_name_, m_client_info_map);
  }

  bool CDescGate::GetClientDescription(const std::string& service_name_, const std::string& method_name_, std::string& req_type_desc_, std::string& resp_type_desc_)
  {
    return GetServiceDescription(service_name_, method_name_, req_type_desc_, resp_type_desc_, m_client_info_map);
  }

  bool CDescGate::ApplySample(const Registration::Sample& sample_, eTLayerType /*layer_*/)
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

        ApplyServiceDescription(m_service_info_map, sample_.service.sname, sample_.service.sid, method.mname, request_type, response_type, GetServiceMethodQuality(request_type, response_type));
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

        ApplyServiceDescription(m_client_info_map, sample_.client.sname, sample_.client.sid, method.mname, request_type, response_type, GetClientMethodQuality(request_type, response_type));
      }
      break;
    case bct_unreg_client:
      RemServiceDescription(m_client_info_map, sample_.client.sname, sample_.client.sid);
      break;
    case bct_reg_publisher:
      ApplyTopicDescription(sample_.topic.tname, sample_.topic.tid, sample_.topic.tdatatype, GetPublisherQuality(sample_.topic.tdatatype));
      break;
    case bct_unreg_publisher:
      RemTopicDescription(sample_.topic.tname, sample_.topic.tid);
      break;
    case bct_reg_subscriber:
      ApplyTopicDescription(sample_.topic.tname, sample_.topic.tid, sample_.topic.tdatatype, GetSubscriberQuality(sample_.topic.tdatatype));
      break;
    case bct_unreg_subscriber:
      RemTopicDescription(sample_.topic.tname, sample_.topic.tid);
      break;
    default:
    {
      Logging::Log(log_level_debug1, "CDescGate::ApplySample : unknown sample type");
    }
    break;
    }

    return true;
  }

  bool CDescGate::ApplyTopicDescription(const std::string& topic_name_, const std::string& topic_id_, const SDataTypeInformation& topic_info_, const QualityFlags description_quality_)
  {
    const std::unique_lock<std::mutex> lock(m_topic_info_map.sync);
    m_topic_info_map.map->remove_deprecated();

    const auto topic_info_it = m_topic_info_map.map->find(topic_name_);

    // new element (no need to check anything, just add it)
    if (topic_info_it == m_topic_info_map.map->end())
    {
      // create a new topic entry
      STopicInfoQuality& topic_info = (*m_topic_info_map.map)[topic_name_];
      topic_info.id = topic_id_;
      topic_info.info = topic_info_;
      topic_info.quality = description_quality_;
      return true;
    }

    // we do not use the [] operator here to not update the timestamp
    // by accessing the map entry
    // 
    // a topic with the same name but different type name or different description
    // should NOT update the timestamp of an existing entry
    // 
    // otherwise there could be a scenario where a "lower quality topic" would keep a 
    // "higher quality topic" alive (even it is no more existing)
    STopicInfoQuality topic_info = (*topic_info_it).second;

    // first let's check whether the current information has a higher quality
    // if it has a higher quality, we overwrite it
    if (description_quality_ > topic_info.quality)
    {
      // overwrite attributes
      topic_info.id = topic_id_;
      topic_info.info = topic_info_;
      topic_info.quality = description_quality_;

      // update attributes and return
      (*m_topic_info_map.map)[topic_name_] = topic_info;
      return true;
    }

    // this is the same topic (topic name, topic type name, topic type description)
    // independing from the unique id
    if (topic_info.info == topic_info_)
    {
      // update timestamp (by just accessing the entry) and return
      (*m_topic_info_map.map)[topic_name_] = topic_info;
      return false;
    }

    // topic type name or topic description differ but we logged this before
    if (topic_info.type_missmatch_logged)
    {
      return false;
    }

    // topic type name or topic description differ and this is not logged yet
    // so we log the error and update the entry one time
    bool update_topic_info(false);

    // topic type name differs
    // we log the error and update the entry one time
    if (!topic_info_.encoding.empty()
      && !topic_info.info.encoding.empty()
      && (topic_info.info.encoding != topic_info_.encoding)
      )
    {
      std::string tencoding1 = topic_info.info.encoding;
      std::string tencoding2 = topic_info_.encoding;
      std::replace(tencoding1.begin(), tencoding1.end(), '\0', '?');
      std::replace(tencoding1.begin(), tencoding1.end(), '\t', '?');
      std::replace(tencoding2.begin(), tencoding2.end(), '\0', '?');
      std::replace(tencoding2.begin(), tencoding2.end(), '\t', '?');
      std::string msg = "eCAL Pub/Sub encoding mismatch for topic ";
      msg += topic_name_;
      msg += " (\'";
      msg += tencoding1;
      msg += "\' <> \'";
      msg += tencoding2;
      msg += "\')";
      eCAL::Logging::Log(log_level_warning, msg);

      // mark as logged
      topic_info.type_missmatch_logged = true;
      // and update its attributes
      update_topic_info = true;
    }

    // topic type name differs
    // we log the error and update the entry one time
    if (!topic_info_.name.empty()
      && !topic_info.info.name.empty()
      && (topic_info.info.name != topic_info_.name)
      )
    {
      std::string ttype1 = topic_info.info.name;
      std::string ttype2 = topic_info_.name;
      std::replace(ttype1.begin(), ttype1.end(), '\0', '?');
      std::replace(ttype1.begin(), ttype1.end(), '\t', '?');
      std::replace(ttype2.begin(), ttype2.end(), '\0', '?');
      std::replace(ttype2.begin(), ttype2.end(), '\t', '?');
      std::string msg = "eCAL Pub/Sub type mismatch for topic ";
      msg += topic_name_;
      msg += " (\'";
      msg += ttype1;
      msg += "\' <> \'";
      msg += ttype2;
      msg += "\')";
      eCAL::Logging::Log(log_level_warning, msg);

      // mark as logged
      topic_info.type_missmatch_logged = true;
      // and update its attributes
      update_topic_info = true;
    }

    // topic type description differs
    // we log the error and update the entry one time
    if (!topic_info_.descriptor.empty()
      && !topic_info.info.descriptor.empty()
      && (topic_info.info.descriptor != topic_info_.descriptor)
      )
    {
      std::string msg = "eCAL Pub/Sub description mismatch for topic ";
      msg += topic_name_;
      eCAL::Logging::Log(log_level_warning, msg);

      // mark as logged
      topic_info.type_missmatch_logged = true;
      // and update its attributes
      update_topic_info = true;
    }

    // update topic info attributes
    if (update_topic_info)
    {
      (*m_topic_info_map.map)[topic_name_] = topic_info;
    }

    return false;
  }

  bool CDescGate::RemTopicDescription(const std::string& topic_name_, const std::string& topic_id_)
  {
    const std::unique_lock<std::mutex> lock(m_topic_info_map.sync);
    m_topic_info_map.map->remove_deprecated();

    const auto topic_info_it = m_topic_info_map.map->find(topic_name_);
    if (topic_info_it != m_topic_info_map.map->end())
    {
      STopicInfoQuality& topic_info = (*m_topic_info_map.map)[topic_name_];
      if (topic_info.id == topic_id_)
      {
        m_topic_info_map.map->erase(topic_name_);
        return true;
      }
    }
    return false;
  }

  bool CDescGate::ApplyServiceDescription(SServiceMethodInfoMap& service_method_map_
    , const std::string& service_name_
    , const std::string& service_id_
    , const std::string& method_name_
    , const SDataTypeInformation& request_type_information_
    , const SDataTypeInformation& response_type_information_
    , const QualityFlags description_quality_)
  {
    std::tuple<std::string, std::string> service_method_tuple = std::make_tuple(service_name_, method_name_);

    const std::lock_guard<std::mutex> lock(service_method_map_.sync);
    service_method_map_.map->remove_deprecated();

    SServiceMethodInfoQuality req_service_info;
    req_service_info.id = service_id_;
    req_service_info.info.request_type = request_type_information_;
    req_service_info.info.response_type = response_type_information_;
    req_service_info.quality = description_quality_;

    // create a new entry if service is not existing and return
    auto service_info_map_it = service_method_map_.map->find(service_method_tuple);
    if (service_info_map_it == service_method_map_.map->end())
    {
      (*service_method_map_.map)[service_method_tuple] = req_service_info;
      return true;
    }

    // service is existing, the requested service has a higher quality
    // overwrite the existing entry with the new attributes
    SServiceMethodInfoQuality service_info = (*service_info_map_it).second;
    if (description_quality_ > service_info.quality)
    {
      (*service_method_map_.map)[service_method_tuple] = req_service_info;
      return true;
    }

    // the entry is existing, the requested quality is not higher
    // update the existing entry if it has the same attributes (independing from the unique id)
    if (service_info.info == req_service_info.info)
    {
      (*service_method_map_.map)[service_method_tuple] = req_service_info;
      return true;
    }

    return false;
  }

  bool CDescGate::RemServiceDescription(SServiceMethodInfoMap& service_method_map_, const std::string& service_name_, const std::string& service_id_)
  {
    std::list<std::tuple<std::string, std::string>> service_method_tuple_to_remove;

    const std::lock_guard<std::mutex> lock(service_method_map_.sync);
    service_method_map_.map->remove_deprecated();

    bool success(false);
    for (auto&& service : *service_method_map_.map)
    {
      auto service_method_tuple = service.first;
      if (std::get<0>(service_method_tuple) == service_name_)
      {
        auto service_info = service.second;
        if (service_info.id == service_id_)
        {
          service_method_tuple_to_remove.push_back(service_method_tuple);
          success =  true;
        }
      }
    }

    for (const auto& service_method_tuple : service_method_tuple_to_remove)
    {
      (*service_method_map_.map).erase(service_method_tuple);
    }

    return success;
  }

  void CDescGate::GetServices(std::map<std::tuple<std::string, std::string>, SServiceMethodInformation>& service_info_map_, const SServiceMethodInfoMap& service_method_map_)
  {
    std::map<std::tuple<std::string, std::string>, SServiceMethodInformation> map;

    const std::lock_guard<std::mutex> lock(service_method_map_.sync);
    service_method_map_.map->remove_deprecated();

    for (const auto& service_info : (*service_method_map_.map))
    {
      map.emplace(service_info.first, service_info.second.info);
    }
    service_info_map_.swap(map);
  }

  void CDescGate::GetServiceMethodNames(std::vector<std::tuple<std::string, std::string>>& method_names_, const SServiceMethodInfoMap& service_method_map_)
  {
    method_names_.clear();

    const std::lock_guard<std::mutex> lock(service_method_map_.sync);
    service_method_map_.map->remove_deprecated();
    method_names_.reserve(service_method_map_.map->size());

    for (const auto& service_info : (*service_method_map_.map))
    {
      method_names_.emplace_back(service_info.first);
    }
  }

  bool CDescGate::GetServiceTypeNames(const std::string& service_name_, const std::string& method_name_, std::string& req_type_name_, std::string& resp_type_name_, const SServiceMethodInfoMap& service_method_map_)
  {
    std::tuple<std::string, std::string> service_method_tuple = std::make_tuple(service_name_, method_name_);

    const std::lock_guard<std::mutex> lock(service_method_map_.sync);
    service_method_map_.map->remove_deprecated();
    auto service_info_map_it = service_method_map_.map->find(service_method_tuple);

    if (service_info_map_it == service_method_map_.map->end()) return false;
    req_type_name_ = (*service_info_map_it).second.info.request_type.name;
    resp_type_name_ = (*service_info_map_it).second.info.response_type.name;
    return true;
  }

  bool CDescGate::GetServiceDescription(const std::string& service_name_, const std::string& method_name_, std::string& req_type_desc_, std::string& resp_type_desc_, const SServiceMethodInfoMap& service_method_map_)
  {
    std::tuple<std::string, std::string> service_method_tuple = std::make_tuple(service_name_, method_name_);

    const std::lock_guard<std::mutex> lock(service_method_map_.sync);
    service_method_map_.map->remove_deprecated();
    auto service_info_map_it = service_method_map_.map->find(service_method_tuple);

    if (service_info_map_it == service_method_map_.map->end()) return false;
    req_type_desc_ = (*service_info_map_it).second.info.request_type.descriptor;
    resp_type_desc_ = (*service_info_map_it).second.info.response_type.descriptor;
    return true;
  }
}
