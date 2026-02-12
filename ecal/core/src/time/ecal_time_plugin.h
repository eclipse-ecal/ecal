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

#include <string>
#include <variant>
#include <vector>
#include <optional>

#include <ecal_utils/dynamic_library.h>
#include "ecaltime.h"
#include "time/ecal_time_base.h"

namespace eCAL
{

  class CTimePlugin : public CTimeBase
  {
  public:
    struct Error
    {
      enum class Code
      {
        InvalidArgument,
        LoadFailed,
        PlatformError,
        IncompleteAPI
      };

      Code code{ Code::PlatformError };
      std::string message;
    };

    // Path handling is done outside: caller passes the exact path to try.
    static std::variant<CTimePlugin, Error> LoadFromPath(std::string full_path) noexcept;

    CTimePlugin(const CTimePlugin&) = delete;
    CTimePlugin& operator=(const CTimePlugin&) = delete;

    CTimePlugin(CTimePlugin&&) noexcept;
    CTimePlugin& operator=(CTimePlugin&&) = delete;

    ~CTimePlugin();

    const std::string& LoadedPath() const noexcept { return loaded_path_; }

    long long GetNanoseconds() const override;
    bool SetNanoseconds(long long time_) const override;

    bool IsSynchronized() const override;
    bool IsMaster() const override;

    void SleepForNanoseconds(long long duration_nsecs_) const override;

    void GetStatus(int& error_, std::string* const status_message_) const override;

  private:
    struct Api
    {
      decltype(&etime_initialize)            Initialize = nullptr;
      decltype(&etime_finalize)              Finalize = nullptr;
      decltype(&etime_get_nanoseconds)       GetNanoseconds = nullptr;
      decltype(&etime_set_nanoseconds)       SetNanoseconds = nullptr;
      decltype(&etime_is_synchronized)       IsSynchronized = nullptr;
      decltype(&etime_is_master)             IsMaster = nullptr;
      decltype(&etime_sleep_for_nanoseconds) SleepForNanoseconds = nullptr;
      decltype(&etime_get_status)            GetStatus = nullptr;
    };

    CTimePlugin(EcalUtils::DynamicLibrary&& lib, Api api, std::string loaded_path) noexcept;

    static std::variant<Api, Error> LoadApi(const EcalUtils::DynamicLibrary& lib) noexcept;

  private:
    EcalUtils::DynamicLibrary loaded_library_;
    Api api_;
    std::string loaded_path_;
  };
}
