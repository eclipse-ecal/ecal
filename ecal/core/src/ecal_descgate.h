/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
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

#include <ecal/registration.h>
#include <ecal/pubsub/types.h>
#include <ecal/service/types.h>
#include <ecal/util.h>

#include "serialization/ecal_struct_sample_registration.h"

#include <atomic>
#include <chrono>
#include <unordered_map>
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
    std::set<STopicId> GetPublisherIDs() const;
    bool GetPublisherInfo(const STopicId& id_, SDataTypeInformation& topic_info_) const;
    Registration::CallbackToken AddPublisherEventCallback(const Registration::TopicEventCallbackT& callback_);
    void RemPublisherEventCallback(Registration::CallbackToken token_);

    // get subscriber information
    std::set<STopicId> GetSubscriberIDs() const;
    bool GetSubscriberInfo(const STopicId& id_, SDataTypeInformation& topic_info_) const;
    Registration::CallbackToken AddSubscriberEventCallback(const Registration::TopicEventCallbackT& callback_);
    void RemSubscriberEventCallback(Registration::CallbackToken token_);

    // get service information
    std::set<SServiceId> GetServerIDs() const;
    bool GetServerInfo(const SServiceId& id_, ServiceMethodInformationSetT& service_info_) const;

    // get client information
    std::set<SServiceId> GetClientIDs() const;
    bool GetClientInfo(const SServiceId& id_, ServiceMethodInformationSetT& service_info_) const;

    // delete copy constructor and copy assignment operator
    CDescGate(const CDescGate&) = delete;
    CDescGate& operator=(const CDescGate&) = delete;

    // delete move constructor and move assignment operator
    CDescGate(CDescGate&&) = delete;
    CDescGate& operator=(CDescGate&&) = delete;

  private:
    class CollectedTopicInfo
    {
      struct TopicInfo
      {
        STopicId id;
        SDataTypeInformation datatype_info;
      };

      mutable std::mutex mutex;
      std::unordered_map<EntityIdT, TopicInfo> map;

    public:
      void RegisterSample(
        const Registration::Sample& sample_,
        const std::function<void(const STopicId&)>& on_new_topic);

      void UnregisterSample(
        const Registration::Sample& sample_,
        const std::function<void(const STopicId&)>& on_erased_topic);

      std::set<STopicId> GetIDs() const;

      bool GetInfo(const STopicId& id_, SDataTypeInformation& topic_info_) const;
    };

    class CollectedServiceInfo
    {
      struct ServiceInfo
      {
        SServiceId id;
        ServiceMethodInformationSetT service_method_information;
      };

      mutable std::mutex mutex;
      std::unordered_map<EntityIdT, ServiceInfo> map;

    public:
      template <typename Service>
      void RegisterSample(
        const Registration::SampleIdentifier& sample_id_, const Service& sample_service_)
      {
        std::lock_guard<std::mutex> guard(mutex);

        auto iterator = map.find(sample_id_.entity_id);

        if (iterator != map.end())
        {
          // See if we need to update service method information.
          // In theory, this should be an invariant, at least with v6.
          iterator->second.service_method_information = Convert(sample_service_);
        }
        else
        {
          auto service_id = eCAL::SServiceId{ ConvertToEntityId(sample_id_), sample_service_.service_name };
          auto service_method_information = Convert(sample_service_);
          auto service_info = ServiceInfo{ service_id, std::move(service_method_information) };

          map.emplace(sample_id_.entity_id, service_info);
        }
      }

      void UnregisterSample(
        const Registration::Sample& sample_c);

      std::set<SServiceId> GetIDs() const;

      bool GetInfo(const SServiceId& id_, ServiceMethodInformationSetT& topic_info_) const;
    };

    using TopicEventCallbackMap = std::map<Registration::CallbackToken, Registration::TopicEventCallbackT>;
    struct STopicEventCallbackMap
    {
      mutable std::mutex mtx;
      TopicEventCallbackMap map;
    };

    Registration::CallbackToken CreateToken();
      
    // internal quality topic info publisher/subscriber maps
    CollectedTopicInfo                       m_publisher_infos;
    STopicEventCallbackMap                   m_publisher_callback_map;

    CollectedTopicInfo                       m_subscriber_infos;
    STopicEventCallbackMap                   m_subscriber_callback_map;

    // internal quality service info service/client maps
    CollectedServiceInfo                     m_server_infos;
    CollectedServiceInfo                     m_client_infos;

    std::atomic<Registration::CallbackToken> m_callback_token{ 0 };
  };
}
