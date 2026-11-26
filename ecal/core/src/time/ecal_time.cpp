/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
 * Copyright 2025 AUMOVIO and subsidiaries. All rights reserved.
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
 * @brief  eCAL time interface
**/

#include <ecal/ecal.h>

#include <chrono>
#include <string>

#if ECAL_CORE_TIMEPLUGIN
#include "ecal_timegate.h"
#include "ecal_global_accessors.h"
#endif
#include "ecal_time_localtime.h"

namespace eCAL
{
  namespace Time
  {
    const std::string& GetName()
    {
#if ECAL_CORE_TIMEPLUGIN
      auto timegate = g_timegate();
      if (timegate)
      {
        return timegate->GetName();
      }
      else
#endif
      {
        return CLocalTimeProvider::GetName();
      }
    }

    long long GetMicroSeconds()
    {
#if ECAL_CORE_TIMEPLUGIN
      auto timegate = g_timegate();
      if (timegate)
      {
        return timegate->GetMicroSeconds();
      }
      else
#endif
      {
        return CLocalTimeProvider::GetMicroseconds();
      }
    }

    long long GetNanoSeconds()
    {
#if ECAL_CORE_TIMEPLUGIN
      auto timegate = g_timegate();
      if (timegate)
      {
        return timegate->GetNanoSeconds();
      }
      else
#endif
      {
        return CLocalTimeProvider::GetNanoSeconds();
      }
    }

    bool SetNanoSeconds(long long time_)
    {
#if ECAL_CORE_TIMEPLUGIN
      auto timegate = g_timegate();
      if (timegate)
      {
        return timegate->SetNanoSeconds(time_);
      }
      else
#endif
      {
        return CLocalTimeProvider::SetNanoSeconds(time_);
      }
    }

    bool IsSynchronized()
    {
#if ECAL_CORE_TIMEPLUGIN
      auto timegate = g_timegate();
      if (timegate)
      {
        return timegate->IsSynchronized();
      }
      else
#endif
      {
        return CLocalTimeProvider::IsSynchronized();
      }
    }

    bool IsMaster()
    {
#if ECAL_CORE_TIMEPLUGIN
      auto timegate = g_timegate();
      if (timegate)
      {
        return timegate->IsMaster();
      }
      else
#endif
      {
        return CLocalTimeProvider::IsMaster();
      }
    }
    
    void SleepForNanoseconds(long long duration_nsecs_)
    {
#if ECAL_CORE_TIMEPLUGIN
      auto timegate = g_timegate();
      if (timegate)
      {
        timegate->SleepForNanoseconds(duration_nsecs_);
      }
      else
#endif
      {
        CLocalTimeProvider::SleepForNanoseconds(duration_nsecs_);
      }
    }

    void GetStatus(int& error_, std::string* const status_message_)
    {
#if ECAL_CORE_TIMEPLUGIN
      auto timegate = g_timegate();
      if (timegate)
      {
        timegate->GetStatus(error_, status_message_);
      }
      else
#endif
      {
        CLocalTimeProvider::GetStatus(error_, status_message_);
      }
    }
  }
}
