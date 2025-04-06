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
 * @file   ecal_c/process.h
 * @brief  eCAL process c interface
**/

#ifndef ecal_c_process_h_included
#define ecal_c_process_h_included

#include <ecal_c/export.h>
#include <ecal_c/process_severity.h>

#ifdef __cplusplus
extern "C"
{
#endif /*__cplusplus*/

  /**
   * @brief  Dump configuration to console. 
  **/
  ECALC_API void eCAL_Process_DumpConfig();

  /**
   * @brief  Dump configiguration into null-terminated string
   *
   * @param[out] configuration_ Returned null-terminated configuration string. Must point to NULL and needs to be released by eCAL_Free(). 
   * 
  **/
  ECALC_API void eCAL_Process_DumpConfigString(char** configuration_);

  /**
   * @brief  Get current host name.
   *
   * @return Host name if suceeded, empty string otherwise.
  **/
  ECALC_API const char* eCAL_Process_GetHostName();

  /**
   * @brief  Get current SHM transport domain.
   *
   * @return SHM transport domain if suceeded, empty string otherwise.
  **/
  ECALC_API const char* eCAL_Process_GetShmTransportDomain();

  /**
   * @brief  Get current unit name (defined via eCAL::Initialize).
   *
   * @return  Unit name if suceeded, empty string otherwise.
  **/
  ECALC_API const char* eCAL_Process_GetUnitName();

  /**
   * @brief  Sleep current thread.
   *
   * Because of the fact that std::this_thread::sleep_for is vulnerable to system clock changes
   * on Windows, Sleep function from synchapi.h had to be used for Windows. This insures time
   * robustness on all platforms from a thread sleep perspective.
   *
   * @param  time_ms_  Time to sleep in ms.
  **/
  ECALC_API void eCAL_Process_SleepMS(long time_ms_);

  /**
   * @brief  Sleep current thread.
   *
   * Because of the fact that std::this_thread::sleep_for is vulnerable to system clock changes
   * on Windows, Sleep function from synchapi.h had to be used for Windows. This insures time
   * robustness on all platforms from a thread sleep perspective. Used with ns unit to obtain bigger precision.
   *
   * @param  time_ns_  Time to sleep in ns.
  **/
  ECALC_API void eCAL_Process_SleepNS(long long time_ns_);

  /**
   * @brief  Get current process id. 
   *
   * @return  The process id. 
  **/
  ECALC_API int eCAL_Process_GetProcessID();

  /**
   * @brief  Get current process id as string.
   *
   * @return  The process id if suceeded, empty string otherwise.
  **/
  ECALC_API const char* eCAL_Process_GetProcessIDAsString();

  /**
   * @brief  Get current process name.
   *
   * @return Process name if suceeded, empty string otherwise.
  **/
  ECALC_API const char* eCAL_Process_GetProcessName();

  /**
   * @brief  Get current process parameter.
   *
   * @return  Process parameter if suceeded, empty string otherwise.
  **/
  ECALC_API const char* eCAL_Process_GetProcessParameter();

  /**
   * @brief  Set process state info. 
   *
   * @param severity_  Severity. 
   * @param level_     Severity level.
   * @param info_      Info message.
   *
  **/
  ECALC_API void eCAL_Process_SetState(enum eCAL_Process_eSeverity severity_, enum eCAL_Process_eSeverityLevel level_, const char* info_);
#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*ecal_c_process_h_included*/
