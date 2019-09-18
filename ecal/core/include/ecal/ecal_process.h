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
 * @file   ecal_process.h
 * @brief  eCAL process interface
**/

#pragma once

#include <string>

#include <ecal/ecal_os.h>
#include <ecal/ecal_process_mode.h>
#include <ecal/ecal_process_severity.h>
#include <ecal/ecal_callback.h>

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
     * @brief  Get current unit name (defined via eCAL::Initialize). 
     *
     * @return  Unit name or empty string if failed. 
    **/
    ECAL_API std::string GetUnitName();

    /**
     * @brief  Get current process parameter (defined via eCAL::Initialize(argc_, arg_v). 
     *
     * @param sep_  Separator. 
     *
     * @return  Task parameter separated by 'sep_' or zero string if failed. 
    **/
    ECAL_API std::string GetTaskParameter(const char* sep_);

    /**
     * @brief  Sleep current thread. 
     *
     * @param  time_ms_  Time to sleep in ms. 
    **/
    ECAL_API void SleepMS(long time_ms_);

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
     * @brief  Get CPU usage of current process. 
     *
     * @return  The CPU usage in percent. 
    **/
    ECAL_API float GetProcessCpuUsage();

    /**
     * @brief  Get memory usage of current process. 
     *
     * @return  The memory usage in bytes. 
    **/
    ECAL_API unsigned long GetProcessMemory();

    /**
     * @deprecated  Use the function GetWClock() instead
    **/
    [[deprecated("use GetWClock() instead")]]
    ECAL_API long long GetSClock();

    /**
     * @deprecated  Use the function GetWBytes() instead
    **/

    [[deprecated("use GetWBytes() instead")]]
    ECAL_API long long GetSBytes();

    /**
     * @brief  Get the write clock of the current process. 
     *
     * @return  The message write count per second. 
    **/
    ECAL_API long long GetWClock();

    /**
     * @brief  Get the write bytes of the current process. 
     *
     * @return  The message write bytes per second. 
    **/
    ECAL_API long long GetWBytes();

    /**
     * @brief  Get the read clock of the current process. 
     *
     * @return  The message read count per second. 
    **/
    ECAL_API long long GetRClock();

    /**
     * @brief  Get the read bytes of the current process. 
     *
     * @return  The message read bytes per second. 
    **/
    ECAL_API long long GetRBytes();

    /**
     * @brief  Set process state info. 
     *
     * @param severity_  Severity. 
     * @param level_     Severity level.
     * @param info_      Info message.
     *
    **/
    ECAL_API void SetState(eCAL_Process_eSeverity severity_, eCAL_Process_eSeverity_Level level_, const char* info_);

    /**
     * @brief  Add registration callback.
     *
     * @param event_          The type of registration.
     * @param callback_       The callback.
     *
     * @return  Zero if succeeded.
    **/
    ECAL_API int AddRegistrationCallback(enum eCAL_Registration_Event event_, RegistrationCallbackT callback_);

    /**
    * @brief  Remove registration callback.
    *
    * @param event_          The type of registration.
    *
    * @return  Zero if succeeded.
    **/
    ECAL_API int RemRegistrationCallback(enum eCAL_Registration_Event event_);

    /**
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
    ECAL_API int StartProcess(const char* proc_name_, const char* proc_args_, const char* working_dir_, bool create_console_, eCAL_Process_eStartMode process_mode_, bool block_);

    /**
     * @brief  Stop specified process (windows only). 
     *
     * @param proc_name_  Process name. 
     *
     * @return  True if successful. 
    **/
    ECAL_API bool StopProcess(const char* proc_name_);

    /**
     * @brief  Stop specified process (windows only). 
     *
     * @param proc_id_    Process id. 
     *
     * @return  True if successful. 
    **/
    ECAL_API bool StopProcess(int proc_id_);
  }
  /** @example process.cpp
  * This is an example how functions from the eCAL::Process namespace may be used.
  */
}

#ifdef ECAL_C_DLL

#include <ecal/ecalc.h>

