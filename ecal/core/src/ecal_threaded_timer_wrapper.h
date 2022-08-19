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
 * @brief  This class is a template class for creating a separate thread that calls a functor
 *         until it is timed out or stopped.
 *         The thread can be kept alive by cyclicly calling `KeepAlive`
 *         The thread can be stopped by calling `Destroy`,
**/

#pragma once

#include <memory>
#include <chrono>
#include <mutex>
#include <atomic>
#include <thread>
#include <cassert>

  template <typename T>
  class CThreadedTimerWrapper
  {
  public:
    static std::weak_ptr<CThreadedTimerWrapper<T>> CreateCThreadedTimerWrapper(std::chrono::milliseconds timeout, std::shared_ptr<T> executor)
    {
      std::shared_ptr<CThreadedTimerWrapper<T>> that(new CThreadedTimerWrapper<T>(timeout, executor));
      //auto that = std::make_shared<CThreadedTimerWrapper>(timeout);
      that->m_self = that;
      return that;
    }

    ~CThreadedTimerWrapper()
    {
      // Join or detach the old thread. We cannot join a thread from it's own
      // thread, so we detach the thread in that case.
      if (std::this_thread::get_id() == m_thread.get_id())
        m_thread.detach();
      else
        m_thread.join();
    }

    CThreadedTimerWrapper ( const CThreadedTimerWrapper& ) = delete;
    CThreadedTimerWrapper ( CThreadedTimerWrapper && ) = delete;	
    CThreadedTimerWrapper& operator= ( const CThreadedTimerWrapper& ) = delete; 
    CThreadedTimerWrapper& operator= ( CThreadedTimerWrapper && ) = delete;	


    void KeepAlive()
    {
      std::lock_guard<std::mutex> lock(m_last_accessed_mutex);
      m_last_accessed = std::chrono::steady_clock::now();
    }

    void Destroy()
    {
      m_stop = true;
    }


  protected:
    CThreadedTimerWrapper(std::chrono::milliseconds timeout, std::shared_ptr<T> executor)
      : m_last_accessed(std::chrono::steady_clock::now())
      , m_stop (false)
      , m_timeout(timeout)
      , m_executor(executor)
    {
      m_thread = std::thread(&CThreadedTimerWrapper::Process, this);
    }
      
    void Process()
    {
      while (!m_stop)
      {
        (*m_executor)();
        if (TimedOut())
          m_stop = true;  
      }

      // cannot join thread... thats a bummer -> detach the thread?

      m_self.reset();
    }

    bool TimedOut()
    {
      std::lock_guard<std::mutex> lock(m_last_accessed_mutex);
      return  std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - m_last_accessed) > m_timeout;
    }

    std::shared_ptr<CThreadedTimerWrapper> m_self;
    std::thread m_thread;
    std::mutex m_last_accessed_mutex;
    std::chrono::steady_clock::time_point m_last_accessed;
    std::atomic<bool> m_stop;
    std::chrono::milliseconds m_timeout;
    std::shared_ptr<T> m_executor;
  };