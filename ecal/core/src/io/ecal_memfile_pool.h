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
 * @brief  memory file pool handler
**/

#pragma once

#include <ecal/ecal.h>

#include "ecal_global_accessors.h"
#include "ecal_def.h"
#include "ecal_memfile.h"

#include <mutex>
#include <atomic>
#include <map>
#include <thread>

namespace eCAL
{
  ////////////////////////////////////////
  // CMemFileObserver
  ////////////////////////////////////////
  class CMemFileObserver
  {
  public:
    CMemFileObserver();
    ~CMemFileObserver();

    void ResetTimeout();
    void Stop();
    bool IsStopped() {return(m_is_stopped);};

    void Observe(const std::string& topic_name_, const std::string& topic_id_, const std::string& memfile_name_, const std::string& memfile_event_, const int timeout_max_);

  protected:
    std::mutex         m_thread_sync;
    std::atomic<bool>  m_do_stop;
    std::atomic<bool>  m_is_stopped;
    std::atomic<int>   m_timeout_read;
    std::atomic<int>   m_timeout_ack;
    EventHandleT       m_event_snd;
    EventHandleT       m_event_ack;
    CMemoryFile        m_memfile;
    std::vector<char>  m_ecal_buffer;
  };

  ////////////////////////////////////////
  // CMemFileThread
  ////////////////////////////////////////
  class CMemFileThread
  {
  public:
    CMemFileThread(const std::string& topic_name_, const std::string& topic_id_, const std::string& memfile_name_, const std::string& memfile_event_, const int timeout_max_);
    ~CMemFileThread();

    bool Stop();
    bool Join();
    bool ResetTimeout();
    bool IsStopped();

  protected:
    std::thread       m_thread;
    CMemFileObserver  m_observer;
    std::string       m_topic_id;
  };

  ////////////////////////////////////////
  // CMemFileThreadPool
  ////////////////////////////////////////
  class CMemFileThreadPool
  {
  public:
    CMemFileThreadPool();
    ~CMemFileThreadPool();

    void Create();
    void Destroy();

    bool AssignThread(const std::string& topic_id_, const std::string& memfile_event_, const std::string& memfile_name_, const std::string& topic_name_);

  protected:
    std::atomic<bool>                       m_created;
    std::mutex                              m_thread_pool_sync;
    std::map<std::string, CMemFileThread*>  m_thread_pool;
  };
}
