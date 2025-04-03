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
 * @file   ecal_c/timer.h
 * @brief  eCAL timer c interface
**/

#ifndef ecal_c_timer_h_included
#define ecal_c_timer_h_included

#include <ecal_c/export.h>
#include <ecal_c/types.h>

#ifdef __cplusplus
extern "C"
{
#endif /*__cplusplus*/
  typedef void (*eCAL_TimerCallbackT)(void*);

  typedef struct eCAL_Timer eCAL_Timer;

  /**
   * @brief Creates a new timer instance.
   *
   * @return Timer handle if succeeded, otherwise NULL. The handle needs to be deleted by eCAL_Timer_Delete().
  **/
  ECALC_API eCAL_Timer* eCAL_Timer_New();

  /**
   * @brief Deletes a timer instance.
   *
   * @param timer_  Timer handle.
  **/
  ECALC_API void eCAL_Timer_Delete(eCAL_Timer* timer_);

  /**
   * @brief Start the timer.
   *
   * @param timeout_       Timer callback loop time in ms.
   * @param callback_      The callback function.
   * @param user_argument_ User argument that is forwarded to the callback. Optional, can be NULL.
   * @param delay_         Timer callback delay for first call in ms. Optional, can be NULL.
   *
   * @return Zero if succeed, non-zero otherwise.
  **/
  ECALC_API int eCAL_Timer_Start(eCAL_Timer* timer_, int timeout_, eCAL_TimerCallbackT callback_, void* user_argument_, const int* delay_);

  /**
   * @brief Stop the timer.
   *
   * @return Zero if succeed, non-zero otherwise.
  **/
  ECALC_API int eCAL_Timer_Stop(eCAL_Timer* timer_);
#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*ecal_c_timer_h_included*/
