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
#include <cstdint>

struct Process
{
  enum class Severity
  {
    UNKNOWN, HEALTHY, WARNING, CRITICAL, FAILED
  };

  enum class SeverityLevel
  {
    UNKNOWN, LEVEL_1, LEVEL_2, LEVEL_3, LEVEL_4, LEVEL_5
  };

  enum class TimeSyncState
  {
    NONE, REALTIME, REPLAY
  };

  int32_t process_id;
  std::string name;
  std::string host_name;
  std::string unit_name;
  std::string params;
  Severity severity;
  SeverityLevel severity_level;
  std::string state_info;
  TimeSyncState time_sync_state;
  std::string time_sync_mod_name;
  std::string component_init_info;
  std::string ecal_runtime_version;
};
