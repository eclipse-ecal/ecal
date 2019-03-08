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

#include <ecal/ecal.h>

#include "ecal_def.h"
#include "ecal_message.h"
#include "pubsub/ecal_subgate.h"

#include "ecal_memfile_pool.h"

#include <iostream>

namespace eCAL
{
  ////////////////////////////////////////
  // CMemFileObserver
  ////////////////////////////////////////
  CMemFileObserver::CMemFileObserver() :
    m_do_stop(false),
    m_is_stopped(false),
    m_timeout(0)
  {
  }

  CMemFileObserver::~CMemFileObserver()
  {
  }

  void CMemFileObserver::ResetTimeout()
  {
    m_timeout = 0;
  }

  void CMemFileObserver::Stop()
  {
    if(m_is_stopped) return;

    // signal to stop and
    m_do_stop = true;

    // release sync event
    gSetEvent(m_event_snd);
  }

  void CMemFileObserver::Observe(const std::string& topic_name_, const std::string& memfile_name_, const std::string& memfile_event_, const int timeout_max_)
  {
#ifndef NDEBUG
    // log it
    Logging::Log(log_level_debug2, std::string(topic_name_ + "::MemFile Thread Started (" + memfile_name_ + ", " + memfile_event_ + ")"));
#endif
    // open memory file event
    gOpenEvent(&m_event_snd, memfile_event_);
    gOpenEvent(&m_event_ack, memfile_event_ + "_ack");

    // create memory file
    CMemoryFile memfile;
    memfile.Create(memfile_name_.c_str(), false);

    uint64_t sample_clock = 0;
    while((m_timeout < timeout_max_) && !m_do_stop)
    {
      // central memory file event sync with 5 ms
      const int evt_timeout = 5;
      if(gWaitForEvent(m_event_snd, evt_timeout))
      {
        std::lock_guard<std::mutex> lock(m_thread_sync);
        if(m_do_stop) break;

        // try to open memory file with timeout 5 ms
        if(memfile.Open(5))
        {
          // read memory file header
          SEcalMessage ecal_message;

          // retrieve size of received buffer
          size_t data_size = memfile.DataSize();

          // if there are less data then size of the header struct, return false
          if(data_size >= sizeof(SEcalMessage))
          {
            // read header from memory buffer
            memfile.Read(&ecal_message, sizeof(SEcalMessage), 0);
          }

          // read memory file content
          if(ecal_message.data_size > 0)
          {
            m_ecal_buffer.resize((size_t)ecal_message.data_size);
            memfile.Read(m_ecal_buffer.data(), (size_t)ecal_message.data_size, ecal_message.hdr_size);
          }

          // close memory file
          memfile.Close();

          // send ack event
          gSetEvent(m_event_ack);

          // process content
          if((m_ecal_buffer.size() > 0) && (ecal_message.clock > sample_clock))
          {
            // store clock
            sample_clock = ecal_message.clock;
#ifndef NDEBUG
            // log it
            Logging::Log(log_level_debug3, std::string(topic_name_ + "::MemFile Read (" + std::to_string(m_ecal_buffer.size()) + " Bytes)"));
#endif
            // add sample to data reader
            if (g_subgate()) g_subgate()->ApplySample(topic_name_, memfile_name_, m_ecal_buffer.data(), m_ecal_buffer.size(), (long long)ecal_message.id, (long long)ecal_message.clock, (long long)ecal_message.time, (size_t)ecal_message.hash, eCAL::pb::tl_ecal_shm);
          }
        }

        // reset timeout
        m_timeout = 0;
      }
      else
      {
        // increase timeout
        m_timeout += evt_timeout;
      }
    }

    // destroy memory file
    memfile.Destroy(false);

    // close memory file events
    gCloseEvent(m_event_snd);
    gCloseEvent(m_event_ack);

#ifndef NDEBUG
    // log it
    if(m_do_stop)
    {
      Logging::Log(log_level_debug2, std::string(topic_name_ + "::CMemFileObserver::ThreadFun(") + memfile_name_ + ", " + memfile_event_ + ") - STOPPED");
    }
    else
    {
      Logging::Log(log_level_debug2, std::string(topic_name_ + "::CMemFileObserver::ThreadFun(") + memfile_name_ + ", " + memfile_event_ + ") - TIMEOUT");
    }
#endif
    // mark as stopped
    m_is_stopped = true; //-V1020
  }

