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
 * @file   log.cpp
 * @brief  eCAL logging c interface
**/

#include <ecal/ecal.h>
#include <ecal_c/log.h>

#include "common.h"

#include <map>

extern "C"
{
  ECALC_API void eCAL_Logging_Log(enum eCAL_Logging_eLogLevel level_, const char* message_)
  {
    static const std::map<eCAL_Logging_eLogLevel, eCAL::Logging::eLogLevel> log_level_map
    {
        {eCAL_Logging_eLogLevel_none, eCAL::Logging::log_level_none},
        {eCAL_Logging_eLogLevel_all, eCAL::Logging::log_level_all},
        {eCAL_Logging_eLogLevel_info, eCAL::Logging::log_level_info},
        {eCAL_Logging_eLogLevel_warning, eCAL::Logging::log_level_warning},
        {eCAL_Logging_eLogLevel_error, eCAL::Logging::log_level_error},
        {eCAL_Logging_eLogLevel_fatal, eCAL::Logging::log_level_fatal},
        {eCAL_Logging_eLogLevel_debug1, eCAL::Logging::log_level_debug1},
        {eCAL_Logging_eLogLevel_debug2, eCAL::Logging::log_level_debug2},
        {eCAL_Logging_eLogLevel_debug3, eCAL::Logging::log_level_debug3},
        {eCAL_Logging_eLogLevel_debug4, eCAL::Logging::log_level_debug4}
    };
    eCAL::Logging::Log(log_level_map.at(level_), message_);
  }

  ECALC_API int eCAL_Logging_GetLogging(void** log_, size_t* log_length_)
  {
    if (log_ == NULL || log_length_ == NULL) return 1;
    if (*log_ != NULL || *log_length_ != 0) return 1;

    std::string buffer;
    if (eCAL::Logging::GetLogging(buffer))
    {
      *log_ = std::malloc(buffer.size());
      if (*log_ != NULL)
      {
        std::memcpy(*log_, buffer.data(), buffer.size());
        *log_length_ = buffer.size();
        
      }
    }
    return !static_cast<int>(*log_ != NULL);
  }
}
