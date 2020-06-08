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

#pragma once

#include <string>
#include <functional>
#include <vector>
#include <chrono>
#include <memory>

#include <rec_client_core/state.h>

#include "addon.h"

namespace eCAL
{
  namespace rec
  {
    class AddonManager
    {
    ////////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////////
    public:
      AddonManager(std::function<void(std::int64_t job_id, const std::string& addon_id, const RecAddonJobStatus& job_status)> set_job_status_function);
      ~AddonManager();

    ////////////////////////////////////////
    // Public API
    ////////////////////////////////////////
    public:
      void SetEnabledAddons(const std::set<std::string>& addon_ids);
      std::set<std::string> GetEnabledAddons() const;

      void Initialize();
      void Deinitialize();

      void SetPreBuffer(bool enabled, std::chrono::steady_clock::duration length);

      void StartRecording(const JobConfig& job_config);
      void StopRecording();
      void SaveBuffer(const JobConfig& job_config);

      std::vector<RecorderAddonStatus> GetAddonStatuses() const;

    ////////////////////////////////////////
    // Private methods
    ////////////////////////////////////////
    private:
        std::vector<std::string> GetAddonDirectories();
        std::vector<std::string> GetAddonExecutablePaths(const std::vector<std::string>& addon_dirs);


    ////////////////////////////////////////
    // Member Variables
    ////////////////////////////////////////
    private:
      std::vector<std::unique_ptr<Addon>> enabled_addons_;
      std::vector<std::unique_ptr<Addon>> disabled_addons_;
      std::vector<std::unique_ptr<Addon>> uninitialized_addons_;

      // Addon Settings
      bool                                initialized_;
      bool                                pre_buffer_enabled_;
      std::chrono::steady_clock::duration max_pre_buffer_length_;
    };
  }
}