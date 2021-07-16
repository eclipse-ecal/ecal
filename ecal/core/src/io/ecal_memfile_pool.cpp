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
#include "ecal_config_hlp.h"
#include "ecal_memfile_header.h"
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
    m_timeout_read(0),
    m_timeout_ack(PUB_MEMFILE_ACK_TO)
  {
    m_timeout_ack = eCALPAR(PUB, MEMFILE_ACK_TO);
  }

  CMemFileObserver::~CMemFileObserver() = default;

  void CMemFileObserver::ResetTimeout()
  {
    m_timeout_read = 0;
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
    if (m_timeout_ack != 0)
    {
      gOpenEvent(&m_event_ack, memfile_event_ + "_ack");
    }

    // create memory file
    CMemoryFile memfile;
    memfile.Create(memfile_name_.c_str(), false);

    uint64_t sample_clock = 0;
    while((m_timeout_read < timeout_max_) && !m_do_stop)
    {
      // central memory file event sync with 5 ms
      const int evt_timeout = 5;
      if(gWaitForEvent(m_event_snd, evt_timeout))
      {
        std::lock_guard<std::mutex> lock(m_thread_sync);
        if(m_do_stop) break;

        // try to open memory file with timeout 5 ms
        if(memfile.GetReadAccess(5))
        {
          // read memory file header
          SMemFileHeader memfile_hdr;

          // retrieve size of received buffer
          size_t buffer_size = memfile.DataSize();

          // do we have at least the first two bytes ? (hdr_size)
          if (buffer_size >= 2)
          {
            // read received header's size
            memfile.Read(&memfile_hdr, 2, 0);
            uint16_t rcv_hdr_size = memfile_hdr.hdr_size;
            // if the header size exceeds current header version size -> limit it to that one
            uint16_t hdr_bytes2copy = std::min(rcv_hdr_size, static_cast<uint16_t>(sizeof(SMemFileHeader)));
            if (hdr_bytes2copy <= buffer_size)
            {
              // now read all we can get from the received header
              memfile.Read(&memfile_hdr, hdr_bytes2copy, 0);
            }
          }

          // are we allowed to use zero copy ?
          // -------------------------------------------------------------------------
          // That means we call the user callback (ApplySample) from within the opened memory file.
          // So we do not waste time by copying the payload in an intermediate buffer
          // but the file keeps opened and blocked until the callback returns.
          // Other subscriber can not access the content this time !
          // -------------------------------------------------------------------------
          auto zero_copy_allowed = memfile_hdr.options.zero_copy;
          if (zero_copy_allowed != 0)
          {
            // read memory file content if
            // - we have some data and 
            // - did not receive them before
            if ((memfile_hdr.data_size > 0) && (memfile_hdr.clock > sample_clock))
            {
              // acquire memory file payload pointer (no copying here)
              const void* buf(nullptr);
              if (memfile.GetReadAddress(buf, memfile_hdr.data_size, memfile_hdr.hdr_size) > 0)
              {
                // store clock
                sample_clock = memfile_hdr.clock;
#ifndef NDEBUG
                // log it
                Logging::Log(log_level_debug3, std::string(topic_name_ + "::MemFile GetReadAddress (" + std::to_string(memfile_hdr.data_size) + " Bytes)"));
#endif
                // add sample to data reader (and call user callback function)
                if (g_subgate()) g_subgate()->ApplySample(topic_name_, memfile_name_, static_cast<const char*>(buf), memfile_hdr.data_size, (long long)memfile_hdr.id, (long long)memfile_hdr.clock, (long long)memfile_hdr.time, (size_t)memfile_hdr.hash, eCAL::pb::tl_ecal_shm);
              }
            }

            // close memory file
            memfile.ReleaseReadAccess();

            // send ack event
            if (m_timeout_ack != 0)
            {
              gSetEvent(m_event_ack);
            }
          }
          else // zero_copy_allowed
          {
            // read memory file content if we have some data and did not receive them before
            if ((memfile_hdr.data_size > 0) && (memfile_hdr.clock > sample_clock))
            {
              // read payload
              m_ecal_buffer.resize((size_t)memfile_hdr.data_size);
              memfile.Read(m_ecal_buffer.data(), (size_t)memfile_hdr.data_size, memfile_hdr.hdr_size);

              // store clock
              sample_clock = memfile_hdr.clock;
            }

            // close memory file
            memfile.ReleaseReadAccess();

            // send ack event
            if (m_timeout_ack != 0)
            {
              gSetEvent(m_event_ack);
            }

            // process payload
            if (m_ecal_buffer.size() > 0)
            {
#ifndef NDEBUG
              // log it
              Logging::Log(log_level_debug3, std::string(topic_name_ + "::MemFile Read (" + std::to_string(m_ecal_buffer.size()) + " Bytes)"));
#endif
              // add sample to data reader (and call user callback function)
              if (g_subgate()) g_subgate()->ApplySample(topic_name_, memfile_name_, m_ecal_buffer.data(), m_ecal_buffer.size(), (long long)memfile_hdr.id, (long long)memfile_hdr.clock, (long long)memfile_hdr.time, (size_t)memfile_hdr.hash, eCAL::pb::tl_ecal_shm);
            }
          }
        }

        // reset timeout
        m_timeout_read = 0;
      }
      else
      {
        // increase timeout
        m_timeout_read += evt_timeout;
      }
    }

    // destroy memory file
    memfile.Destroy(false);

    // close memory file events
    gCloseEvent(m_event_snd);
    if (m_timeout_ack != 0)
    {
      gCloseEvent(m_event_ack);
    }

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

  CMemFileThread::~CMemFileThread() = default;

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

  CMemFileThreadPool::~CMemFileThreadPool() = default;

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
