/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2024 Continental Corporation
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

#include <ecal/ecal_registration.h>
#include <ecal/ecal_types.h>
#include <ecal/ecal_util.h>

#include "serialization/ecal_struct_sample_registration.h"

#include <atomic>
#include <chrono>
#include <map>
#include <mutex>
#include <string>
#include <tuple>

namespace eCAL
{
  class CDescGate
  {
  public:
    CDescGate();
    ~CDescGate();

    // apply samples to description gate
    void ApplySample(const Registration::Sample& sample_, eTLayerType layer_);

    // get publisher information
    std::set<Registration::STopicId> GetPublisherIDs() const;
    bool GetPublisherInfo(const Registration::STopicId& id_, Registration::SQualityTopicInfo& topic_info_) const;
    Registration::CallbackToken AddPublisherEventCallback(const Registration::TopicIDCallbackT& callback_);
    void RemPublisherEventCallback(Registration::CallbackToken token_);

    // get subscriber information
    std::set<Registration::STopicId> GetSubscriberIDs() const;
    bool GetSubscriberInfo(const Registration::STopicId& id_, Registration::SQualityTopicInfo& topic_info_) const;
    Registration::CallbackToken AddSubscriberEventCallback(const Registration::TopicIDCallbackT& callback_);
    void RemSubscriberEventCallback(Registration::CallbackToken token_);

    // get service information
    std::set<Registration::SServiceId> GetServiceIDs() const;
    bool GetServiceInfo(const Registration::SServiceId& id_, Registration::SQualityServiceInfo& service_info_) const;

    // get client information
    std::set<Registration::SServiceId> GetClientIDs() const;
    bool GetClientInfo(const Registration::SServiceId& id_, Registration::SQualityServiceInfo& service_info_) const;

    // delete copy constructor and copy assignment operator
    CDescGate(const CDescGate&) = delete;
    CDescGate& operator=(const CDescGate&) = delete;

    // delete move constructor and move assignment operator
    CDescGate(CDescGate&&) = delete;
    CDescGate& operator=(CDescGate&&) = delete;

  protected:
    using QualityTopicIdMap  = std::map<Registration::STopicId, Registration::SQualityTopicInfo>;
    struct SQualityTopicIdMap
    {
      mutable std::mutex mtx;
      QualityTopicIdMap  map;
    };

    using TopicIdCallbackMap = std::map<Registration::CallbackToken, Registration::TopicIDCallbackT>;
    struct STopicIdCallbackMap
    {
      mutable std::mutex mtx;
      TopicIdCallbackMap map;
    };

    using QualityServiceIdMap = std::map<Registration::SServiceId, Registration::SQualityServiceInfo>;
    struct SQualityServiceIdMap
    {
      mutable std::mutex  mtx;
      QualityServiceIdMap id_map;
    };

    static std::set<Registration::STopicId>   GetTopicIDs(const SQualityTopicIdMap& topic_info_map_);
    static bool                               GetTopic   (const Registration::STopicId& id_, const SQualityTopicIdMap& topic_info_map_, Registration::SQualityTopicInfo& topic_info_);

    static std::set<Registration::SServiceId> GetServiceIDs(const SQualityServiceIdMap& service_method_info_map_);
    static bool                               GetService   (const Registration::SServiceId& id_, const SQualityServiceIdMap& service_method_info_map_, Registration::SQualityServiceInfo& service_method_info_);

    static void ApplyTopicDescription(SQualityTopicIdMap& topic_info_map_,
                                      const STopicIdCallbackMap& topic_callback_map_, 
                                      const Registration::SampleIdentifier& topic_id_,
                                      const std::string& topic_name_,
                                      const SDataTypeInformation& topic_info_,
                                      Registration::DescQualityFlags topic_quality_);

    static void RemTopicDescription(SQualityTopicIdMap& topic_info_map_,
                                    const STopicIdCallbackMap& topic_callback_map_,
                                    const Registration::SampleIdentifier& topic_id_,
                                    const std::string& topic_name_);

    static void ApplyServiceDescription(SQualityServiceIdMap& service_method_info_map_,
                                        const Registration::SampleIdentifier& service_id_,
                                        const std::string& service_name_,
                                        const std::string& method_name_,
                                        const SDataTypeInformation& request_type_information_,
                                        const SDataTypeInformation& response_type_information_,
                                        Registration::DescQualityFlags request_type_quality_,
                                        Registration::DescQualityFlags response_type_quality_);

    static void RemServiceDescription(SQualityServiceIdMap& service_method_info_map_,
                                      const Registration::SampleIdentifier& service_id_,
                                      const std::string& service_name_);

    Registration::CallbackToken CreateToken();
      
    // internal quality topic info publisher/subscriber maps
    SQualityTopicIdMap   m_publisher_info_map;
    STopicIdCallbackMap  m_publisher_callback_map;

    SQualityTopicIdMap   m_subscriber_info_map;
    STopicIdCallbackMap  m_subscriber_callback_map;

    // internal quality service info service/client maps
    SQualityServiceIdMap m_service_info_map;
    SQualityServiceIdMap m_client_info_map;

    mutable std::mutex                       m_callback_token_mtx;
    std::atomic<Registration::CallbackToken> m_callback_token{ 0 };
  };
}
