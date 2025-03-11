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
 * @file   timer.cpp
 * @brief  eCAL timer c interface
**/

#include <ecal/ecal.h>
#include <ecal_c/timer.h>

extern "C"
{
  struct eCAL_Timer
  {
    eCAL::CTimer* handle;
  };

  ECALC_API eCAL_Timer* eCAL_Timer_New()
  {
    return new eCAL_Timer{ new eCAL::CTimer() };
  }

  ECALC_API eCAL_Timer* eCAL_Timer_New2(int timeout_, eCAL_TimerCallbackT callback_, const int* delay_)
  {
    auto callback = [callback_]()
    {
      callback_();
    };
    return new eCAL_Timer{ new eCAL::CTimer(timeout_, callback, (delay_ != NULL ) ? *delay_ : 0) };
  }

  ECALC_API void eCAL_Timer_Delete(eCAL_Timer* timer_)
  {
    delete timer_->handle;
    delete timer_;
  }

  ECALC_API int eCAL_Timer_Start(eCAL_Timer* timer_, int timeout_, eCAL_TimerCallbackT callback_, const int* delay_)
  {
    auto callback = [callback_]()
    {
      callback_();
    };
    return !static_cast<int>(timer_->handle->Start(timeout_, callback, (delay_ != NULL) ? *delay_ : 0));
  }

  ECALC_API int eCAL_Timer_Stop(eCAL_Timer* timer_)
  {
    return !static_cast<int>(timer_->handle->Stop());
  }
}