namespace eCAL
{
  namespace Process
  {
    inline void DumpConfig()
    {
      eCAL_Process_DumpConfig();
    }

    inline std::string GetHostName()
    {
      void* buf = nullptr;
      std::string name;
      size_t buf_len = eCAL_Process_GetHostName(&buf, ECAL_ALLOCATE_4ME);
      if(buf_len > 0)
      {
        name = std::string(static_cast<char*>(buf), buf_len);
        eCAL_FreeMem(buf);
      }
      return(name);
    }

    inline std::string GetUnitName()
    {
      void* buf = nullptr;
      std::string name;
      size_t buf_len = eCAL_Process_GetUnitName(&buf, ECAL_ALLOCATE_4ME);
      if(buf_len > 0)
      {
        name = std::string(static_cast<char*>(buf), buf_len);
        eCAL_FreeMem(buf);
      }
      return(name);
    }

    inline std::string GetTaskParameter(const char* sep_)
    {
      void* buf = nullptr;
      std::string par;
      size_t buf_len = eCAL_Process_GetTaskParameter(&buf, ECAL_ALLOCATE_4ME, sep_);
      if(buf_len > 0)
      {
        par = std::string(static_cast<char*>(buf), buf_len);
        eCAL_FreeMem(buf);
      }
      return(par);
    }

    inline void SleepMS(long time_ms_)
    {
      eCAL_Process_SleepMS(time_ms_);
    }

    inline int GetProcessID()
    {
      return(eCAL_Process_GetProcessID());
    }

    inline std::string GetProcessName()
    {
      void* buf = nullptr;
      std::string name;
      size_t buf_len = eCAL_Process_GetProcessName(&buf, ECAL_ALLOCATE_4ME);
      if(buf_len > 0)
      {
        name = std::string(static_cast<char*>(buf), buf_len);
        eCAL_FreeMem(buf);
      }
      return(name);
    }

    inline std::string GetProcessParameter()
    {
      void* buf = nullptr;
      std::string par;
      size_t buf_len = eCAL_Process_GetProcessParameter(&buf, ECAL_ALLOCATE_4ME);
      if(buf_len > 0)
      {
        par = std::string(static_cast<char*>(buf), buf_len);
        eCAL_FreeMem(buf);
      }
      return(par);
    }

    inline float GetProcessCpuUsage()
    {
      return(eCAL_Process_GetProcessCpuUsage());
    }

    inline unsigned long GetProcessMemory()
    {
      return(eCAL_Process_GetProcessMemory());
    }
    
    [[deprecated("use GetWClock() instead")]]
    inline long long GetSClock()
    {
      return(eCAL_Process_GetWClock());
    }

    [[deprecated("use GetWBytes() instead")]]
    inline long long GetSBytes()
    {
      return(eCAL_Process_GetWBytes());
    }

    inline long long GetWClock()
    {
      return(eCAL_Process_GetWClock());
    }

    inline long long GetWBytes()
    {
      return(eCAL_Process_GetWBytes());
    }

    inline long long GetRClock()
    {
      return(eCAL_Process_GetRClock());
    }

    inline long long GetRBytes()
    {
      return(eCAL_Process_GetRBytes());
    }

    inline void SetState(eCAL_Process_eSeverity severity_, eCAL_Process_eSeverity_Level level_, const char* info_)
    {
      eCAL_Process_SetState(severity_, level_, info_);
    }

    inline int StartProcess(const char* proc_name_, const char* proc_args_, const char* working_dir_, bool create_console_, eCAL_Process_eStartMode process_mode_, bool block_)
    {
      return(eCAL_Process_StartProcess(proc_name_, proc_args_, working_dir_, create_console_, process_mode_, block_));
    }

    inline bool StopProcess(const char* proc_name_)
    {
      return(eCAL_Process_StopProcessName(proc_name_) != 0);
    }

    inline bool StopProcess(int proc_id_)
    {
      return(eCAL_Process_StopProcessID(proc_id_) != 0);
    }
  }
}

#endif /* ECAL_C_DLL */
