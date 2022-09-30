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
#include "ecal_descgate.h"
#include <assert.h>
#include <algorithm>
#include <mutex>

namespace eCAL
{
  CDescGate::CDescGate()  = default;
  CDescGate::~CDescGate() = default;

  void CDescGate::Create()
  {
  }

  void CDescGate::Destroy()
  {
  }

  void CDescGate::ApplyTopicDescription(const std::string& topic_name_, const std::string& topic_type_, const std::string& topic_desc_, const QualityFlags description_quality_)
  {
    std::unique_lock<std::shared_timed_mutex> lock(m_topic_info_map_mutex);
    const auto topic_info_it = m_topic_info_map.find(topic_name_);
    
    // new element (no need to check anything, just add it)
    if(topic_info_it == m_topic_info_map.end())
    {
      // TODO: Maybe we should copy the description from another topic with the same type, if it is empty?
      STopicInfo topic_info;
      topic_info.type_name           = topic_type_;
      topic_info.type_description    = topic_desc_;
      topic_info.description_quality = description_quality_;
      m_topic_info_map.emplace(topic_name_, std::move(topic_info));
    }
    else
    {
      // existing type for the same topic name should be equal !!
      // we log the error only one time
      if(  !topic_info_it->second.type_missmatch_logged
            && !topic_type_.empty()
            && !topic_info_it->second.type_name.empty()
            && (topic_info_it->second.type_name != topic_type_)
        )
      {
        std::string ttype1 =  topic_info_it->second.type_name;
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

        topic_info_it->second.type_missmatch_logged = true;
      }

      // existing description for the same topic name should be equal !!
      // we log the warning only one time
      if ( !topic_info_it->second.type_missmatch_logged
            && !topic_desc_.empty()
            && !topic_info_it->second.type_description.empty()
            && (topic_info_it->second.type_description != topic_desc_)
        )
      {
        std::string msg = "eCAL Pub/Sub description mismatch for topic ";
        msg += topic_name_;
        eCAL::Logging::Log(log_level_warning, msg);

        topic_info_it->second.type_missmatch_logged = true;
      }

      // Now let's check whether the current information has a higher quality.
      // If it has a higher quality, we overwrite it.
      if (description_quality_ > topic_info_it->second.description_quality)
      {
        topic_info_it->second.type_name           = topic_type_;
        topic_info_it->second.type_description         = topic_desc_;
        topic_info_it->second.description_quality = description_quality_;
      }
    }
  }

  bool CDescGate::GetTopicTypeName(const std::string& topic_name_, std::string& topic_type_)
  {
    if(topic_name_.empty()) return(false);

    std::shared_lock<std::shared_timed_mutex> lock(m_topic_info_map_mutex);
    const auto topic_info_it = m_topic_info_map.find(topic_name_);

    if(topic_info_it == m_topic_info_map.end()) return(false);
    topic_type_ = topic_info_it->second.type_name;
    return(true);
  }

  bool CDescGate::GetTopicDescription(const std::string& topic_name_, std::string& topic_desc_)
  {
    if(topic_name_.empty()) return(false);

    std::shared_lock<std::shared_timed_mutex> lock(m_topic_info_map_mutex);
    const auto topic_info_it = m_topic_info_map.find(topic_name_);

    if(topic_info_it == m_topic_info_map.end()) return(false);
    topic_desc_ = topic_info_it->second.type_description;
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

    std::shared_lock<std::shared_timed_mutex> lock(m_service_info_map_mutex);
    auto service_info_map_it = m_service_info_map.find(service_method_tuple);
    if (service_info_map_it == m_service_info_map.end())
    {
      SServiceMethodInfo service_info;
      service_info.request_type_name         = req_type_name_;
      service_info.request_type_description  = req_type_desc_;
      service_info.response_type_name        = resp_type_name_;
      service_info.response_type_description = resp_type_desc_;
      service_info.info_quality              = info_quality_;

      m_service_info_map[service_method_tuple] = std::move(service_info);
    }
    else
    {
      // do we need to check consistency ?
      if (info_quality_ > service_info_map_it->second.info_quality)
      {
        service_info_map_it->second.request_type_name         = req_type_name_;
        service_info_map_it->second.request_type_description  = req_type_desc_;
        service_info_map_it->second.response_type_name        = resp_type_name_;
        service_info_map_it->second.response_type_description = resp_type_desc_;
        service_info_map_it->second.info_quality              = info_quality_;
      }
    }
  }

  bool CDescGate::GetServiceTypeNames(const std::string& service_name_, const std::string& method_name_, std::string& req_type_name_, std::string& resp_type_name_)
  {
    std::tuple<std::string, std::string> service_method_tuple = std::make_tuple(service_name_, method_name_);

    std::shared_lock<std::shared_timed_mutex> lock(m_service_info_map_mutex);
    auto service_info_map_it = m_service_info_map.find(service_method_tuple);

    if (service_info_map_it == m_service_info_map.end()) return false;

    req_type_name_  = service_info_map_it->second.request_type_name;
    resp_type_name_ = service_info_map_it->second.response_type_name;

    return true;
  }

  bool CDescGate::GetServiceDescription(const std::string& service_name_, const std::string& method_name_, std::string& req_type_desc_, std::string& resp_type_desc_)
  {
    std::tuple<std::string, std::string> service_method_tuple = std::make_tuple(service_name_, method_name_);

    std::shared_lock<std::shared_timed_mutex> lock(m_service_info_map_mutex);
    auto service_info_map_it = m_service_info_map.find(service_method_tuple);

    if (service_info_map_it == m_service_info_map.end()) return false;
    
    req_type_desc_  = service_info_map_it->second.request_type_description;
    resp_type_desc_ = service_info_map_it->second.response_type_description;

    return true;
  }
};
