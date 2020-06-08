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

#include "addon_manager.h"

#include <rec_client_core/ecal_rec_logger.h>

#include <ecal_utils/filesystem.h>
#include <ecal_utils/string.h>

#include <list>

namespace eCAL
{
  namespace rec
  {
    ////////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////////

    std::vector<std::string> AddonManager::GetAddonDirectories()
    {
      std::vector<std::string> addon_dirs;
      auto default_addon_dir = EcalUtils::Filesystem::ApplicationDir();
#ifdef WIN32
      default_addon_dir += std::string(1, EcalUtils::Filesystem::NativeSeparator()) + "ecalrec_addons";
#else
      default_addon_dir += std::string(1, EcalUtils::Filesystem::NativeSeparator()) + ".."
                         + std::string(1, EcalUtils::Filesystem::NativeSeparator()) + "lib"
                         + std::string(1, EcalUtils::Filesystem::NativeSeparator()) + "ecal"
                         + std::string(1, EcalUtils::Filesystem::NativeSeparator()) + "addons"
                         + std::string(1, EcalUtils::Filesystem::NativeSeparator()) + "rec";
#endif
      
      addon_dirs.push_back(default_addon_dir);

      auto additional_addon_dirs = std::getenv("ECAL_REC_ADDON_PATH");
      if (additional_addon_dirs != nullptr)
      {
        auto dir_delimiter
#ifdef WIN32
        { ";" };
#else
        { ":" };
#endif
        EcalUtils::String::Split(std::string(additional_addon_dirs), dir_delimiter, addon_dirs);
      }

      return addon_dirs;
    }

    std::vector<std::string> AddonManager::GetAddonExecutablePaths(const std::vector<std::string>& addon_dirs)
    {
      std::vector<std::string> addon_exec_paths;
      for (const auto& addon_dir : addon_dirs)
      {
        auto addon_exec_files = EcalUtils::Filesystem::DirContent(addon_dir);

        for (const auto& addon_exec_file : addon_exec_files)
        {
          if(
            (addon_exec_file.second.GetType() == EcalUtils::Filesystem::Type::RegularFile) &&
            (addon_exec_file.second.PermissionRootExecute() || addon_exec_file.second.PermissionOwnerExecute() || addon_exec_file.second.PermissionGroupExecute())
            )
          {
            addon_exec_paths.push_back(addon_dir + std::string(1, EcalUtils::Filesystem::NativeSeparator()) + addon_exec_file.first);
          }
        }

      }
      return addon_exec_paths;
    }

    AddonManager::AddonManager(std::function<void(std::int64_t job_id, const std::string& addon_id, const RecAddonJobStatus& job_status)> set_job_status_function)
      : initialized_          (false)
      , pre_buffer_enabled_   (false)
      , max_pre_buffer_length_(std::chrono::steady_clock::duration(0))
    {

      auto addon_dirs = GetAddonDirectories();
      auto addon_paths = GetAddonExecutablePaths(addon_dirs);

      for (const auto& addon_path : addon_paths)
      {
        EcalRecLogger::Instance()->info("Found addon in path \"" + addon_path + "\".");
        uninitialized_addons_.emplace_back(std::make_unique<Addon>(addon_path, set_job_status_function));
      }

      std::unordered_map<std::string, std::unique_ptr<Addon>> initialized_addons;

      for(auto counter = 0; counter < 100; ++counter)
      {
        //std::vector<decltype(uninitialized_addons_)::iterator> erasable_addon_iters;
        for (auto addon_iter = uninitialized_addons_.begin(); addon_iter != uninitialized_addons_.end(); )
        {
          auto addon_id = (*addon_iter)->GetAddonId();
          
          if (!addon_id.empty())
          {
            if (!initialized_addons.emplace(std::piecewise_construct, std::forward_as_tuple(addon_id), std::forward_as_tuple(std::move((*addon_iter)))).second)
            {
              EcalRecLogger::Instance()->warn("Addon \"" + addon_id + "\" already exists.");
            }
            else
            {
              EcalRecLogger::Instance()->info("Addon  \"" + addon_id + "\" successfully loaded.");
            }
            addon_iter = uninitialized_addons_.erase(addon_iter);
          }
          else
          {
            ++addon_iter;
          }
        }

        if (uninitialized_addons_.empty()) break;

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
      }

      if (!uninitialized_addons_.empty())
      {
        for (const auto& uninitialized_addon : uninitialized_addons_)
        {
          EcalRecLogger::Instance()->warn("Addon  \"" + uninitialized_addon->GetStatus().addon_executable_path_ +"\" does not response.");
        }
      }

      // Move initialized add-ons to disabled add-ons list
      for (auto&& initialized_addon : initialized_addons)
      {
        disabled_addons_.push_back(std::move(initialized_addon.second));
      }
    }

    AddonManager::~AddonManager()
    {}

    ////////////////////////////////////////
    // Public API
    ////////////////////////////////////////

