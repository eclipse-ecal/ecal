/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
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
 * @brief  eCAL time gateway class
**/

#include <atomic>
#include <ecal/ecal.h>

#include <ecal/config.h>

#include "ecal_def.h"
#include "ecal_timegate.h"
#include "util/getenvvar.h"

#include <chrono>
#include <string>

namespace
{
  constexpr const char* ECAL_TIME_PLUGIN_DIR = "ecaltime_plugins";

  std::string build_dynamic_library_name(std::string module_name)
  {
#ifdef _WIN32
#ifndef NDEBUG
    module_name += "d";
#endif // !NDEBUG
    // set extension
    module_name += ".dll";
#endif // _WIN32

#ifdef __linux__
    module_name = "lib" + module_name + ".so";
#endif // __linux__

#ifdef __APPLE__
    module_name = "lib" + module_name + ".dylib";
#endif // __APPLE__

    return module_name;
  }

  std::string remove_quotes(std::string module_name)
  {
    if (module_name.size() > 2)
    {
      if (module_name[0] == '\"' || module_name[0] == '\'')
      {
        module_name.erase(0, 1);
      }
      if (module_name[module_name.size() - 1] == '\"' || module_name[module_name.size() - 1] == '\'')
      {
        module_name.erase(module_name.size() - 1);
      }
    }
    return module_name;
  }

  std::vector<std::string> build_plugin_paths(std::string dynamic_library_name)
  {
    // TODO should we use ECAL_OS_WINDOWS?
#ifdef _WIN32
    std::string path_separator = "\\";
#else
    std::string path_separator = "/";
#endif

    // 1. Load from paths that are specified in the time plugin environment variable
    std::vector<std::string> candidate_paths;
    const auto ecal_time_plugin_paths = splitPaths(getEnvVar("ECAL_TIME_PLUGIN_PATH"));
    for (const auto& ecal_time_plugin_path : ecal_time_plugin_paths)
    {
      const auto module_path = ecal_time_plugin_path + path_separator + dynamic_library_name;
      candidate_paths.push_back(module_path);
    }

    // 2. Load from standard path
    candidate_paths.push_back(dynamic_library_name);
  
    // 3. Load from sub folder ECAL_TIME_PLUGIN_DIR
    const auto module_path = std::string(ECAL_TIME_PLUGIN_DIR) + path_separator + dynamic_library_name;
    candidate_paths.push_back(module_path);

    return candidate_paths;
  }
}


namespace eCAL
{
  std::shared_ptr<CTimeGate> CTimeGate::CreateTimegate()
  {
    const std::string module_name = remove_quotes(Config::GetTimesyncModuleName());

    if (module_name.empty())
    {
      eCAL::Logging::Log(Logging::log_level_warning, "Could not load eCAL time sync module, no name specified. Falling back to ecaltime-localtime.");
      return nullptr;
    }

    if (module_name == "ecaltime-localtime")
    {
      eCAL::Logging::Log(Logging::log_level_info, "Using built-in local time plugin.");
      return nullptr;
    }

    const auto possible_plugin_paths = build_plugin_paths(build_dynamic_library_name(module_name));

    for (const auto& path : possible_plugin_paths)
    {
      auto load_result = CTimePlugin::LoadFromPath(path);
      if (std::holds_alternative<CTimePlugin>(load_result))
      {
        eCAL::Logging::Log(Logging::log_level_info, "Successfully loaded eCAL time sync plugin from path: " + std::get<CTimePlugin>(load_result).LoadedPath());
        return std::shared_ptr<CTimeGate>(new CTimeGate(std::get<CTimePlugin>(std::move(load_result)), module_name));
      }
      else
      {
        const auto& error = std::get<CTimePlugin::Error>(load_result);
        if (error.code == CTimePlugin::Error::Code::IncompleteAPI)
        {
          eCAL::Logging::Log(Logging::log_level_warning, "eCAL time sync plugin API incomplete for plugin at path: " + path + ". Error: " + error.message + " Falling back to ecaltime-localtime.");
          return nullptr;
        }
      }
    }
    eCAL::Logging::Log(Logging::log_level_warning, "Failed to load eCAL time sync plugin " + module_name + ". Shared library (or dependencies) not found. Falling back to ecaltime-localtime.");
    return nullptr;
  }

  CTimeGate::CTimeGate(CTimePlugin&& time_plugin, std::string plugin_name)
    : m_time_plugin(std::move(time_plugin))
    , m_plugin_name(std::move(plugin_name))
  {
  }

  CTimeGate::~CTimeGate() = default;

  //////////////////////////////////////////////////////////////////
  // CTimeGate
  //////////////////////////////////////////////////////////////////
  const std::string& CTimeGate::GetName() const
  {
    return(m_plugin_name);
  }

  long long CTimeGate::GetMicroSeconds()
  {
    return(GetNanoSeconds() / 1000);
  }

  long long CTimeGate::GetNanoSeconds()
  {
    return m_time_plugin.GetNanoseconds();
  }

  bool CTimeGate::SetNanoSeconds(long long time_)
  {
    return m_time_plugin.SetNanoseconds(time_);
  }

  bool CTimeGate::IsSynchronized()
  {
    return m_time_plugin.IsSynchronized();
  }

  bool CTimeGate::IsMaster()
  {
    return m_time_plugin.IsMaster();
  }

  void CTimeGate::SleepForNanoseconds(long long duration_nsecs_)
  {
    m_time_plugin.SleepForNanoseconds(duration_nsecs_);
  }

  void CTimeGate::GetStatus(int& error_, std::string* const status_message_)
  {
    m_time_plugin.GetStatus(error_, status_message_);
  }
}
