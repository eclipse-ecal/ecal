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

#include <ecal/ecal_types.h>
#include <ecal/ecal_util.h>

#include "serialization/ecal_serialize_sample_registration.h"
#include "util/ecal_expmap.h"

#include <map>
#include <mutex>
#include <string>
#include <type_traits>

namespace eCAL
{
  class CDescGate
  {
  public:
    CDescGate();
    ~CDescGate();

    void Create();
    void Destroy();

    struct STopicNameId
    {
      std::string topic_name;
      std::string topic_id;

      bool operator<(const STopicNameId& other) const
      {
        if (topic_name != other.topic_name)
          return topic_name < other.topic_name;
        return topic_id < other.topic_id;
      }
    };

    using TopicNameIdMap = std::map<STopicNameId, Util::SQualityDataTypeInformation>;
    TopicNameIdMap GetPublisher();
    TopicNameIdMap GetSubscriber();

    struct SServiceMethodNameId
    {
      std::string service_name;
      std::string service_id;
      std::string method_name;

      bool operator<(const SServiceMethodNameId& other) const
      {
        if (service_name != other.service_name)
          return service_name < other.service_name;
        if (service_id != other.service_id)
          return service_id < other.service_id;
        return method_name < other.method_name;
      }
    };

    using ServiceMethodNameIdMap = std::map<SServiceMethodNameId, Util::SQualityServiceMethodInformation>;
    ServiceMethodNameIdMap GetServices();
    ServiceMethodNameIdMap GetClients();

  protected:
    using TopicInfoMap = eCAL::Util::CExpMap<STopicNameId, Util::SQualityDataTypeInformation>;
    struct STopicInfoMap
    {
      explicit STopicInfoMap(const std::chrono::milliseconds& timeout_) :
        map(std::make_unique<TopicInfoMap>(timeout_))
      {
      };
      mutable std::mutex            sync;
      std::unique_ptr<TopicInfoMap> map;
    };
    STopicInfoMap m_publisher_info_map;
    STopicInfoMap m_subscriber_info_map;

    TopicNameIdMap GetTopics(const STopicInfoMap& topic_map_);

    using ServiceMethodInfoMap = eCAL::Util::CExpMap<SServiceMethodNameId, Util::SQualityServiceMethodInformation>;
    struct SServiceMethodInfoMap
    {
      explicit SServiceMethodInfoMap(const std::chrono::milliseconds& timeout_) :
        map(std::make_unique<ServiceMethodInfoMap>(timeout_))
      {
      };
      mutable std::mutex                    sync;
      std::unique_ptr<ServiceMethodInfoMap> map;
    };
    SServiceMethodInfoMap m_service_info_map;
    SServiceMethodInfoMap m_client_info_map;

    ServiceMethodNameIdMap GetServices(const SServiceMethodInfoMap& service_method_info_map_);

    void ApplySample(const Registration::Sample& sample_, eTLayerType layer_);
      
    void ApplyTopicDescription(STopicInfoMap& topic_info_map_,
                               const std::string& topic_name_,
                               const std::string& topic_id_,
                               const SDataTypeInformation& topic_info_,
                               Util::DescQualityFlags topic_quality_);

    void RemTopicDescription(STopicInfoMap& topic_info_map_,
                             const std::string& topic_name_,
                             const std::string& topic_id_);

    void ApplyServiceDescription(SServiceMethodInfoMap& service_method_info_map_,
                                 const std::string& service_name_,
                                 const std::string& service_id_,
                                 const std::string& method_name_,
                                 const SDataTypeInformation& request_type_information_,
                                 const SDataTypeInformation& response_type_information_,
                                 Util::DescQualityFlags service_quality_);

    void RemServiceDescription(SServiceMethodInfoMap& service_method_info_map_,
                               const std::string& service_name_,
                               const std::string& service_id_);
  };
}
