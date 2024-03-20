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

#pragma once

#include <chrono>
#include <ecal/ecal_types.h>
#include <ecal/ecal_util.h>

#include "ecal_global_accessors.h"
#include "ecal_def.h"
#include "serialization/ecal_serialize_sample_registration.h"
#include "util/ecal_expmap.h"

#include <map>
#include <memory>
#include <shared_mutex>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace eCAL
{
  class CDescGate
  {
  public:
    CDescGate();
    ~CDescGate();

    void Create();
    void Destroy();

    void GetTopics(std::unordered_map<std::string, SDataTypeInformation>& topic_info_map_);
    void GetTopicNames(std::vector<std::string>& topic_names_);
    bool GetDataTypeInformation(const std::string& topic_name_, SDataTypeInformation& topic_info_);

    void GetServices(std::map<std::tuple<std::string, std::string>, SServiceMethodInformation>& service_info_map_);
    void GetServiceNames(std::vector<std::tuple<std::string, std::string>>& service_method_names_);
    bool GetServiceTypeNames(const std::string& service_name_, const std::string& method_name_, std::string& req_type_name_, std::string& resp_type_name_);
    bool GetServiceDescription(const std::string& service_name_, const std::string& method_name_, std::string& req_type_desc_, std::string& resp_type_desc_);

  protected:
    bool ApplySample(const Registration::Sample& sample_, eTLayerType layer_);
      
    bool ApplyTopicDescription(const std::string& topic_name_,
                               const SDataTypeInformation& topic_info_);

    bool ApplyServiceDescription(const std::string& service_name_, 
                                 const std::string& method_name_, 
                                 const SDataTypeInformation& request_type_information_,
                                 const SDataTypeInformation& response_type_information_);

    // key: topic name | value: topic (type/desc), quality
    using TopicInfoMap = eCAL::Util::CExpMap<std::string, SDataTypeInformation>;      //!< Map containing { TopicName -> (Type, Description) } mapping of all topics that are currently known
    struct STopicInfoMap
    {
      explicit STopicInfoMap(const std::chrono::milliseconds& timeout_) :
        map(std::make_unique<TopicInfoMap>(timeout_))
      {
      };
      mutable std::mutex              sync;                                        //!< Mutex protecting the map
      std::unique_ptr<TopicInfoMap>   map;                                         //!< Map containing information about each known topic
    };
    STopicInfoMap m_topic_info_map;

    // key: tup<service name, method name> | value: request (type/desc), response (type/desc), quality
    using ServiceMethodInfoMap = eCAL::Util::CExpMap<std::tuple<std::string, std::string>, SServiceMethodInformation>; //!< Map { (ServiceName, MethodName) -> ( (ReqType, ReqDescription), (RespType, RespDescription) ) } mapping of all currently known services
    struct SServiceMethodInfoMap
    {
      explicit SServiceMethodInfoMap(const std::chrono::milliseconds& timeout_) :
        map(std::make_unique<ServiceMethodInfoMap>(timeout_))
      {
      };
      mutable std::mutex                    sync;                                  //!< Mutex protecting the map
      std::unique_ptr<ServiceMethodInfoMap> map;                                   //!< Map containing information about each known service
    };
    SServiceMethodInfoMap m_service_info_map;
  };
}
