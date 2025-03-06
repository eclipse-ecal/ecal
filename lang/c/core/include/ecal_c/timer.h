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
  typedef void (*eCAL_TimerCallbackT)();

  typedef struct eCAL_Timer eCAL_Timer;

  ECALC_API eCAL_Timer* eCAL_Timer_New();
  ECALC_API eCAL_Timer* eCAL_Timer_New2(int timeout_, eCAL_TimerCallbackT callback_, const int* delay_);
  ECALC_API void eCAL_Timer_Delete(eCAL_Timer* timer_);

  ECALC_API int eCAL_Timer_Start(eCAL_Timer* timer_, int timeout_, eCAL_TimerCallbackT callback_, const int* delay_);

  ECALC_API int eCAL_Timer_Stop(eCAL_Timer* timer_);
#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*ecal_c_timer_h_included*/
