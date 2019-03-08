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
 * @file   ecal_timed_cb.h
 * @brief  eCAL timer callback class (for internal use only !!)
**/

#pragma once

#include <atomic>
#include <chrono>
#include <functional>
#include <thread>
#include <assert.h>

namespace eCAL
{
  class CTimedCB;

  /**
   * @brief Timer callback function type.
  **/
  typedef std::function<void(void)> TimerCallbackT;

  /**
   * @brief eCAL timer callback class.
   *
   * The CTimedCB class is used to realize simple time triggered callbacks.
  **/
  class CTimedCB
  {
  public:
    /**
     * @brief Constructor.
    **/
    CTimedCB() : m_stop(false), m_running(false) {}

    /**
     * @brief Constructor.
     *
     * @param timeout_    Timer callback loop time in ms.
     * @param callback_   The callback function.
     * @param delay_      Timer callback delay for first call in ms.
    **/
    CTimedCB(int timeout_, TimerCallbackT callback_, int delay_ = 0) : m_stop(false), m_running(false) { Start(timeout_, callback_, delay_); }

    /**
     * @brief Destructor.
    **/
    virtual ~CTimedCB() { Stop(); }

    /**
     * @brief Start the timer.
     *
     * @param timeout_    Timer callback loop time in ms.
     * @param callback_   The callback function.
     * @param delay_      Timer callback delay for first call in ms.
     *
     * @return  True if timer could be started.
    **/
    bool Start(const int timeout_, TimerCallbackT callback_, const int delay_ = 0)
    {
      assert(m_running == false);
      if (m_running)    return(false);
      if (timeout_ < 0) return(false);
      m_stop = false;
      m_thread = std::thread(&CTimedCB::Thread, this, callback_, timeout_, delay_);
      m_running = true;
      return(true);
    }

    /**
     * @brief Stop the timer.
     *
     * @return  True if timer could be stopped.
    **/
    bool Stop()
    {
      if (!m_running) return(false);
      m_stop = true;
      m_thread.join();
      m_running = false;
      return(true);
    }

  private:
    // this object must not be copied.
    CTimedCB(const CTimedCB&);
    CTimedCB& operator=(const CTimedCB&);

    void Thread(TimerCallbackT callback_, int timeout_, int delay_)
    {
      assert(callback_ != nullptr);
      if (callback_ == nullptr) return;
      if (delay_ > 0) std::this_thread::sleep_for(std::chrono::milliseconds(delay_));
      while (!m_stop)
      {
        auto start = std::chrono::steady_clock::now();
        (callback_)();
        if (timeout_ > 0) std::this_thread::sleep_until(start + std::chrono::milliseconds(timeout_));
      }
      m_stop = false;
    }

    std::atomic<bool> m_stop;
    std::atomic<bool> m_running;
    std::thread       m_thread;
  };
}
