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
 * @brief  eCAL timer callback class (based on eCAL::Time interface)
**/

#include <ecal/ecal.h>

#include <atomic>
#include <chrono>
#include <thread>
#include <assert.h>

namespace eCAL
{
  class CTimerImpl
  {
  public:
    CTimerImpl() : m_stop(false), m_running(false) {}

    CTimerImpl(const int timeout_, TimerCallbackT callback_, const int delay_) : m_stop(false), m_running(false) { Start(timeout_, callback_, delay_); }

    virtual ~CTimerImpl() { Stop(); }

    bool Start(const int timeout_, TimerCallbackT callback_, const int delay_)
    {
      assert(m_running == false);
      if(m_running)    return(false);
      if(timeout_ < 0) return(false);
      m_stop = false;
      m_thread = std::thread(&CTimerImpl::Thread, this, callback_, timeout_, delay_);
      m_running = true;
      return(true);
    }

    bool Stop()
    {
      if(!m_running) return(false);
      m_stop = true;
      m_thread.join();
      m_running = false;
      return(true);
    }

  private:
    void Thread(TimerCallbackT callback_, int timeout_, int delay_)
    {
      assert(callback_ != nullptr);
      if (callback_ == nullptr) return;
      if (delay_ > 0) eCAL::Time::sleep_for(std::chrono::milliseconds(delay_));

      std::chrono::nanoseconds loop_duration((long long)timeout_ * 1000LL * 1000LL);

      while (!m_stop)
      {
        auto start = eCAL::Time::ecal_clock::now();
        (callback_)();
        auto end = eCAL::Time::ecal_clock::now();

        if (end < start) {
          // If the time jumped backwards, we do not know how long the function took to complete. Thus, we will assume it was "fast" and wait for the maximum time.
          eCAL::Time::sleep_for(loop_duration);
        }
        else {
          // Sleep for the remaining time
          auto sleep_duration = loop_duration - (end - start);
          eCAL::Time::sleep_for(sleep_duration);
        }
      }
      m_stop = false;
    }

    std::atomic<bool> m_stop;
    std::atomic<bool> m_running;
    std::thread       m_thread;
  };


  CTimer::CTimer() : m_timer(nullptr)
  {
    m_timer = new CTimerImpl();
  }

  CTimer::CTimer(const int timeout_, TimerCallbackT callback_, const int delay_ /*= 0*/) : m_timer(nullptr)
  { 
    m_timer = new CTimerImpl();
    m_timer->Start(timeout_, callback_, delay_);
  }

  CTimer::~CTimer()
  {
    Stop();
    delete m_timer;
  }

  bool CTimer::Start(const int timeout_, TimerCallbackT callback_, const int delay_ /*= 0*/)
  {
    return(m_timer->Start(timeout_, callback_, delay_));
  }

  bool CTimer::Stop()
  {
    return(m_timer->Stop());
  }
}
