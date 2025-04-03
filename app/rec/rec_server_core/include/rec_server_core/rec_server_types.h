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
    struct TopicInfo
    {
      TopicInfo(const std::string& type)
        : type_(type)
      {}

      std::string                                        type_;                 ///< Type of the topic (e.g. the protobuf-type)
      std::map<std::string, std::set<std::string>>       publishers_;           ///< {hostname: [publisher_names]}
      std::map<std::pair<std::string, int32_t>, double>  rec_subscribers_;      ///< {(hostname, process_id): data_frequency}
    };

    typedef std::map<std::string, eCAL::rec_server::TopicInfo>                       TopicInfoMap_T;
    typedef std::map<std::string, bool>                                              HostsRunningEcalRec_T;
    typedef std::function<void(const TopicInfoMap_T&, const HostsRunningEcalRec_T&)> PostUpdateCallback_T;

    typedef std::map<std::string, std::pair<eCAL::rec::RecorderStatus, eCAL::Time::ecal_clock::time_point>> RecorderStatusMap_T;
  }
}
