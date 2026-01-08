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

#include <atomic>
#include <cstddef>
#include <memory>
#include <shared_mutex>
#include <string>
#include <unordered_map>

#include "serialization/ecal_struct_sample_registration.h"

namespace eCAL
{
  class CSubscriberImpl;

  /*
  * Manages only subscribers and their registrations, but not dataflow.
  */
  class CSubGate
  {
  public:
    CSubGate();
    ~CSubGate();                             

    CSubGate(const CSubGate&) = delete;               
    CSubGate& operator=(const CSubGate&) = delete;
    CSubGate(CSubGate&&) noexcept = delete;
    CSubGate& operator=(CSubGate&&) noexcept = delete;

    void Start();
    void Stop();

    bool Register(const std::string& topic_name_, const std::shared_ptr<CSubscriberImpl>& datareader_);
    bool Unregister(const std::string& topic_name_, const std::shared_ptr<CSubscriberImpl>& datareader_);

    void GetRegistrations(Registration::SampleList& reg_sample_list_);

  protected:
    static std::atomic<bool> m_created;

    using TopicNameSubscriberMapT = std::unordered_multimap<std::string, std::shared_ptr<CSubscriberImpl>>;
    std::shared_timed_mutex  m_topic_name_subscriber_mutex;
    TopicNameSubscriberMapT  m_topic_name_subscriber_map;
  };
}
