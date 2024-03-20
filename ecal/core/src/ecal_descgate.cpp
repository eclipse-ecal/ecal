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

namespace eCAL
{
  CDescGate::CDescGate() :
    m_topic_info_map  (std::chrono::milliseconds(Config::GetMonitoringTimeoutMs())),
    m_service_info_map(std::chrono::milliseconds(Config::GetMonitoringTimeoutMs()))
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
      map.emplace(topic_info.first, topic_info.second);
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
    topic_info_ = (*topic_info_it).second;
    return(true);
  }

  void CDescGate::GetServices(std::map<std::tuple<std::string, std::string>, SServiceMethodInformation>& service_info_map_)
  {
    std::map<std::tuple<std::string, std::string>, SServiceMethodInformation> map;

    const std::lock_guard<std::mutex> lock(m_service_info_map.sync);
    m_service_info_map.map->remove_deprecated();

    for (const auto& service_info : (*m_service_info_map.map))
    {
      map.emplace(service_info.first, service_info.second);
    }
    service_info_map_.swap(map);
  }

  void CDescGate::GetServiceNames(std::vector<std::tuple<std::string, std::string>>& service_method_names_)
  {
    service_method_names_.clear();

    const std::lock_guard<std::mutex> lock(m_service_info_map.sync);
    m_service_info_map.map->remove_deprecated();
    service_method_names_.reserve(m_service_info_map.map->size());

    for (const auto& service_info : (*m_service_info_map.map))
    {
      service_method_names_.emplace_back(service_info.first);
    }
  }

  bool CDescGate::GetServiceTypeNames(const std::string& service_name_, const std::string& method_name_, std::string& req_type_name_, std::string& resp_type_name_)
  {
    std::tuple<std::string, std::string> service_method_tuple = std::make_tuple(service_name_, method_name_);

    const std::lock_guard<std::mutex> lock(m_service_info_map.sync);
    m_service_info_map.map->remove_deprecated();
    auto service_info_map_it = m_service_info_map.map->find(service_method_tuple);

    if (service_info_map_it == m_service_info_map.map->end()) return false;
    req_type_name_  = (*service_info_map_it).second.request_type.name;
    resp_type_name_ = (*service_info_map_it).second.response_type.name;
    return true;
  }

  bool CDescGate::GetServiceDescription(const std::string& service_name_, const std::string& method_name_, std::string& req_type_desc_, std::string& resp_type_desc_)
  {
    std::tuple<std::string, std::string> service_method_tuple = std::make_tuple(service_name_, method_name_);

    const std::lock_guard<std::mutex> lock(m_service_info_map.sync);
    m_service_info_map.map->remove_deprecated();
    auto service_info_map_it = m_service_info_map.map->find(service_method_tuple);

    if (service_info_map_it == m_service_info_map.map->end()) return false;
    req_type_desc_  = (*service_info_map_it).second.request_type.descriptor;
    resp_type_desc_ = (*service_info_map_it).second.response_type.descriptor;
    return true;
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
        SDataTypeInformation request_type;
        request_type.name       = method.req_type;
        request_type.descriptor = method.req_desc;

        SDataTypeInformation response_type{};
        response_type.name       = method.resp_type;
        response_type.descriptor = method.resp_desc;

        ApplyServiceDescription(sample_.service.sname, method.mname, request_type, response_type);
      }
    }
      break;
    case bct_unreg_service:
      break;
    case bct_reg_client:
      //for (const auto& method : sample_.client.methods)
      //{
      //  SDataTypeInformation request_type;
      //  request_type.name       = method.req_type;
      //  request_type.descriptor = method.req_desc;

      //  SDataTypeInformation response_type{};
      //  response_type.name       = method.resp_type;
      //  response_type.descriptor = method.resp_desc;

      //  ApplyClientDescription(sample_.service.sname, method.mname, request_type, response_type);
      //}
      break;
    case bct_unreg_client:
      break;
    case bct_reg_publisher:
      ApplyTopicDescription(sample_.topic.tname, sample_.topic.tdatatype);
      break;
    case bct_unreg_publisher:
      break;
    case bct_reg_subscriber:
      ApplyTopicDescription(sample_.topic.tname, sample_.topic.tdatatype);
      break;
    case bct_unreg_subscriber:
      break;
    default:
    {
      Logging::Log(log_level_debug1, "CDescGate::ApplySample : unknown sample type");
    }
    break;
    }

    return true;
  }

  bool CDescGate::ApplyTopicDescription(const std::string& topic_name_, const SDataTypeInformation& topic_info_)
  {
    const std::unique_lock<std::mutex> lock(m_topic_info_map.sync);
    m_topic_info_map.map->remove_deprecated();

    // update topic entry (and its timestamp)
    (*m_topic_info_map.map)[topic_name_] = topic_info_;
    return true;
  }

  bool CDescGate::ApplyServiceDescription(const std::string& service_name_
                                        , const std::string& method_name_
                                        , const SDataTypeInformation& request_type_information_
                                        , const SDataTypeInformation& response_type_information_)
  {
    std::tuple<std::string, std::string> service_method_tuple = std::make_tuple(service_name_, method_name_);

    const std::lock_guard<std::mutex> lock(m_service_info_map.sync);
    m_service_info_map.map->remove_deprecated();

    // aggregate service information
    SServiceMethodInformation service_info;
    service_info.request_type  = request_type_information_;
    service_info.response_type = response_type_information_;
    
    // update service entry (and its timestamp)
    (*m_service_info_map.map)[service_method_tuple] = service_info;
    return true;
  }
}
