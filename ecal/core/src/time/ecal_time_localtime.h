/* ========================= eCAL LICENSE =================================
 *
 * Copyright 2026 AUMOVIO and subsidiaries. All rights reserved.
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

#include "time/ecal_time_base.h"

#include <chrono>

#include <ecal/process.h>

namespace eCAL
{
  class CLocalTimeProvider
  {
  public:
    static const std::string& GetName()
    {
      static const std::string name{ "ecaltime-localtime" };
      return name;
    }

    static long long GetNanoseconds()
    {
      const std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
      return(std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count());

    }
    
    static long long GetMicroseconds()
    {
      const std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
      return std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();
    }

    static bool SetNanoseconds(long long /*time_*/)
    {
      return false;
    }

    static bool IsSynchronized()
    {
      // technically not true, but has been the legacy behavior for localtime.
      return true;
    }

    static bool IsMaster()
    {
      // technically not true, but has been the legacy behavior for localtime.
      return true;
    }

    static void SleepForNanoseconds(long long duration_nsecs_)
    {
      eCAL::Process::SleepNS(duration_nsecs_);
    }

    static void GetStatus(int& error_, std::string* const status_message_)
    {
      error_ = 0;
      if (status_message_)
      {
        *status_message_ = "everything is fine.";
      }
    }

  };
}
