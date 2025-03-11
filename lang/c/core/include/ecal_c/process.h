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

  ECALC_API void eCAL_Process_DumpConfig2(char** configuration_);


  ECALC_API const char* eCAL_Process_GetHostName();

  ECALC_API const char* eCAL_Process_GetShmTransportDomain();

  ECALC_API const char* eCAL_Process_GetUnitName();

  ECALC_API void eCAL_Process_SleepMS(long time_ms_);

  ECALC_API void eCAL_Process_SleepNS(long long time_ns_);

  /**
   * @brief  Get current process id. 
   *
   * @return  The process id. 
  **/
  ECALC_API int eCAL_Process_GetProcessID();

  ECALC_API const char* eCAL_Process_GetProcessIDAsString();

  ECALC_API const char* eCAL_Process_GetProcessName();

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
