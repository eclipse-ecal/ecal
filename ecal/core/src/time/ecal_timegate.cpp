/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2024 Continental Corporation
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

#include <ecal/ecal_config.h>

#include "ecal_def.h"
#include "ecal_timegate.h"
#include "util/getenvvar.h"

#include <chrono>
#include <string>

#define etime_initialize_name            "etime_initialize"
#define etime_finalize_name              "etime_finalize"
#define etime_get_nanoseconds_name       "etime_get_nanoseconds"
#define etime_set_nanoseconds_name       "etime_set_nanoseconds"
#define etime_is_synchronized_name       "etime_is_synchronized"
#define etime_is_master_name             "etime_is_master"
#define etime_sleep_for_nanoseconds_name "etime_sleep_for_nanoseconds"
#define etime_get_status_name            "etime_get_status"

#define ecal_time_plugin_dir             "ecaltime_plugins"

#if defined(__linux__) || defined(__APPLE__)
#include <dlfcn.h>
#endif // defined(__linux__) || defined(__APPLE__)

namespace eCAL
{
  //////////////////////////////////////////////////////////////////
  // CTimeGate
  //////////////////////////////////////////////////////////////////
  std::atomic<bool> CTimeGate::m_created;

  CTimeGate::CTimeGate() :
    m_is_initialized_rt(false),
    m_is_initialized_replay(false),
    m_successfully_loaded_rt(false),
    m_successfully_loaded_replay(false),
    m_sync_mode(eTimeSyncMode::none)
  {
  }

  CTimeGate::~CTimeGate()
  {
    Stop();
  }

  void CTimeGate::Start(enum eTimeSyncMode sync_mode_)
  {
    if(m_created) return;

    // load time sync modules
    m_time_sync_modname.clear();
    m_is_initialized_rt     = false;
    m_is_initialized_replay = false;
    m_sync_mode             = sync_mode_;

    switch (m_sync_mode)
    {
    case eTimeSyncMode::none:
      break;
    case eTimeSyncMode::realtime:
      m_time_sync_modname = Config::GetTimesyncModuleName();
      m_successfully_loaded_rt = LoadModule(m_time_sync_modname, m_time_sync_rt);
      break;
    case eTimeSyncMode::replay:
      m_time_sync_modname = Config::GetTimesyncModuleReplay();
      m_successfully_loaded_replay = LoadModule(m_time_sync_modname, m_time_sync_replay);
      break;
    }

    // initialize real time
    if (!m_is_initialized_rt && (m_time_sync_rt.etime_initialize_ptr != nullptr))
    {
      m_is_initialized_rt = m_time_sync_rt.etime_initialize_ptr() == 0;
    }

    // initialize replay
    if (!m_is_initialized_replay && (m_time_sync_replay.etime_initialize_ptr != nullptr))
    {
      m_is_initialized_replay = m_time_sync_replay.etime_initialize_ptr() == 0;
    }

    m_created = true;
  }

  void CTimeGate::Stop()
  {
    if(!m_created) return;
    switch (m_sync_mode)
    {
    case eTimeSyncMode::none:
      break;
    case eTimeSyncMode::realtime:
      if (m_is_initialized_rt)
        m_time_sync_rt.etime_finalize_ptr();
      break;
    case eTimeSyncMode::replay:
      if (m_is_initialized_replay)
        m_time_sync_replay.etime_finalize_ptr();
      break;
    default:
      break;
    }
    m_is_initialized_rt     = false;
    m_is_initialized_replay = false;

    m_created = false;
  }

  std::string CTimeGate::GetName()
  {
    if (!m_created) return("");
    return(m_time_sync_modname);
  }

  long long CTimeGate::GetMicroSeconds()
  {
    if (!m_created) return(0);
    return(GetNanoSeconds() / 1000);
  }

  long long CTimeGate::GetNanoSeconds()
  {
    if (!m_created) return(0);

    long long master_ns(0);

    switch (m_sync_mode)
    {
    case eTimeSyncMode::none:
      break;
    case eTimeSyncMode::realtime:
      if (m_is_initialized_rt)
        master_ns = m_time_sync_rt.etime_get_nanoseconds_ptr();
      break;
    case eTimeSyncMode::replay:
      if (m_is_initialized_replay)
        master_ns = m_time_sync_replay.etime_get_nanoseconds_ptr();
      break;
    default:
      break;
    }

    return(master_ns);
  }

  bool CTimeGate::SetNanoSeconds(long long time_)
  {
    if (!m_created) return(false);

    switch (m_sync_mode)
    {
    case eTimeSyncMode::none:
      return(false);
    case eTimeSyncMode::realtime:
      if (m_is_initialized_rt)
        return(m_time_sync_rt.etime_set_nanoseconds_ptr(time_) == 0);
      break;
    case eTimeSyncMode::replay:
      if (m_is_initialized_replay)
        return(m_time_sync_replay.etime_set_nanoseconds_ptr(time_) == 0);
      break;
    default:
      return(false);
    }

    return(false);
  }

