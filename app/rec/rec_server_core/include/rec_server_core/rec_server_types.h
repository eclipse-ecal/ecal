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

#pragma once

#include <string>
#include <map>
#include <functional>
#include <set>
#include <chrono>
#include <memory>

#include <ecal/time.h>

#include <rec_client_core/topic_info.h>
#include <rec_client_core/job_config.h>
#include <rec_client_core/state.h>

namespace eCAL
{
  namespace rec_server
  {
    using TopicName_T = std::string;
    using Hostname_T = std::string;
    using UnitName_T = std::string;
    using DataFrequency_T = double;
    using ProcessID_T = int32_t;
    using IsEcalRecClientRunning_T = bool;

    struct TopicInfo
    {
      TopicInfo()
      {
      }

      void AddTypeInfo(const eCAL::SDataTypeInformation& data_type_info)
      {
        type_.insert(data_type_info);
      }

      std::set<SDataTypeInformation> type_;
      std::map<Hostname_T, std::set<UnitName_T>> publishers_;
      std::map<std::pair<Hostname_T, ProcessID_T>, DataFrequency_T> rec_subscribers_;
    };

    using TopicInfoMap_T = std::map<TopicName_T, eCAL::rec_server::TopicInfo>;
    using HostsRunningEcalRec_T = std::map<Hostname_T, IsEcalRecClientRunning_T>;
    using PostUpdateCallback_T = std::function<void(const TopicInfoMap_T&, const HostsRunningEcalRec_T&)>;
    using RecorderStatusMap_T = std::map<std::string, std::pair<eCAL::rec::RecorderStatus, eCAL::Time::ecal_clock::time_point>>;
  }
}
