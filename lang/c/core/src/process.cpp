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
 * @file   process.cpp
 * @brief  eCAL process c interface
**/

#include <ecal/ecal.h>
#include <ecal_c/process.h>

#include <map>

#include "common.h"

extern "C"
{
  ECALC_API void eCAL_Process_DumpConfig()
  {
    eCAL::Process::DumpConfig();
  }

  ECALC_API void eCAL_Process_DumpConfig2(char** configuration_string_)
  {
    if (configuration_string_ == NULL) return;
    if (*configuration_string_ != NULL) return;
    std::string buffer;
    
    eCAL::Process::DumpConfig(buffer);
    const auto size = buffer.size() + 1;
    *configuration_string_ = reinterpret_cast<char*>(std::malloc(size));
    if (*configuration_string_ != NULL)
    {
      std::strncpy(*configuration_string_, buffer.c_str(), size);
    }
  }

  ECALC_API const char* eCAL_Process_GetHostName()
  {
    return eCAL::Process::GetHostName().c_str();
  }

  ECALC_API const char* eCAL_Process_GetShmTransportDomain()
  {
    return eCAL::Process::GetShmTransportDomain().c_str();
  }

  ECALC_API const char* eCAL_Process_GetUnitName()
  {
    return eCAL::Process::GetUnitName().c_str();
  }

  ECALC_API void eCAL_Process_SleepMS(long time_ms_)
  {
    eCAL::Process::SleepMS(time_ms_);
  }

  ECALC_API void eCAL_Process_SleepNS(long long time_ns_)
  {
    eCAL::Process::SleepNS(time_ns_);
  }

  ECALC_API int eCAL_Process_GetProcessID()
  {
    return eCAL::Process::GetProcessID();
  }

  ECALC_API const char* eCAL_Process_GetProcessIDAsString()
  {
    return eCAL::Process::GetProcessIDAsString().c_str();
  }

  ECALC_API const char* eCAL_Process_GetProcessName()
  {
    return eCAL::Process::GetProcessName().c_str();
  }

  ECALC_API const char* eCAL_Process_GetProcessParameter()
  {
    return eCAL::Process::GetProcessParameter().c_str();
  }

  ECALC_API void eCAL_Process_SetState(enum eCAL_Process_eSeverity severity_, enum eCAL_Process_eSeverityLevel level_, const char* info_)
  {
    static const std::map<enum eCAL_Process_eSeverity, eCAL::Process::eSeverity> severity_map
    {
      {eCAL_Process_eSeverity_unknown, eCAL::Process::eSeverity::unknown},
      {eCAL_Process_eSeverity_healthy, eCAL::Process::eSeverity::healthy},
      {eCAL_Process_eSeverity_warning, eCAL::Process::eSeverity::warning},
      {eCAL_Process_eSeverity_critical, eCAL::Process::eSeverity::critical},
      {eCAL_Process_eSeverity_failed, eCAL::Process::eSeverity::failed},
    };

    static const std::map<enum eCAL_Process_eSeverityLevel, eCAL::Process::eSeverityLevel> severity_level_map
    {
      {eCAL_Process_eSeverityLevel_level1, eCAL::Process::eSeverityLevel::level1},
      {eCAL_Process_eSeverityLevel_level2, eCAL::Process::eSeverityLevel::level2},
      {eCAL_Process_eSeverityLevel_level3, eCAL::Process::eSeverityLevel::level3},
      {eCAL_Process_eSeverityLevel_level4, eCAL::Process::eSeverityLevel::level4},
      {eCAL_Process_eSeverityLevel_level5, eCAL::Process::eSeverityLevel::level5},
    };

    eCAL::Process::SetState(severity_map.at(severity_), severity_level_map.at(level_), info_);
  }
}
