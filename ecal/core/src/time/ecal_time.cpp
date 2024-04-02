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
 * @brief  eCAL time interface
**/

#include <ecal/ecal.h>

#include <chrono>
#include <string>

#if ECAL_CORE_TIMEGATE
#include "ecal_timegate.h"
#endif

namespace eCAL
{
  namespace Time
  {
    std::string GetName()
    {
#if ECAL_CORE_TIMEGATE
      if ((g_timegate() != nullptr) && g_timegate()->IsValid())
      {
        return(g_timegate()->GetName());
      }
#endif
      return "";
    }

    long long GetMicroSeconds()
    {
#if ECAL_CORE_TIMEGATE
      if ((g_timegate() != nullptr) && g_timegate()->IsValid())
      {
        return(g_timegate()->GetMicroSeconds());
      }
#endif
      const std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
      return(std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count());
    }

    long long GetNanoSeconds()
    {
#if ECAL_CORE_TIMEGATE
      if ((g_timegate() != nullptr) && g_timegate()->IsValid())
      {
        return(g_timegate()->GetNanoSeconds());
      }
#endif
      const std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
      return(std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count());
    }

    bool SetNanoSeconds(long long time_)
    {
#if ECAL_CORE_TIMEGATE
      if ((g_timegate() != nullptr) && g_timegate()->IsValid())
      {
        return(g_timegate()->SetNanoSeconds(time_));
      }
#endif
      (void)time_;
      return(false);
    }

    bool IsSynchronized()
    {
#if ECAL_CORE_TIMEGATE
      if ((g_timegate() != nullptr) && g_timegate()->IsValid())
      {
        return(g_timegate()->IsSynchronized());
      }
#endif
      return(false);
    }

    bool IsMaster()
    {
#if ECAL_CORE_TIMEGATE
      if ((g_timegate() != nullptr) && g_timegate()->IsValid())
      {
        return(g_timegate()->IsMaster());
      }
#endif
      return(false);
    }
    
    void SleepForNanoseconds(long long duration_nsecs_)
    {
#if ECAL_CORE_TIMEGATE
      if ((g_timegate() != nullptr) && g_timegate()->IsValid())
      {
        g_timegate()->SleepForNanoseconds(duration_nsecs_);
      }
#endif
      eCAL::Process::SleepFor(std::chrono::nanoseconds(duration_nsecs_));
    }

    void GetStatus(int& error_, std::string* const status_message_)
    {
#if ECAL_CORE_TIMEGATE
      if (g_timegate() == nullptr)
      {
        error_ = -1;
        if (status_message_ != nullptr)
        {
          status_message_->assign("Timegate has not been initialized!");
        }
      }
      else 
      {
        g_timegate()->GetStatus(error_, status_message_);
      }
#else
      error_ = -1;
      if (status_message_ != nullptr)
      {
        status_message_->assign("Timegate functionality not available.");
      }
#endif
    }
  }
}
