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

#include <ecal_c/callback.h>

#ifdef __cplusplus
extern "C"
{
#endif /*__cplusplus*/
  /**
   * @brief Create a timer. 
   *
   * @return  Handle to created timer or NULL if failed.
  **/
  ECALC_API ECAL_HANDLE eCAL_Timer_Create();

  /**
   * @brief Create a timer. 
   *
   * @param handle_  Timer handle. 
   *
   * @return  Handle to created timer or NULL if failed.
  **/
  ECALC_API int eCAL_Timer_Destroy(ECAL_HANDLE handle_);

  /**
   * @brief Start the timer. 
   *
   * @param handle_    Timer handle. 
   * @param timeout_   Timer callback loop time in ms.
   * @param callback_  The callback function. 
   * @param delay_     Timer callback delay for first call in ms.
   * @param par_       User defined context that will be forwarded to the callback function.  
   *
   * @return  None zero if succeeded.
  **/
  ECALC_API int eCAL_Timer_Start(ECAL_HANDLE handle_, int timeout_, TimerCallbackCT callback_, int delay_, void* par_);

  /**
   * @brief Stop the timer. 
   *
   * @param handle_  Timer handle. 
   *
   * @return  None zero if succeeded.
  **/
  ECALC_API int eCAL_Timer_Stop(ECAL_HANDLE handle_);
#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*ecal_c_timer_h_included*/
