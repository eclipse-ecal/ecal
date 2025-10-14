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
 * @brief  eCAL subscriber gateway class
**/

#pragma once

#include "pubsub/ecal_subscriber_impl.h"

#include <atomic>
#include <cstddef>
#include <memory>
#include <shared_mutex>
#include <string>
#include <unordered_map>

namespace eCAL
{
  class CSubGate
  {
  public:
    CSubGate();
    ~CSubGate();

    void Start();
    void Stop();

    bool Register(const std::string& topic_name_, const std::shared_ptr<CSubscriberImpl>& datareader_);
    bool Unregister(const std::string& topic_name_, const std::shared_ptr<CSubscriberImpl>& datareader_);

    bool HasSample(const std::string& sample_name_);

    bool ApplySample(const char* serialized_sample_data_, size_t serialized_sample_size_, eTLayerType layer_);
    bool ApplySample(const Payload::TopicInfo& topic_info_, const char* buf_, size_t len_, long long id_, long long clock_, long long time_, size_t hash_, eTLayerType layer_);

    void ApplyPublisherRegistration(const Registration::Sample& ecal_sample_);
    void ApplyPublisherUnregistration(const Registration::Sample& ecal_sample_);

    void GetRegistrations(Registration::SampleList& reg_sample_list_);

  protected:
    static std::atomic<bool> m_created;

    using TopicNameSubscriberMapT = std::unordered_multimap<std::string, std::weak_ptr<CSubscriberImpl>>;
    std::shared_timed_mutex  m_topic_name_subscriber_mutex;
    TopicNameSubscriberMapT  m_topic_name_subscriber_map;
  };
}
