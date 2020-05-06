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
#include <thread>

#include "ecal_timegate.h"

namespace eCAL
{
  namespace Time
  {
    std::string GetName()
    {
      if (!g_timegate() || !g_timegate()->IsValid()) return("");
      return(g_timegate()->GetName());
    }

    long long GetMicroSeconds()
    {
      if (!g_timegate() || !g_timegate()->IsValid())
      {
        std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
        return(std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count());
      }
      return(g_timegate()->GetMicroSeconds());
    }

    long long GetNanoSeconds()
    {
      if (!g_timegate() || !g_timegate()->IsValid())
      {
        std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
        return(std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count());
      }
      return(g_timegate()->GetNanoSeconds());
    }

    bool SetNanoSeconds(long long time_)
    {
      if (!g_timegate() || !g_timegate()->IsValid()) return(false);
      return(g_timegate()->SetNanoSeconds(time_));
    }

    bool IsSynchronized()
    {
      if (!g_timegate() || !g_timegate()->IsValid()) return(false);
      return(g_timegate()->IsSynchronized());
    }

    bool IsMaster()
    {
      if (!g_timegate() || !g_timegate()->IsValid()) return(false);
      return(g_timegate()->IsMaster());
    }
    
    void SleepForNanoseconds(long long duration_nsecs_)
    {
      if (!g_timegate() || !g_timegate()->IsValid())
      {
        std::this_thread::sleep_for(std::chrono::nanoseconds(duration_nsecs_));
      }
      else
      {
        g_timegate()->SleepForNanoseconds(duration_nsecs_);
      }
    }

    void GetStatus(int& error_, std::string* const status_message_) {
      if (!g_timegate()) {
        error_ = -1;
        if (status_message_) {
          status_message_->assign("Timegate has not been initialized!");
        }
      }
      else {
        g_timegate()->GetStatus(error_, status_message_);
      }
    }
  }
}
