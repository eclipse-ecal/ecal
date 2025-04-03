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
 * @file   timer.h
 * @brief  eCAL timer interface
**/

#pragma once

#include <ecal/os.h>
#include <functional>
#include <memory>

namespace eCAL
{
  class CTimerImpl;
  using TimerCallbackT = std::function<void ()>;

  /**
   * @brief eCAL timer class.
   *
   * The CTimer class is used to realize simple time triggered callbacks.
  **/
  class CTimer 
  {
  public:
    /**
     * @brief Constructor. 
    **/
    ECAL_API CTimer();

    /**
     * @brief Constructor. 
     *
     * @param timeout_    Timer callback loop time in ms.
     * @param callback_   The callback function. 
     * @param delay_      Timer callback delay for first call in ms.
    **/
    ECAL_API CTimer(int timeout_, const TimerCallbackT& callback_, int delay_ = 0);

    /**
     * @brief Destructor. 
    **/
    ECAL_API virtual ~CTimer();

    // Object not copyable / moveable
    CTimer(const CTimer&) = delete;
    CTimer& operator=(const CTimer&) = delete;
    CTimer(CTimer&& rhs) = delete;
    CTimer& operator=(CTimer&& rhs) = delete;

    /**
     * @brief Start the timer. 
     *
     * @param timeout_    Timer callback loop time in ms.
     * @param callback_   The callback function. 
     * @param delay_      Timer callback delay for first call in ms.
     *
     * @return  True if timer could be started. 
    **/
    ECAL_API bool Start(int timeout_, const TimerCallbackT& callback_, int delay_ = 0);

    /**
     * @brief Stop the timer. 
     *
     * @return  True if timer could be stopped. 
    **/
    ECAL_API bool Stop();

  protected:
    // class members
    std::unique_ptr<CTimerImpl> m_timer;
  };
}

