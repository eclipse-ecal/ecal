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
 * @file   process.cpp
 * @brief  eCAL process c interface
**/

#include <ecal/ecal.h>
#include <ecal_c/process.h>

#include "common.h"

extern "C"
{
  ECALC_API void eCAL_Process_DumpConfig()
  {
    eCAL::Process::DumpConfig();
  }

  ECALC_API int eCAL_Process_GetHostName(void* name_, int name_len_)
  {
    const std::string name = eCAL::Process::GetHostName();
    if (!name.empty())
    {
      return(CopyBuffer(name_, name_len_, name));
    }
    return(0);
  }

  ECALC_API int eCAL_Process_GetUnitName(void* name_, int name_len_)
  {
    const std::string name = eCAL::Process::GetUnitName();
    if (!name.empty())
    {
      return(CopyBuffer(name_, name_len_, name));
    }
    return(0);
  }

  ECALC_API int eCAL_Process_GetTaskParameter(void* par_, int par_len_, const char* sep_)
  {
    const std::string par = eCAL::Process::GetTaskParameter(sep_);
    if (!par.empty())
    {
      return(CopyBuffer(par_, par_len_, par));
    }
    return(0);
  }

  ECALC_API void eCAL_Process_SleepMS(long time_ms_)
  {
    eCAL::Process::SleepMS(time_ms_);
  }

  ECALC_API int eCAL_Process_GetProcessID()
  {
    return(eCAL::Process::GetProcessID());
  }

  ECALC_API int eCAL_Process_GetProcessName(void* name_, int name_len_)
  {
    const std::string name = eCAL::Process::GetProcessName();
    if (!name.empty())
    {
      return(CopyBuffer(name_, name_len_, name));
    }
    return(0);
  }

  ECALC_API int eCAL_Process_GetProcessParameter(void* par_, int par_len_)
  {
    const std::string par = eCAL::Process::GetProcessParameter();
    if (!par.empty())
    {
      return(CopyBuffer(par_, par_len_, par));
    }
    return(0);
  }

  ECALC_API void eCAL_Process_SetState(enum eCAL_Process_eSeverity severity_, enum eCAL_Process_eSeverity_Level level_, const char* info_)
  {
    // This is potentially dangerous, need to keep enums in sync.
    eCAL::Process::SetState(static_cast<eCAL::Process::eSeverity>(severity_), static_cast<eCAL::Process::eSeverityLevel>(level_), info_);
  }

  ECALC_API int eCAL_Process_StartProcess(const char* proc_name_, const char* proc_args_, const char* working_dir_, int create_console_, enum eCAL_Process_eStartMode process_mode_, int block_)
  {
    return(eCAL::Process::StartProcess(proc_name_, proc_args_, working_dir_, create_console_ != 0, static_cast<eCAL::Process::eStartMode>(process_mode_), block_ != 0));
  }

  ECALC_API int eCAL_Process_StopProcessName(const char* proc_name_)
  {
    return static_cast<int>(eCAL::Process::StopProcess(proc_name_));
  }

  ECALC_API int eCAL_Process_StopProcessID(int proc_id_)
  {
    return static_cast<int>(eCAL::Process::StopProcess(proc_id_));
  }
}