  bool CTimeGate::IsSynchronized()
  {
    if (!m_created) return(false);

    switch (m_sync_mode)
    {
    case eTimeSyncMode::none:
      return(false);
    case eTimeSyncMode::realtime:
      if (m_is_initialized_rt)
        return(m_time_sync_rt.etime_is_synchronized_ptr() != 0);
      break;
    case eTimeSyncMode::replay:
      if (m_is_initialized_replay)
        return(m_time_sync_replay.etime_is_synchronized_ptr() != 0);
      break;
    default:
      return(false);
    }
    return(false);
  }

  bool CTimeGate::IsMaster()
  {
    if (!m_created) return(false);

    switch (m_sync_mode)
    {
    case eTimeSyncMode::none:
      return(false);
    case eTimeSyncMode::realtime:
      if (m_is_initialized_rt)
        return(m_time_sync_rt.etime_is_master_ptr() != 0);
      break;
    case eTimeSyncMode::replay:
      if (m_is_initialized_replay)
        return(m_time_sync_replay.etime_is_master_ptr() != 0);
      break;
    default:
      return(false);
    }
    return(false);
  }

  void CTimeGate::SleepForNanoseconds(long long duration_nsecs_)
  {
    if (!m_created) return;

    switch (m_sync_mode)
    {
    case eTimeSyncMode::none:
      eCAL::Process::SleepFor(std::chrono::nanoseconds(duration_nsecs_));
      break;
    case eTimeSyncMode::realtime:
      if (m_is_initialized_rt) {
        m_time_sync_rt.etime_sleep_for_nanoseconds_ptr(duration_nsecs_);
      }
      else {
      eCAL::Process::SleepFor(std::chrono::nanoseconds(duration_nsecs_));
      }
      break;
    case eTimeSyncMode::replay:
      if (m_is_initialized_replay) {
        m_time_sync_replay.etime_sleep_for_nanoseconds_ptr(duration_nsecs_);
      }
      else {
      eCAL::Process::SleepFor(std::chrono::nanoseconds(duration_nsecs_));
      }
      break;
    default:
      eCAL::Process::SleepFor(std::chrono::nanoseconds(duration_nsecs_));
    }
  }

  void CTimeGate::GetStatus(int& error_, std::string* const status_message_)
  {
    if (!m_created) {
      error_ = -1;
      if (status_message_ != nullptr) {
        status_message_->assign("eCAL Timegate has not been created.");
      }
    }
    else
    {
      static const int buffer_len = 256;
      switch (m_sync_mode)
      {
      case eTimeSyncMode::none:
        error_ = 0;
        if (status_message_ != nullptr) {
          status_message_->assign("Timesync mode is set to NONE.");
        }
        break;
      case eTimeSyncMode::realtime:
        if (!m_successfully_loaded_rt){
          error_ = -1;
          if (status_message_ != nullptr) {
            status_message_->assign("Failed to load realtime timesync module ");
            status_message_->append(GetName());
          }
        } else {
          if (status_message_ != nullptr) {
            char buffer[buffer_len];
            buffer[0] = 0x0; 
            m_time_sync_rt.etime_get_status_ptr(&error_, buffer, buffer_len);
            buffer[buffer_len - 1] = 0x0;              // Just in case the module forgot to null-terminate the string
            status_message_->assign(buffer);
          }
          else {
            m_time_sync_rt.etime_get_status_ptr(&error_, nullptr, 0);
          }
        }
        break;
      case eTimeSyncMode::replay:
        if (!m_successfully_loaded_replay){
          error_ = -1;
          if (status_message_ != nullptr) {
            status_message_->assign("Failed to load realtime timesync module ");
            status_message_->append(GetName());
          }
        } else {
          if (status_message_ != nullptr) {
            char buffer[buffer_len];
            buffer[0] = 0x0;
            m_time_sync_replay.etime_get_status_ptr(&error_, buffer, buffer_len);
            buffer[buffer_len - 1] = 0x0;              // Just in case the module forgot to null-terminate the string
            status_message_->assign(buffer);
          }
          else {
            m_time_sync_replay.etime_get_status_ptr(&error_, nullptr, 0);
          }
        }
        break;
      default:
        error_ = -1;
        if (status_message_ != nullptr) {
          status_message_->assign("Unknown Error.");
        }
      }
    }
  }

  bool CTimeGate::IsValid()
  {
    int error(0);
    GetStatus(error, nullptr);
    return (error >= 0);
  }

