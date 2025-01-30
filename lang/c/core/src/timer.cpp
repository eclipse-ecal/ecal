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

#include <mutex>

namespace
{
  std::recursive_mutex g_timer_callback_mtx; // NOLINT(*-avoid-non-const-global-variables)
  void g_timer_callback(const TimerCallbackCT callback_, void* par_)
  {
    const std::lock_guard<std::recursive_mutex> lock(g_timer_callback_mtx);
    callback_(par_);
  }
}

extern "C"
{
  ECALC_API ECAL_HANDLE eCAL_Timer_Create()
  {
    auto* timer = new eCAL::CTimer; // NOLINT(*-owning-memory)
    return(timer);
  }

  ECALC_API int eCAL_Timer_Destroy(ECAL_HANDLE handle_)
  {
    if (handle_ == nullptr) return(0);
    auto* timer = static_cast<eCAL::CTimer*>(handle_);
    delete timer; // NOLINT(*-owning-memory)
    return(1);
  }

  ECALC_API int eCAL_Timer_Start(ECAL_HANDLE handle_, int timeout_, TimerCallbackCT callback_, int delay_, void* par_)
  {
    if (handle_ == nullptr) return(0);
    auto* timer = static_cast<eCAL::CTimer*>(handle_);
    auto callback = std::bind(g_timer_callback, callback_, par_);
    if (timer->Start(timeout_, callback, delay_)) return(1);
    else                                         return(0);
  }

  ECALC_API int eCAL_Timer_Stop(ECAL_HANDLE handle_)
  {
    if (handle_ == nullptr) return(0);
    auto* timer = static_cast<eCAL::CTimer*>(handle_);
    if (timer->Stop()) return(1);
    else              return(0);
  }
}
