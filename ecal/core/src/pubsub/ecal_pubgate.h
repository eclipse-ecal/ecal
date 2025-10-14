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
 * @brief  eCAL publisher gateway class
**/

#pragma once

#include "ecal_publisher_impl.h"
#include "serialization/ecal_struct_sample_registration.h"

#include <atomic>
#include <map>
#include <memory>
#include <shared_mutex>
#include <string>
#include <unordered_map>

namespace eCAL
{
  class CPubGate
  {
  public:
    CPubGate();
    ~CPubGate();

    void Start();
    void Stop();

    bool Register(const std::string& topic_name_, const std::shared_ptr<CPublisherImpl>& publisher_);
    bool Unregister(const std::string& topic_name_, const std::shared_ptr<CPublisherImpl>& publisher_);

    void ApplySubscriberRegistration(const Registration::Sample& ecal_sample_);
    void ApplySubscriberUnregistration(const Registration::Sample& ecal_sample_);

    void GetRegistrations(Registration::SampleList& reg_sample_list_);

  protected:
    static std::atomic<bool>  m_created;

    using TopicNamePublisherMapT = std::multimap<std::string, std::weak_ptr<CPublisherImpl>>;
    std::shared_timed_mutex  m_topic_name_publisher_mutex;
    TopicNamePublisherMapT   m_topic_name_publisher_map;
  };
}
