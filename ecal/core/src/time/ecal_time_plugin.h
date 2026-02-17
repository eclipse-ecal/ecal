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
        InvalidArgument,        /* An empty path was passed to loading the library */
        LoadFailed,             /* Plugin or dependent shared libraries could not be loaded */
        PlatformError,
        IncompleteAPI,          /* Plugin has been loaded, but not all symbols were found */
        InitializationFailed    /* Plugin has been loaded, but initilization failed */
      };

      Code code{ Code::PlatformError };
      std::string message;
    };

    // Factory method to try and load a given plugin
    // Loading of the plugin can fail for serveral reasons, as indicated by the error code.
    static std::variant<CTimePlugin, Error> LoadFromPath(std::string full_path_) noexcept;

    CTimePlugin(const CTimePlugin&) = delete;
    CTimePlugin& operator=(const CTimePlugin&) = delete;

    CTimePlugin(CTimePlugin&&) noexcept;
    CTimePlugin& operator=(CTimePlugin&&) = delete;

    ~CTimePlugin();

    const std::string& LoadedPath() const noexcept { return m_loaded_path; }

    long long GetNanoSeconds() const override;
    bool SetNanoSeconds(long long time_) const override;

    bool IsSynchronized() const override;
    bool IsMaster() const override;

    void SleepForNanoseconds(long long duration_nsecs_) const override;

    void GetStatus(int& error_, std::string* const status_message_) const override;

  private:
    /* Structure to enable calling into the plugin */
    struct Api
    {
      decltype(&etime_initialize)            Initialize = nullptr;
      decltype(&etime_finalize)              Finalize = nullptr;
      decltype(&etime_get_nanoseconds)       GetNanoSeconds = nullptr;
      decltype(&etime_set_nanoseconds)       SetNanoSeconds = nullptr;
      decltype(&etime_is_synchronized)       IsSynchronized = nullptr;
      decltype(&etime_is_master)             IsMaster = nullptr;
      decltype(&etime_sleep_for_nanoseconds) SleepForNanoseconds = nullptr;
      decltype(&etime_get_status)            GetStatus = nullptr;
    };

    CTimePlugin(EcalUtils::DynamicLibrary&& lib_, Api api_, std::string loaded_path_) noexcept;

    static std::variant<Api, Error> LoadApi(const EcalUtils::DynamicLibrary& lib_) noexcept;

  private:
    EcalUtils::DynamicLibrary m_loaded_library;
    Api m_api;
    std::string m_loaded_path;
  };
}
