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
 * @file   ecal_c/time.h
 * @brief  eCAL time c interface
**/

#ifndef ecal_c_time_h_included
#define ecal_c_time_h_included

#include <ecal_c/export.h>

#ifdef __cplusplus
extern "C"
{
#endif /*__cplusplus*/

  /**
   * @brief  Get interface name.
   *
   * @return  Get time sync interface name.
  **/
  ECALC_API int eCAL_Time_GetName(void* name_, int name_len_);

  /**
   * @brief  Get current time
   *
   * @return  current time in us.
  **/
  ECALC_API long long eCAL_Time_GetMicroSeconds();

  /**
   * @brief  Get current time
   *
   * @return  current time in ns.
  **/
  ECALC_API long long eCAL_Time_GetNanoSeconds();

  /**
   * @brief  Set current time in nano seconds if host is time master.
   *
   * @param time_  Current time in ns.
   *
   * @return  Zero if succeeded non zero otherwise.
  **/
  ECALC_API int eCAL_Time_SetNanoSeconds(long long time_);

  /**
   * @brief  Returns time synchronization state.
   *
   * @return  Non zero if process is time synchronized.
  **/
  ECALC_API int eCAL_Time_IsTimeSynchronized();

  /**
   * @brief  Checks whether this host is time master.
   *
   * @return  Non zero if host is time master.
  **/
  ECALC_API int eCAL_Time_IsTimeMaster();
  
  /**
   * @brief Blocks for the given amount of nanoseconds.
   *
   * The actual amount of (real-) time is influenced by the current rate at
   * which the time is proceeding.
   * It is not guaranteed, that the precision of this function actually  is in
   * nanoseconds. Limitations of the operating system might reduce the accuracy.
   *
   * @param duration_nsecs_ the duration in nanoseconds
  **/
  ECALC_API void eCAL_Time_SleepForNanoseconds(long long duration_nsecs_);

  /**
   * @brief Get the current error code and status message
   *
   * An error code of 0 is considered to be OK. Any other error code is
   * considered to indicate a problem. Time Adapters may use a set of error
   * codes to indicate specific problems.
   * The status message may be a nullpointer. If the user is not interested in
   * the status message, max_len_ has to be < 0.
   * If max_len_ is ECAL_ALLOCATE_4ME, memory will be allocated. The calling
   * function has to free it.
   *
   * @param error_ [out] The error code
   * @param status_message_ [out] A pointer to a char array for the status message. The message will NOT be null-terminated.
   * @param max_len_ [in] The length of the allocated memory for the status_message_, ECAL_ALOCATE_4ME if the function has to allocate memory or -1 if not interested in the message.
   * @return
  **/
  ECALC_API int eCAL_Time_GetStatus(int* error_, char** status_message_, const int max_len_);
  
#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*ecal_c_time_h_included*/
