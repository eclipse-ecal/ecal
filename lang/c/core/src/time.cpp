/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2019 Continental Corporation
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
 * @file   time.cpp
 * @brief  eCAL time c interface
**/

#include <ecal/ecal.h>
#include <ecal_c/types.h>
#include <ecal_c/time.h>

#include "common.h"

extern "C"
{
  ECALC_API char* eCAL_Time_GetName()
  {
    return Clone_CString(eCAL::Time::GetName().c_str());
  }

  ECALC_API long long eCAL_Time_GetMicroSeconds()
  {
    return(eCAL::Time::GetMicroSeconds());
  }

  ECALC_API long long eCAL_Time_GetNanoSeconds()
  {
    return(eCAL::Time::GetNanoSeconds());
  }

  ECALC_API int eCAL_Time_SetNanoSeconds(long long time_)
  {
    return static_cast<int>(eCAL::Time::SetNanoSeconds(time_));
  }

  ECALC_API int eCAL_Time_IsTimeSynchronized()
  {
    return static_cast<int>(eCAL::Time::IsSynchronized());
  }

  ECALC_API int eCAL_Time_IsTimeMaster()
  {
    return static_cast<int>(eCAL::Time::IsMaster());
  }

  ECALC_API void eCAL_Time_SleepForNanoseconds(long long duration_nsecs_)
  {
    eCAL::Time::SleepForNanoseconds(duration_nsecs_);
  }

  ECALC_API void eCAL_Time_GetStatus(int* error_, char** status_message_)
  {
    if (error_ == NULL) return;
    if (*error_ != 0) return;

    std::unique_ptr<std::string> status_message;
    if (status_message_ != NULL)
    {
      if (*status_message_ != NULL) return;
      status_message = std::make_unique<std::string>();
    }

    eCAL::Time::GetStatus(*error_, status_message.get());
    if (status_message_ != NULL)
    {
      *status_message_ = reinterpret_cast<char*>(std::malloc(status_message->size() + 1));
      if (*status_message_ != NULL)
      {
        std::strncpy(*status_message_, status_message->c_str(), status_message->size() + 1);
      }
    }
  }
}