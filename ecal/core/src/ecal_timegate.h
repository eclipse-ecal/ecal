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
 * @brief  eCAL time gateway class
**/

#pragma once

#include <ecal/ecal.h>

#include "ecal_global_accessors.h"

#include <atomic>

#ifdef ECAL_OS_WINDOWS

#include "ecal_win_main.h"

typedef int       (__cdecl *etime_initialize)           (void);
typedef int       (__cdecl *etime_finalize)             (void);
typedef long long (__cdecl *etime_get_nanoseconds)      (void);
typedef int       (__cdecl *etime_set_nanoseconds)      (long long time_);
typedef int       (__cdecl *etime_is_synchronized)      (void);
typedef int       (__cdecl *etime_is_master)            (void);
typedef int       (__cdecl *etime_sleep_for_nanoseconds)(long long duration_nsecs_);
typedef void      (__cdecl *etime_get_status)           (int*, char*, const int);

#endif // ECAL_OS_WINDOWS

#ifdef ECAL_OS_LINUX
typedef int       (*etime_initialize)           (void);
typedef int       (*etime_finalize)             (void);
typedef long long (*etime_get_nanoseconds)      (void);
typedef int       (*etime_set_nanoseconds)      (long long time_);
typedef int       (*etime_is_synchronized)      (void);
typedef int       (*etime_is_master)            (void);
typedef int       (*etime_sleep_for_nanoseconds)(long long duration_nsecs_);
typedef void      (*etime_get_status)           (int*, char*, const int);
#endif //ECAL_OS_LINUX


namespace eCAL
{
  class CTimeGate
  {
  public:
    enum eTimeSyncMode { none, realtime, replay };

    CTimeGate();
    ~CTimeGate();

    void Create(enum eTimeSyncMode sync_mode_);
    void Destroy();

    std::string GetName();

    long long GetMicroSeconds();
    long long GetNanoSeconds();

    bool SetNanoSeconds(long long time_);

    bool IsSynchronized();
    bool IsMaster();
    
    void SleepForNanoseconds(long long duration_nsecs_);

    void GetStatus(int& error_, std::string* const status_message_);
    bool IsValid();

    eTimeSyncMode GetSyncMode() { return(m_sync_mode); };

  protected:
    static std::atomic<bool>  m_created;
    std::string               m_time_sync_modname;
    std::atomic<bool>         m_is_initialized_rt;
    std::atomic<bool>         m_is_initialized_replay;
    std::atomic<bool>         m_successfully_loaded_rt;
    std::atomic<bool>         m_successfully_loaded_replay;
    eTimeSyncMode             m_sync_mode;

    struct STimeDllInterface
    {
      STimeDllInterface() :
        module_handle(nullptr),
        module_name(),
        etime_initialize_ptr(nullptr),
        etime_finalize_ptr(nullptr),
        etime_get_nanoseconds_ptr(nullptr),
        etime_set_nanoseconds_ptr(nullptr),
        etime_is_synchronized_ptr(nullptr),
        etime_is_master_ptr(nullptr),
        etime_sleep_for_nanoseconds_ptr(nullptr),
        etime_get_status_ptr(nullptr)
      {
      }

#ifdef ECAL_OS_WINDOWS
      HINSTANCE                   module_handle;
#else // ECAL_OS_WINDOWS
      void*                       module_handle;
#endif // ECAL_OS_WINDOWS

      std::string                 module_name;
      etime_initialize            etime_initialize_ptr;
      etime_finalize              etime_finalize_ptr;
      etime_get_nanoseconds       etime_get_nanoseconds_ptr;
      etime_set_nanoseconds       etime_set_nanoseconds_ptr;
      etime_is_synchronized       etime_is_synchronized_ptr;
      etime_is_master             etime_is_master_ptr;
      etime_sleep_for_nanoseconds etime_sleep_for_nanoseconds_ptr;
      etime_get_status            etime_get_status_ptr;
    };
    bool LoadModule(const std::string& interface_name_, STimeDllInterface& interface_);

    STimeDllInterface        m_time_sync_rt;
    STimeDllInterface        m_time_sync_replay;
  };
};
