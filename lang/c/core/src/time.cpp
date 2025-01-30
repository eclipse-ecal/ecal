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
  ECALC_API int eCAL_Time_GetName(void* name_, int name_len_)
  {
    const std::string name = eCAL::Time::GetName();
    if (!name.empty())
    {
      return(CopyBuffer(name_, name_len_, name));
    }
    return(0);
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

  ECALC_API int eCAL_Time_GetStatus(int* error_, char** status_message_, const int max_len_)
  {
    if (max_len_ == ECAL_ALLOCATE_4ME || max_len_ > 0)
    {
      std::string status_message;
      eCAL::Time::GetStatus(*error_, &status_message);

      if (!status_message.empty())
      {
        return CopyBuffer(status_message_, max_len_, status_message); // NOLINT(*-multi-level-implicit-pointer-conversion)
      }
      return 0;
    }
    else
    {
      eCAL::Time::GetStatus(*error_, nullptr);
      return 0;
    }
  }
}