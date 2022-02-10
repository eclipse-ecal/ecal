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
 * @file   ecal_timer.h
 * @brief  eCAL timer interface
**/

#pragma once

#include <ecal/ecal_os.h>
#include <functional>

namespace eCAL
{
  class CTimerImpl;
  typedef std::function<void(void)> TimerCallbackT;

  /**
   * @brief eCAL timer class.
   *
   * The CTimer class is used to realize simple time triggered callbacks.
  **/
  class ECAL_API CTimer 
  {
  public:
    /**
     * @brief Constructor. 
    **/
    CTimer();

    /**
     * @brief Constructor. 
     *
     * @param timeout_    Timer callback loop time in ms.
     * @param callback_   The callback function. 
     * @param delay_      Timer callback delay for first call in ms.
    **/
    CTimer(int timeout_, TimerCallbackT callback_, int delay_ = 0);

    /**
     * @brief Destructor. 
    **/
    virtual ~CTimer();

    /**
     * @brief Start the timer. 
     *
     * @param timeout_    Timer callback loop time in ms.
     * @param callback_   The callback function. 
     * @param delay_      Timer callback delay for first call in ms.
     *
     * @return  True if timer could be started. 
    **/
    bool Start(int timeout_, TimerCallbackT callback_, int delay_ = 0);

    /**
     * @brief Stop the timer. 
     *
     * @return  True if timer could be stopped. 
    **/
    bool Stop();

  protected:
    // class members
    CTimerImpl*  m_timer;

  private:
    // this object must not be copied.
    CTimer(const CTimer&);
    CTimer& operator=(const CTimer&);
  };
}
