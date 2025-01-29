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

#ifdef __cplusplus
extern "C"
{
#endif /*__cplusplus*/

  enum eCAL_Process_eStartMode
  {
    proc_smode_normal = 0,  /*!<  0 == start mode normal     */
    proc_smode_hidden = 1,  /*!<  1 == start mode hidden     */
    proc_smode_minimized = 2,  /*!<  2 == start mode minimized  */
    proc_smode_maximized = 3,  /*!<  3 == start mode maximized  */
  };

  /**
 * @brief  Process severity
**/
  enum eCAL_Process_eSeverity
  {
    proc_sev_unknown = 0,  /*!<  0 == condition unknown     */
    proc_sev_healthy = 1,  /*!<  1 == process healthy       */
    proc_sev_warning = 2,  /*!<  2 == process warning level */
    proc_sev_critical = 3,  /*!<  3 == process critical      */
    proc_sev_failed = 4,  /*!<  4 == process failed        */
  };

  /**
   * @brief Process Severity Level
   *
   * enumerations for ECAL_API::SetState functionality
   * where the lowest process severity is generally proc_sev_level1
  **/
  enum eCAL_Process_eSeverity_Level
  {
    proc_sev_level1 = 1,  /*!<  default severity level 1 */
    proc_sev_level2 = 2,  /*!<  severity level 2         */
    proc_sev_level3 = 3,  /*!<  severity level 3         */
    proc_sev_level4 = 4,  /*!<  severity level 4         */
    proc_sev_level5 = 5,  /*!<  severity level 5         */
  };


  /**
   * @brief  Dump configuration to console. 
  **/
  ECALC_API void eCAL_Process_DumpConfig();

  /**
   * @brief  Get current host name. 
   *
   * @param [out] name_      Pointer to store the host name. 
   * @param       name_len_  Length of allocated buffer or ECAL_ALLOCATE_4ME if
   *                         eCAL should allocate the buffer for you (see eCAL_FreeMem). 
   *
   * @return  Buffer length or zero if failed. 
  **/
  ECALC_API int eCAL_Process_GetHostName(void* name_, int name_len_);

  /**
   * @brief  Get process unit name (defined with eCAL_Initialize). 
   *
   * @param [out] name_      Pointer to store the unit name. 
   * @param       name_len_  Length of allocated buffer or ECAL_ALLOCATE_4ME if
   *                         eCAL should allocate the buffer for you (see eCAL_FreeMem). 
   *
   * @return  Buffer length or zero if failed. 
  **/
  ECALC_API int eCAL_Process_GetUnitName(void* name_, int name_len_);

  /**
   * @brief  Get current process parameter (defined via eCAL_Initialize(argc_, arg_v). 
   *
   * @param [out] par_      Pointer to store the process parameter. 
   * @param       par_len_  Length of allocated buffer or ECAL_ALLOCATE_4ME if
   *                        eCAL should allocate the buffer for you (see eCAL_FreeMem). 
   * @param       sep_      Separator. 
   *
   * @return  Buffer length or zero if failed. 
  **/
  ECALC_API int eCAL_Process_GetTaskParameter(void* par_, int par_len_, const char* sep_);

  /**
   * @brief  Sleep current thread. 
   *
   * @param  time_ms_  Time to sleep in ms. 
  **/
  ECALC_API void eCAL_Process_SleepMS(long time_ms_);

  /**
   * @brief  Get current process id. 
   *
   * @return  The process id. 
  **/
  ECALC_API int eCAL_Process_GetProcessID();

  /**
   * @brief  Get current process name. 
   *
   * @param [out] name_      Pointer to store the process name. 
   * @param       name_len_  Length of allocated buffer or ECAL_ALLOCATE_4ME if
   *                         eCAL should allocate the buffer for you (see eCAL_FreeMem). 
   *
   * @return  Process name length or zero if failed. 
  **/
  ECALC_API int eCAL_Process_GetProcessName(void* name_, int name_len_);

  /**
   * @brief  Get current process parameter as string. 
   *
   * @param [out] par_      Pointer to store the process parameter. 
   * @param       par_len_  Length of allocated buffer or ECAL_ALLOCATE_4ME if
   *                        eCAL should allocate the buffer for you (see eCAL_FreeMem). 
   *
   * @return  Process parameter length or zero if failed. 
  **/
  ECALC_API int eCAL_Process_GetProcessParameter(void* par_, int par_len_);

  /**
   * @brief  Set process state info. 
   *
   * @param severity_  Severity. 
   * @param level_     Severity level.
   * @param info_      Info message.
   *
  **/
  ECALC_API void eCAL_Process_SetState(enum eCAL_Process_eSeverity severity_, enum eCAL_Process_eSeverity_Level level_, const char* info_);

  /**
   * @brief  Start specified process (windows only).
   *
   * @param proc_name_       Process name.
   * @param proc_args_       Process argument string.
   * @param working_dir_     Working directory.
   * @param create_console_  Start process in own console window (Windows only).
   * @param process_mode_    Start normal, hidden, minimized, maximized (Windows only).
   * @param block_           Block until process finished.
   *
   * @return  Process id or zero if failed.
  **/
  ECALC_API int eCAL_Process_StartProcess(const char* proc_name_, const char* proc_args_, const char* working_dir_, int create_console_, enum eCAL_Process_eStartMode process_mode_, int block_);

  /**
   * @brief  Stop specified process (windows only).
   *
   * @param proc_name_  Process name.
   *
   * @return  None zero if successful.
  **/
  ECALC_API int eCAL_Process_StopProcessName(const char* proc_name_);

  /**
   * @brief  Stop specified process (windows only).
   *
   * @param proc_id_    Process id.
   *
   * @return  None zero if successful.
  **/
  ECALC_API int eCAL_Process_StopProcessID(int proc_id_);
#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*ecal_c_process_h_included*/