  bool CTimeGate::LoadModule(const std::string& interface_name_, STimeDllInterface& interface_)
  {
    if (interface_name_.empty()) return false;

    std::string module_name = interface_name_;
    if (module_name.size() > 2)
    {
      if (module_name[0] == '\"' || module_name[0] == '\'')
      {
        module_name.erase(0, 1);
      }
      if (module_name[module_name.size()-1] == '\"' || module_name[module_name.size() - 1] == '\'')
      {
        module_name.erase(module_name.size() - 1);
      }
    }

    const auto ecal_time_plugin_paths = splitPaths(getEnvVar("ECAL_TIME_PLUGIN_PATH"));

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

    if (interface_.module_handle == nullptr)
    {
#ifdef _WIN32
      // try to load plugin from paths that are specified in the time plugin environment variable
      for (const auto& ecal_time_plugin_path : ecal_time_plugin_paths)
      {
        const auto module_path = ecal_time_plugin_path + "\\" + module_name;
        interface_.module_handle = LoadLibrary(module_path.c_str());
        if (interface_.module_handle != nullptr) break;
      }

      // try to load plugin in standard path
      if (interface_.module_handle == nullptr)
      {
        interface_.module_handle = LoadLibrary(module_name.c_str());
      }

      // try to load plugin from sub folder "ecal_time_plugin_dir"
      if (interface_.module_handle == nullptr)
      {
        const auto module_path = std::string(ecal_time_plugin_dir) + "\\" + module_name;
        interface_.module_handle = LoadLibrary(module_path.c_str());
      }
#endif // _WIN32
#if defined(__linux__) || defined(__APPLE__)
      // try to load plugin from paths that are specified in the time plugin environment variable
      for (const auto& ecal_time_plugin_path : ecal_time_plugin_paths)
      {
        const auto module_path = ecal_time_plugin_path + "/" + module_name;
        interface_.module_handle = dlopen(module_path.c_str(), RTLD_NOW);
        if (interface_.module_handle) break;
      }

      // try to load plugin in standard path
      if (!interface_.module_handle)
      {
        const auto module_path = module_name;
        interface_.module_handle = dlopen(module_path.c_str(), RTLD_NOW);
      }
#endif // defined(__linux__) || defined(__APPLE__)

      if (interface_.module_handle == nullptr)
      {
        eCAL::Logging::Log(log_level_error, "Could not load eCAL time sync module " + module_name);
        return false;
      }
      else
      {
#ifdef _WIN32
        interface_.module_name               = module_name;
        interface_.etime_initialize_ptr      = (etime_initialize)                  GetProcAddress(interface_.module_handle, etime_initialize_name);
        interface_.etime_finalize_ptr        = (etime_finalize)                    GetProcAddress(interface_.module_handle, etime_finalize_name);
        interface_.etime_get_nanoseconds_ptr = (etime_get_nanoseconds)             GetProcAddress(interface_.module_handle, etime_get_nanoseconds_name);
        interface_.etime_set_nanoseconds_ptr = (etime_set_nanoseconds)             GetProcAddress(interface_.module_handle, etime_set_nanoseconds_name);
        interface_.etime_is_synchronized_ptr = (etime_is_synchronized)             GetProcAddress(interface_.module_handle, etime_is_synchronized_name);
        interface_.etime_is_master_ptr       = (etime_is_master)                   GetProcAddress(interface_.module_handle, etime_is_master_name);
        interface_.etime_sleep_for_nanoseconds_ptr = (etime_sleep_for_nanoseconds) GetProcAddress(interface_.module_handle, etime_sleep_for_nanoseconds_name);
        interface_.etime_get_status_ptr       = (etime_get_status)                 GetProcAddress(interface_.module_handle, etime_get_status_name);
#endif // _WIN32
#if defined(__linux__) || defined(__APPLE__)
        interface_.module_name               = module_name;
        interface_.etime_initialize_ptr      = (etime_initialize)                  dlsym(interface_.module_handle, etime_initialize_name);
        interface_.etime_finalize_ptr        = (etime_finalize)                    dlsym(interface_.module_handle, etime_finalize_name);
        interface_.etime_get_nanoseconds_ptr = (etime_get_nanoseconds)             dlsym(interface_.module_handle, etime_get_nanoseconds_name);
        interface_.etime_set_nanoseconds_ptr = (etime_set_nanoseconds)             dlsym(interface_.module_handle, etime_set_nanoseconds_name);
        interface_.etime_is_synchronized_ptr = (etime_is_synchronized)             dlsym(interface_.module_handle, etime_is_synchronized_name);
        interface_.etime_is_master_ptr       = (etime_is_master)                   dlsym(interface_.module_handle, etime_is_master_name);
        interface_.etime_sleep_for_nanoseconds_ptr = (etime_sleep_for_nanoseconds) dlsym(interface_.module_handle, etime_sleep_for_nanoseconds_name);
        interface_.etime_get_status_ptr      = (etime_get_status)                  dlsym(interface_.module_handle, etime_get_status_name);
#endif // defined(__linux__) || defined(__APPLE__)

        if (  (interface_.etime_initialize_ptr            == nullptr)
           || (interface_.etime_finalize_ptr              == nullptr)
           || (interface_.etime_get_nanoseconds_ptr       == nullptr)
           || (interface_.etime_set_nanoseconds_ptr       == nullptr)
           || (interface_.etime_is_synchronized_ptr       == nullptr)
           || (interface_.etime_is_master_ptr             == nullptr)
           || (interface_.etime_sleep_for_nanoseconds_ptr == nullptr)
           || (interface_.etime_get_status_ptr            == nullptr)
          )
        {
          eCAL::Logging::Log(log_level_error, "Could not load eCAL time sync module " + module_name);
          return false;
        }
      }
    }

    return true;
  }
}
