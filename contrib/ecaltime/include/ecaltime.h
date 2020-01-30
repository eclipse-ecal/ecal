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
 * @brief  eCAL time synchronization interface
**/

#ifndef ecaltime_h_included
#define ecaltime_h_included

#ifdef _MSC_VER
  #ifdef ECAL_TIME_PLUGIN_API_EXPORT
    #define ECALTIME_API __declspec(dllexport)
  #else
    #define ECALTIME_API
  #endif
#else
  #define ECALTIME_API
#endif

#ifdef __cplusplus
extern "C"
{
#endif /*__cplusplus*/
  /**
   * @brief  Initialize time sync interface. 
   *
   * @return  Zero if succeeded. 
  **/
  ECALTIME_API int etime_initialize(void);

  /**
   * @brief  Finalize time sync interface.
   *
   * @return  Zero if succeeded. 
  **/
  ECALTIME_API int etime_finalize(void);

  /**
   * @brief  Get current time
   *
   * @return  current time in ns.
  **/
  ECALTIME_API long long etime_get_nanoseconds();

  /**
   * @brief  Set current time in nano seconds if host is time master.
   *
   * @param time_  Current time in ns.
   *
   * @return  Zero if succeeded non zero otherwise.
  **/
  ECALTIME_API int etime_set_nanoseconds(long long time_);

  /**
   * @brief  Returns time synchronization state. 
   *
   * @return  Non zero if process is time synchronized. 
  **/
  ECALTIME_API int etime_is_synchronized();

  /**
   * @brief  Checks wether this host is time master.
   *
   * @return  Non zero if host is time master.
  **/
  ECALTIME_API int etime_is_master();

  /**
   * @brief Blocks for the given amount of nanoseconds.
   *
   * The actual amount of (real-) time is influenced by the current rate at
   * which the time is proceeding.
   * It is not guaranteed, that the precision of this function actually  is in
   * nanoseconds. Limitations of the operating system might reduce the accuracy.
   *
   * @param duration_nsecs_ the duration in nanoseconds
   */
  ECALTIME_API void etime_sleep_for_nanoseconds(long long duration_nsecs_);

  /**
   * @brief Get the current error code and status message
   *
   * Adapters must return 0 if everything is OK. Each adapter can use it's own
   * set of error codes. Everything non-zero is considered to indicate a
   * problem. Adapters may provide aditional information with a status message.
   *
   * @param error_ [out] the current error code. 0 indicates that everything is OK
   * @param status_message_ [in, out] a pre-allocated char array. The status message will be written to this memory. May be null, if the caller is not interested in the message.
   * @param max_len_ [in] length of the allocated memory
   */
  ECALTIME_API void etime_get_status(int* error_, char* status_message_, int max_len_);

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*ecaltime_h_included*/
