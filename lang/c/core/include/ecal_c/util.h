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
 * @file   ecal_c/util.h
 * @brief  eCAL utility c interface
**/

#ifndef ecal_c_util_h_included
#define ecal_c_util_h_included

#include <ecal_c/export.h>

#ifdef __cplusplus
extern "C"
{
#endif /*__cplusplus*/

  /**
   * @brief Retrieve eCAL data path.
   *
   *        Checks if a valid eCAL data path is available in the following order:
   *        1. ECAL_DATA environment variable path
   *        2. Local user path (win: Appdata/Local, unix: ~/.ecal)
   *        3. System paths like /etc/ecal, ProgramData/eCAL
   *
   * @return  First directory that is not empty. Returns empty string if no valid directory is found.
   *          The returned null-terminated string needs to be released by eCAL_Free().
  **/
  ECALC_API char* eCAL_Util_GeteCALDataDir();

  /**
   * @brief Returns the path to the eCAL log directory. Searches in following order:
   *
   *        1. Environment variable ECAL_LOG_DIR
   *        2. Environment variable ECAL_DATA (also checking for logs subdirectory)
   *        3. The path provided from the configuration
   *        4. The path where ecal.yaml was loaded from (also checking for logs subdirectory)
   *        5. The temporary directory (e.g. /tmp [unix], Appdata/local/Temp [win])
   *        6. Fallback path /ecal_tmp
   *
   *        In case of 5/6, a unique temporary folder will be created.
   *
   * @returns Path to the eCAL log directory if succeeded. Empty string if no root path could be found.
   *          The returned null-terminated string needs to be released by eCAL_Free().
  **/
  ECALC_API char* eCAL_Util_GeteCALLogDir();

  /**
   * @brief Send shutdown event to specified local user process using it's unit name.
   *
   * @param unit_name_  Process unit name. 
  **/
  ECALC_API void eCAL_Util_ShutdownProcess(const char* unit_name_);

  /**
   * @brief Send shutdown event to specified local user process using it's process id.
   *
   * @param process_id_  Process id. 
  **/
  ECALC_API void eCAL_Util_ShutdownProcessId(int process_id_);

  /**
   * @brief Send shutdown event to all local user processes.
  **/
  ECALC_API void eCAL_Util_ShutdownProcesses();
#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*ecal_c_util_h_included*/
