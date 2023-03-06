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

#include "ecal_descgate.h"
#include <assert.h>
#include <algorithm>
#include <mutex>

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
  }

  void CDescGate::Destroy()
  {
  }

  void CDescGate::ApplyTopicDescription(const std::string& topic_name_, const std::string& topic_type_, const std::string& topic_desc_, const QualityFlags description_quality_)
  {
    std::unique_lock<std::shared_timed_mutex> lock(m_topic_info_map.sync);
    m_topic_info_map.map->remove_deprecated();

    const auto topic_info_it = m_topic_info_map.map->find(topic_name_);
    
    // new element (no need to check anything, just add it)
    if(topic_info_it == m_topic_info_map.map->end())
    {
      // TODO: Maybe we should copy the description from another topic with the same type, if it is empty?
      STopicInfoQuality& topic_info = (*m_topic_info_map.map)[topic_name_];
      topic_info.info.type_name        = topic_type_;
      topic_info.info.type_description = topic_desc_;
      topic_info.description_quality   = description_quality_;
    }
    else
    {
      // existing type for the same topic name should be equal !!
      // we log the error only one time
      STopicInfoQuality& topic_info = (*m_topic_info_map.map)[topic_name_];

      if(  !topic_info.type_missmatch_logged
            && !topic_type_.empty()
            && !topic_info.info.type_name.empty()
            && (topic_info.info.type_name != topic_type_)
        )
      {
        std::string ttype1 = topic_info.info.type_name;
        std::string ttype2 = topic_type_;
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

        topic_info.type_missmatch_logged = true;
      }

      // existing description for the same topic name should be equal !!
      // we log the warning only one time
      if(   !topic_info.type_missmatch_logged
            && !topic_desc_.empty()
            && !topic_info.info.type_description.empty()
            && (topic_info.info.type_description != topic_desc_)
        )
      {
        std::string msg = "eCAL Pub/Sub description mismatch for topic ";
        msg += topic_name_;
        eCAL::Logging::Log(log_level_warning, msg);

        topic_info.type_missmatch_logged = true;
      }

      // Now let's check whether the current information has a higher quality.
      // If it has a higher quality, we overwrite it.
      if (description_quality_ > topic_info.description_quality)
      {
        topic_info.info.type_name        = topic_type_;
        topic_info.info.type_description = topic_desc_;
        topic_info.description_quality   = description_quality_;
      }
    }
  }

  void CDescGate::GetTopics(std::unordered_map<std::string, Util::STopicInfo>& topic_info_map_)
  {
    std::unordered_map<std::string, Util::STopicInfo> map;

    std::shared_lock<std::shared_timed_mutex> lock(m_topic_info_map.sync);
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

    std::shared_lock<std::shared_timed_mutex> lock(m_topic_info_map.sync);
    m_topic_info_map.map->remove_deprecated();
    topic_names_.reserve(m_topic_info_map.map->size());

    for (const auto& topic_info : (*m_topic_info_map.map))
    {
      topic_names_.emplace_back(topic_info.first);
    }
  }

  bool CDescGate::GetTopicTypeName(const std::string& topic_name_, std::string& topic_type_)
  {
    if(topic_name_.empty()) return(false);

    std::shared_lock<std::shared_timed_mutex> lock(m_topic_info_map.sync);
    const auto topic_info_it = m_topic_info_map.map->find(topic_name_);

    if(topic_info_it == m_topic_info_map.map->end()) return(false);
    topic_type_ = (*topic_info_it).second.info.type_name;
    return(true);
  }

  bool CDescGate::GetTopicDescription(const std::string& topic_name_, std::string& topic_desc_)
  {
    if (topic_name_.empty()) return(false);

    std::shared_lock<std::shared_timed_mutex> lock(m_topic_info_map.sync);
    const auto topic_info_it = m_topic_info_map.map->find(topic_name_);

    if (topic_info_it == m_topic_info_map.map->end()) return(false);
    topic_desc_ = (*topic_info_it).second.info.type_description;
    return(true);
  }
  
  void CDescGate::ApplyServiceDescription(const std::string& service_name_
                                        , const std::string& method_name_
                                        , const std::string& req_type_name_
                                        , const std::string& req_type_desc_
                                        , const std::string& resp_type_name_
                                        , const std::string& resp_type_desc_
                                        , const QualityFlags  info_quality_)
  {
    std::tuple<std::string, std::string> service_method_tuple = std::make_tuple(service_name_, method_name_);

    std::unique_lock<std::shared_timed_mutex> lock(m_service_info_map.sync);
    m_service_info_map.map->remove_deprecated();

    auto service_info_map_it = m_service_info_map.map->find(service_method_tuple);
    if (service_info_map_it == m_service_info_map.map->end())
    {
      SServiceMethodInfoQuality& service_info = (*m_service_info_map.map)[service_method_tuple];
      service_info.info.request_type_name         = req_type_name_;
      service_info.info.request_type_description  = req_type_desc_;
      service_info.info.response_type_name        = resp_type_name_;
      service_info.info.response_type_description = resp_type_desc_;
      service_info.info_quality                   = info_quality_;
    }
    else
    {
      // do we need to check consistency ?
      SServiceMethodInfoQuality& service_info = (*m_service_info_map.map)[service_method_tuple];
      if (info_quality_ > service_info.info_quality)
      {
        service_info.info.request_type_name         = req_type_name_;
        service_info.info.request_type_description  = req_type_desc_;
        service_info.info.response_type_name        = resp_type_name_;
        service_info.info.response_type_description = resp_type_desc_;
        service_info.info_quality                   = info_quality_;
      }
    }
  }

  void CDescGate::GetServices(std::map<std::tuple<std::string, std::string>, Util::SServiceMethodInfo>& service_info_map_)
  {
    std::map<std::tuple<std::string, std::string>, Util::SServiceMethodInfo> map;

    std::shared_lock<std::shared_timed_mutex> lock(m_service_info_map.sync);
    m_service_info_map.map->remove_deprecated();

    for (const auto& service_info : (*m_service_info_map.map))
    {
      map.emplace(service_info.first, service_info.second.info);
    }
    service_info_map_.swap(map);
  }

  void CDescGate::GetServiceNames(std::vector<std::tuple<std::string, std::string>>& service_method_names_)
  {
    service_method_names_.clear();

    std::shared_lock<std::shared_timed_mutex> lock(m_service_info_map.sync);
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

    std::shared_lock<std::shared_timed_mutex> lock(m_service_info_map.sync);
    auto service_info_map_it = m_service_info_map.map->find(service_method_tuple);

    if (service_info_map_it == m_service_info_map.map->end()) return false;
    req_type_name_  = (*service_info_map_it).second.info.request_type_name;
    resp_type_name_ = (*service_info_map_it).second.info.response_type_name;
    return true;
  }

  bool CDescGate::GetServiceDescription(const std::string& service_name_, const std::string& method_name_, std::string& req_type_desc_, std::string& resp_type_desc_)
  {
    std::tuple<std::string, std::string> service_method_tuple = std::make_tuple(service_name_, method_name_);

    std::shared_lock<std::shared_timed_mutex> lock(m_service_info_map.sync);
    auto service_info_map_it = m_service_info_map.map->find(service_method_tuple);

    if (service_info_map_it == m_service_info_map.map->end()) return false;
    req_type_desc_  = (*service_info_map_it).second.info.request_type_description;
    resp_type_desc_ = (*service_info_map_it).second.info.response_type_description;
    return true;
  }
};