  ////////////////////////////////////////
  // CMemFileThread
  ////////////////////////////////////////
  CMemFileThread::CMemFileThread(const std::string& topic_name_, const std::string& topic_id_, const std::string& memfile_name_, const std::string& memfile_event_, const int timeout_max_) :
    m_topic_id(topic_id_)
  {
    m_thread = std::thread(&CMemFileObserver::Observe, &m_observer, topic_name_, memfile_name_, memfile_event_, timeout_max_);
  }

  CMemFileThread::~CMemFileThread()
  {
  }

  bool CMemFileThread::Stop()
  {
    if(m_observer.IsStopped()) return(false);
    m_observer.Stop();
    return(true);
  }

  bool CMemFileThread::Join()
  {
    m_thread.join();
    return(true);
  }

  bool CMemFileThread::ResetTimeout()
  {
    if(m_observer.IsStopped()) return(false);
    m_observer.ResetTimeout();
    return(true);
  }

  bool CMemFileThread::IsStopped()
  {
    return(m_observer.IsStopped());
  }

  ////////////////////////////////////////
  // CMemFileThreadPool
  ////////////////////////////////////////
  CMemFileThreadPool::CMemFileThreadPool() :
    m_created(false)
  {
  }

  CMemFileThreadPool::~CMemFileThreadPool()
  {
  }

  void CMemFileThreadPool::Create()
  {
    if(m_created) return;
    m_created = true;
  }

  void CMemFileThreadPool::Destroy()
  {
    if(!m_created) return;

    std::lock_guard<std::mutex> lock(m_thread_pool_sync);

    for(auto thread : m_thread_pool)
    {
      thread.second->Stop();
    }

    for(auto thread : m_thread_pool)
    {
      thread.second->Join();
    }

    for(auto thread : m_thread_pool)
    {
      delete thread.second;
    }

    m_thread_pool.clear();

    m_created = false;
  }

  bool CMemFileThreadPool::AssignThread(const std::string& topic_id_, const std::string& memfile_event_, const std::string& memfile_name_, const std::string& topic_name_)
  {
    if(!m_created)            return(false);
    if(memfile_name_.empty()) return(false);

    std::lock_guard<std::mutex> lock(m_thread_pool_sync);

    // remove stopped / timeout threads
    for(auto thread_iter = m_thread_pool.begin(); thread_iter != m_thread_pool.end();)
    {
      if(thread_iter->second->IsStopped())
      {
#ifndef NDEBUG
        // log it
        Logging::Log(log_level_debug2, std::string(thread_iter->first + "::CMemFileThreadPool::AssignThread - REMOVED"));
#endif
        thread_iter = m_thread_pool.erase(thread_iter);
      }
      else
      {
        thread_iter++;
      }
    }

    // reset timeout for existing threads
    auto thread_iter = m_thread_pool.find(memfile_event_);
    if(thread_iter != m_thread_pool.end())
    {
      thread_iter->second->ResetTimeout();
      return(true);
    }

    // create a new thread for that topic id
    CMemFileThread* thread = new CMemFileThread(topic_name_, topic_id_, memfile_name_, memfile_event_, CMN_REGISTRATION_TO);
    m_thread_pool[memfile_event_] = thread;
#ifndef NDEBUG
    // log it
    Logging::Log(log_level_debug2, std::string(memfile_name_ + "::CMemFileThreadPool::AssignThread - ADD"));
#endif
    return(true);
  }
}
