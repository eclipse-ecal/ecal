/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2024 Continental Corporation
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
 * @brief  memory file pool handler
**/

#pragma once

#include <chrono>
#include <cstddef>
#include <ecal/log.h>

#include "ecal_event.h"
#include "ecal_memfile.h"
#include "ecal_memfile_header.h"

#include <atomic>
#include <condition_variable>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

namespace eCAL
{
  using MemFileDataCallbackT = std::function<size_t (const char *, size_t, long long, long long, long long, size_t)>;

  ////////////////////////////////////////
  // CMemFileObserver
  ////////////////////////////////////////
  class CMemFileObserver
  {
  public:
    CMemFileObserver();
    ~CMemFileObserver();

    CMemFileObserver(const CMemFileObserver&) = delete;
    CMemFileObserver& operator=(const CMemFileObserver&) = delete;
    CMemFileObserver(CMemFileObserver&& rhs) = delete;
    CMemFileObserver& operator=(CMemFileObserver&& rhs) = delete;

    bool Create(const std::string& memfile_name_, const std::string& memfile_event_);
    bool Destroy();

    bool Start(int timeout_, const MemFileDataCallbackT& callback_);
    bool Stop();
    bool IsObserving() {return(m_is_observing);};

    bool ResetTimeout();

  protected:
    void Observe(int timeout_);
    bool ReadFileHeader(SMemFileHeader& memfile_hdr);

    std::atomic<bool>       m_created;
    std::atomic<bool>       m_do_stop;
    std::atomic<bool>       m_is_observing;

    std::atomic<std::chrono::steady_clock::time_point> m_time_of_last_life_signal;

    MemFileDataCallbackT    m_data_callback;

    std::thread             m_thread;
    EventHandleT            m_event_snd;
    EventHandleT            m_event_ack;
    CMemoryFile             m_memfile;
  };

  ////////////////////////////////////////
  // CMemFileThreadPool
  ////////////////////////////////////////
  class CMemFileThreadPool
  {
  public:
    CMemFileThreadPool();
    ~CMemFileThreadPool();

    void Start();
    void Stop();

    bool ObserveFile(const std::string& memfile_name_, const std::string& memfile_event_, int timeout_observation_ms, const MemFileDataCallbackT& callback_);

  protected:
    void CleanupPoolThread();
    void CleanupPool();

    std::atomic<bool>                                         m_created;
    std::mutex                                                m_observer_pool_sync;
    std::map<std::string, std::shared_ptr<CMemFileObserver>>  m_observer_pool;

    std::atomic<bool>                                         m_do_cleanup;
    std::condition_variable                                   m_do_cleanup_cv;
    std::mutex                                                m_do_cleanup_mtx;
    std::thread                                               m_cleanup_thread;
  };
}
