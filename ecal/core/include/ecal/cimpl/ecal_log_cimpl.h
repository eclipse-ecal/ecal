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
 * @file   ecal_log_cimpl.h
 * @brief  eCAL logging c interface
**/

#ifndef ecal_log_cimpl_h_included
#define ecal_log_cimpl_h_included

#include <ecal/ecal_log_level.h>

#ifdef __cplusplus
extern "C"
{
#endif /*__cplusplus*/
  /**
   * @brief Sets the log level. 
   *
   * @param level_  The level. 
  **/
  ECALC_API void eCAL_Logging_SetLogLevel(enum eCAL_Logging_eLogLevel level_);

  /**
   * @brief Get the current log level.
   *
   * @return   The current log level.
  **/
  ECALC_API enum eCAL_Logging_eLogLevel eCAL_Logging_GetLogLevel();

  /**
   * @brief Log a message (with current log level). 
   *
   * @param msg_  The log message string.
  **/
  ECALC_API void eCAL_Logging_Log(const char* const msg_);

  /**
   * @brief Mark the start of the user core process. 
  **/
  ECALC_API void eCAL_Logging_StartCoreTimer();

  /**
   * @brief Mark the stop of the user core process. 
  **/
  ECALC_API void eCAL_Logging_StopCoreTimer();

  /**
   * @brief Set the current measured core time in s (for user implemented measuring). 
   *
   * @param time_  The core time.
  **/
  ECALC_API void eCAL_Logging_SetCoreTime(double time_);

  /**
   * @brief Returns the current measured core time in s. 
  **/
  ECALC_API double eCAL_Logging_GetCoreTime();
#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*ecal_log_cimpl_h_included*/
