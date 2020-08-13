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
    CTimerImpl() : m_stop(false), m_running(false), m_last_error(0) {}

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
      m_last_error = std::chrono::nanoseconds(0);

      while (!m_stop)
      {
        // Execute callback and measure it's execution time
        auto start = eCAL::Time::ecal_clock::now();
        (callback_)();
        auto end = eCAL::Time::ecal_clock::now();

        // If the time jumped backwards, we do not know how long the function took to complete. 
        // Thus, we will assume it was "fast" and wait for the maximum time.
        if (end < start)
        {
          eCAL::Time::sleep_for(loop_duration);
        }
        else
        {
          // Correct loop duration by last duration error
          auto loop_duration_corr = loop_duration - m_last_error;
          auto sleep_remaining    = loop_duration_corr - (end - start);
#if _WIN32
          // Sleep for the remaining time in multiple loops of smaller sleeps

          // Minimum sleep resolution
          // Lower values will increase the precision but also the CPU usage !
          const auto sleep_resolution_min = std::chrono::microseconds(1);

          // Sleep threshold for switching to minimal sleep intervalls
          const auto sleep_precision_thr  = std::chrono::milliseconds(5);

          while (sleep_remaining.count() > 0)
          {
            // We start with half of the remaining time
            sleep_remaining = sleep_remaining / 2;
            
            // If we reach the system clock sleep precision
            // we reduce the sleep time to the minimum resolution
            if (sleep_remaining < sleep_precision_thr) sleep_remaining = sleep_resolution_min;

            // Sleep and recalculate the remaining time
            eCAL::Time::sleep_for(sleep_remaining);
            sleep_remaining = loop_duration_corr - (eCAL::Time::ecal_clock::now() - start);
          }
#else // _WIN32
          // Sleep for the remaining time in one sleep
          eCAL::Time::sleep_for(sleep_remaining);
#endif //_WIN32

          // Store loop error to correct the next timer run
          m_last_error = (eCAL::Time::ecal_clock::now() - start) - loop_duration_corr;
        }
      }
      m_stop = false;
    }

    std::atomic<bool>        m_stop;
    std::atomic<bool>        m_running;
    std::thread              m_thread;
    std::chrono::nanoseconds m_last_error;
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