    void AddonManager::SetEnabledAddons(const std::set<std::string>& addon_ids)
    {
      std::list<std::unique_ptr<Addon>> addons_to_enable;
      std::list<std::unique_ptr<Addon>> addons_to_disable;

      // Collect addons to enable
      for (const std::string& addon_id : addon_ids)
      {
        bool addon_found{ false };

        for (auto disabled_addon_it = disabled_addons_.begin(); disabled_addon_it != disabled_addons_.end();)
        {
          if ((*disabled_addon_it)->GetAddonId() == addon_id)
          {
            addons_to_enable.push_back(std::move(*disabled_addon_it));
            disabled_addon_it = disabled_addons_.erase(disabled_addon_it);
            addon_found = true;
            break;
          }
          else
          {
            ++disabled_addon_it;
          }
        }

        // Let's see if the addon is already enabled. If it isn't, the user wants to enable a non-existing addon and we print a warning.
        if (!addon_found)
        {
          auto already_enabled_addon = std::find_if(enabled_addons_.begin(), enabled_addons_.end()
                                                  , [&addon_id](const std::unique_ptr<Addon>& addon) -> bool
                                                    {
                                                      return addon->GetAddonId() == addon_id;
                                                    });
          if (already_enabled_addon == enabled_addons_.end())
          {
            EcalRecLogger::Instance()->warn("Failed enabling addon \"" + addon_id + "\": the addon does not exist.");
          }
        }
      }

      // Collect addons to disable
      for (auto enabled_addon_it = enabled_addons_.begin(); enabled_addon_it != enabled_addons_.end();)
      {
        if (addon_ids.find((*enabled_addon_it)->GetAddonId()) == addon_ids.end())
        {
          addons_to_disable.push_back(std::move(*enabled_addon_it));
          enabled_addon_it = enabled_addons_.erase(enabled_addon_it);
        }
        else
        {
          ++enabled_addon_it;
        }
      }

      // Actually enable addons that were not enabled before
      for (auto& addon : addons_to_enable)
      {
        EcalRecLogger::Instance()->info("Enabling addon  \"" + addon->GetAddonId() + "\".");

        addon->SetPreBuffer(pre_buffer_enabled_, max_pre_buffer_length_);
        if (initialized_)
        {
          addon->Initialize();
        }
        enabled_addons_.push_back(std::move(addon));
      }

      // Actually disable addons that were enabled before
      for (auto& addon : addons_to_disable)
      {
        EcalRecLogger::Instance()->info("Disabling addon \"" + addon->GetAddonId() + "\".");

        addon->SetPreBuffer(pre_buffer_enabled_, max_pre_buffer_length_);
        if (initialized_)
        {
          addon->Deinitialize();
        }
        disabled_addons_.push_back(std::move(addon));
      }
    }

    std::set<std::string> AddonManager::GetEnabledAddons() const
    {
      std::set<std::string> enabled_addon_ids;

      for (const auto& addon : enabled_addons_)
      {
        enabled_addon_ids.emplace(addon->GetAddonId());
      }

      return enabled_addon_ids;
    }

    void AddonManager::Initialize()
    {
      initialized_ = true;
      for (const auto& addon : enabled_addons_)
      {
        addon->Initialize();
      }
    }

    void AddonManager::Deinitialize()
    {
      initialized_ = false;
      for (const auto& addon : enabled_addons_)
      {
        addon->Deinitialize();
      }
    }

    void AddonManager::SetPreBuffer(bool enabled, std::chrono::steady_clock::duration length)
    {
      pre_buffer_enabled_    = enabled;
      max_pre_buffer_length_ = length;
      for (const auto& addon : enabled_addons_)
      {
        addon->SetPreBuffer(enabled, length);
      }
    }

    void AddonManager::StartRecording(const JobConfig& job_config)
    {
      for (const auto& addon : enabled_addons_)
      {
        addon->StartRecording(job_config);
      }
    }

    void AddonManager::StopRecording()
    {
      for (const auto& addon : enabled_addons_)
      {
        addon->StopRecording();
      }
    }

    void AddonManager::SaveBuffer(const JobConfig& job_config)
    {
      for (const auto& addon : enabled_addons_)
      {
        addon->SaveBuffer(job_config);
      }
    }

    std::vector<RecorderAddonStatus> AddonManager::GetAddonStatuses() const
    {
      std::vector<RecorderAddonStatus> statuses;
      statuses.reserve(enabled_addons_.size() + disabled_addons_.size());

      for (const auto& addon : enabled_addons_)
      {
        if (addon->IsRunning())
        {
          const auto status = addon->GetStatus();
          statuses.push_back(status);
        }
      }

      for (const auto& addon : disabled_addons_)
      {
        if (addon->IsRunning())
        {
          const auto status = addon->GetStatus();
          statuses.push_back(status);
        }
      }

      return statuses;
    }
  }
}