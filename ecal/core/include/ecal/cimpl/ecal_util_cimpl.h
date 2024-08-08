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
 * @file   ecal_util_cimpl.h
 * @brief  eCAL utility c interface
**/

#ifndef ecal_util_cimpl_h_included
#define ecal_util_cimpl_h_included

#include <ecal/ecal_os.h>

#ifdef __cplusplus
extern "C"
{
#endif /*__cplusplus*/
  /**
   * @brief Send shutdown event to specified local user process using it's unit name.
   *
   * @param unit_name_  Process unit name. 
  **/
  ECALC_API void eCAL_Util_ShutdownUnitName(const char* unit_name_);

  /**
   * @brief Send shutdown event to specified local user process using it's process id.
   *
   * @param process_id_  Process id. 
  **/
  ECALC_API void eCAL_Util_ShutdownProcessID(int process_id_);

  /**
   * @brief Send shutdown event to all local user processes.
  **/
  ECALC_API void eCAL_Util_ShutdownProcesses();

  /**
   * @brief Send shutdown event to all local core components.
  **/
  ECALC_API void eCAL_Util_ShutdownCore();

  /**
   * @brief Enable eCAL message loop back,
   *          that means subscriber will receive messages from
   *          publishers of the same process (default == false).
   *
   * @param state_  Switch on message loop back.. 
  **/
  ECALC_API void eCAL_Util_EnableLoopback(int state_);
#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*ecal_util_cimpl_h_included*/
