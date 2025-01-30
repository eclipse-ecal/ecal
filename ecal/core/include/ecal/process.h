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
 * @file   process.h
 * @brief  eCAL process interface
**/

#pragma once

#include <chrono>
#include <string>

#include <ecal/os.h>
#include <ecal/process_mode.h>
#include <ecal/process_severity.h>
#include <ecal/deprecate.h>

namespace eCAL
{
  namespace Process
  {
    /**
     * @brief  Dump configuration to console. 
    **/
    ECAL_API void DumpConfig();

    /**
     * @brief  Dump configuration to std::string.
     *
     * @param  cfg_s_  String to dump configuration to.
    **/
    ECAL_API void DumpConfig(std::string& cfg_s_);

    /**
     * @brief  Get current host name. 
     *
     * @return  Host name or empty string if failed. 
    **/
    ECAL_API std::string GetHostName();

    /**
     * @brief  Get current SHM transport domain.
     *
     * @return  SHM transport domain or empty string if failed.
    **/
    ECAL_API std::string GetShmTransportDomain();

    /**
     * @brief  Get current unit name (defined via eCAL::Initialize). 
     *
     * @return  Unit name or empty string if failed. 
    **/
    ECAL_API std::string GetUnitName();

    /**
     * @brief  Get current process parameter (defined via eCAL::Initialize(argc, argv). 
     *
     * @param sep_  Separator. 
     *
     * @return  Task parameter separated by 'sep_' or zero string if failed. 
    **/
    ECAL_API std::string GetTaskParameter(const char* sep_);

    /**
     * @brief  Sleep current thread.
     *
     * Because of the fact that std::this_thread::sleep_for is vulnerable to system clock changes
     * on Windows, Sleep function from synchapi.h had to be used for Windows. This insures time
     * robustness on all platforms from a thread sleep perspective.
     *
     * @param  time_ms_  Time to sleep in ms.
    **/
    ECAL_API void SleepMS(long time_ms_);

    /**
     * @brief  Sleep current thread.
     *
     * Because of the fact that std::this_thread::sleep_for is vulnerable to system clock changes
     * on Windows, Sleep function from synchapi.h had to be used for Windows. This insures time
     * robustness on all platforms from a thread sleep perspective. Used with ns unit to obtain bigger precision.
     *
     * @param  time_ns_  Time to sleep in ns.
    **/
    ECAL_API void SleepNS(long long time_ns_);

    /**
     * @brief  Sleep current thread.
     *
     * Templated implementation which takes as argument a std::chrono::duration and calls underlying SleepNS function.
     * By using a std::chrono::duration argument we ensure that conversion to ms would be more precise for Windows Sleep method.
     *
     * @param  time  Time to sleep expressed in std::chrono::duration.
    **/

    template <typename Rep, typename Period>
    void SleepFor( std::chrono::duration<Rep, Period> time )
    {
        auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(time).count();
        SleepNS(ns);
    }

    /**
     * @brief  Get current process id. 
     *
     * @return  The process id. 
    **/
    ECAL_API int GetProcessID();

    /**
     * @brief  Get current process id as string.
     *
     * @return  The process id.
    **/
    ECAL_API std::string GetProcessIDAsString();

    /**
     * @brief  Get current process name. 
     *
     * @return  Process name length or zero if failed. 
    **/
    ECAL_API std::string GetProcessName();

    /**
     * @brief  Get current process parameter. 
     *
     * @return  Process parameter or empty string if failed. 
    **/
    ECAL_API std::string GetProcessParameter();

    /**
     * @brief  Set process state info. 
     *
     * @param severity_  Severity. 
     * @param level_     Severity level.
     * @param info_      Info message.
     *
    **/
    ECAL_API void SetState(eCAL::Process::eSeverity severity_, eCAL::Process::eSeverityLevel level_, const char* info_);

    /**
     * @deprecated This function is deprecated and will be removed in future eCAL versions.
     * @brief  Start specified process (windows only). 
     *
     * @param proc_name_       Process name. 
     * @param proc_args_       Process argument string. 
     * @param working_dir_     Working directory. 
     * @param create_console_  Start process in own console window (Windows only). 
     * @param process_mode_    Start process normal, hidden, minimized, maximized (Windows only). 
     * @param block_           Block until process finished. 
     *
     * @return  Process id or zero if failed. 
    **/
    ECAL_DEPRECATE_SINCE_6_0("This function is deprecated and will be removed in future eCAL versions.")
    ECAL_API int StartProcess(const char* proc_name_, const char* proc_args_, const char* working_dir_, bool create_console_, eCAL::Process::eStartMode process_mode_, bool block_);

    /**
     * @deprecated This function is deprecated and will be removed in future eCAL versions. 
     * @brief  Stop specified process (windows only). 
     *
     * @param proc_name_  Process name. 
     *
     * @return  True if successful. 
    **/
    ECAL_DEPRECATE_SINCE_6_0("This function is deprecated and will be removed in future eCAL versions.")
    ECAL_API bool StopProcess(const char* proc_name_);

    /**
     * @deprecated This function is deprecated and will be removed in future eCAL versions.
     * @brief  Stop specified process (windows only). 
     *
     * @param proc_id_    Process id. 
     *
     * @return  True if successful. 
    **/
    ECAL_DEPRECATE_SINCE_6_0("This function is deprecated and will be removed in future eCAL versions.")
    ECAL_API bool StopProcess(int proc_id_);
  }
  /** @example process.cpp
  * This is an example how functions from the eCAL::Process namespace may be used.
  */
}

