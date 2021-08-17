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
#include "pubsub/ecal_subgate.h"

#include "ecal_memfile_pool.h"

namespace eCAL
{
  ////////////////////////////////////////
  // CMemFileObserver
  ////////////////////////////////////////
  CMemFileObserver::CMemFileObserver() :
    m_do_stop(false),
    m_is_running(false),
    m_timeout_read(0),
    m_timeout_ack(eCALPAR(PUB, MEMFILE_ACK_TO))
  {
  }

  CMemFileObserver::~CMemFileObserver() = default;

  void CMemFileObserver::Start(const std::string& topic_name_, const std::string& topic_id_, const std::string& memfile_name_, const std::string& memfile_event_, const int timeout_max_)
  {
    if (m_is_running) return;

    // open memory file events
    gOpenEvent(&m_event_snd, memfile_event_);
    if (m_timeout_ack != 0)
    {
      gOpenEvent(&m_event_ack, memfile_event_ + "_ack");
    }

    // create memory file access
    m_memfile.Create(memfile_name_.c_str(), false);

    // start observer thread
    m_thread = std::thread(&CMemFileObserver::Observe, this, topic_name_, topic_id_, memfile_name_, memfile_event_, timeout_max_);

    // mark as running
    m_is_running = true;

#ifndef NDEBUG
    // log it
    Logging::Log(log_level_debug2, std::string(topic_name_ + "::MemFile Thread Started (" + memfile_name_ + ", " + memfile_event_ + ")"));
#endif
  }

  void CMemFileObserver::Stop()
  {
    if(!m_is_running) return;

    // signal observer thread to stop
    m_do_stop = true;
    // set sync event to unlock loop
    gSetEvent(m_event_snd);

    // wait for finalization
    m_thread.join();

    // destroy memory file (access only)
    m_memfile.Destroy(false);

    // close memory file events
    gCloseEvent(m_event_snd);
    if (m_timeout_ack != 0)
    {
      gCloseEvent(m_event_ack);
    }
  }

  bool CMemFileObserver::ResetTimeout()
  {
    if (!m_is_running) return(false);
    m_timeout_read = 0;
    return(true);
  }

  void CMemFileObserver::Observe(const std::string& topic_name_, const std::string& topic_id_, const std::string& memfile_name_, const std::string& memfile_event_, const int timeout_max_)
  {
    // internal clock sample update checking
    uint64_t last_sample_clock(0);

    // runs as long as there is no timeout and no external stop request
    while((m_timeout_read < timeout_max_) && !m_do_stop)
    {
      // check for memory file update event from shm writer
      const int evt_timeout = 100;
      if(gWaitForEvent(m_event_snd, evt_timeout))
      {
        // last chance to stop ..
        if(m_do_stop) break;

        // try to open memory file (timeout 5 ms)
        if(m_memfile.GetReadAccess(5))
        {
          // read the file header
          SMemFileHeader memfile_hdr;
          ReadFileHeader(memfile_hdr);

          // check for new content
          if ((memfile_hdr.data_size == 0) || (memfile_hdr.clock <= last_sample_clock))
          {
            // release access and leave
            m_memfile.ReleaseReadAccess();
          }
          else
          {
            auto zero_copy_allowed = memfile_hdr.options.zero_copy;
            // -------------------------------------------------------------------------
            // zero copy mode
            // -------------------------------------------------------------------------
            // That means we call the user callback (ApplySample) from within the opened memory file.
            // So we do not waste time by copying the payload in an intermediate buffer
            // but the file keeps opened and blocked until the callback returns.
            // Other subscriber can not access the content this time !
            // -------------------------------------------------------------------------
            if (zero_copy_allowed != 0)
            {
              // acquire memory file payload pointer (no copying here)
              const void* buf(nullptr);
              if (m_memfile.GetReadAddress(buf, memfile_hdr.data_size, memfile_hdr.hdr_size) > 0)
              {
#ifndef NDEBUG
                // log it
                Logging::Log(log_level_debug3, std::string(topic_name_ + "::MemFile GetReadAddress (" + std::to_string(memfile_hdr.data_size) + " Bytes)"));
#endif
                // add sample to data reader (and call user callback function)
                if (g_subgate()) g_subgate()->ApplySample(topic_name_, topic_id_, static_cast<const char*>(buf), memfile_hdr.data_size, (long long)memfile_hdr.id, (long long)memfile_hdr.clock, (long long)memfile_hdr.time, (size_t)memfile_hdr.hash, eCAL::pb::tl_ecal_shm);
              }

              // store clock
              last_sample_clock = memfile_hdr.clock;

              // close memory file
              m_memfile.ReleaseReadAccess();

              // send ack event
              if (m_timeout_ack != 0)
              {
                gSetEvent(m_event_ack);
              }
            }
            // -------------------------------------------------------------------------
            // buffered mode
            // -------------------------------------------------------------------------
            // we read the data into the receive buffer
            // and close the file immediately
            else
            {
              // read payload
              m_ecal_buffer.resize((size_t)memfile_hdr.data_size);
              m_memfile.Read(m_ecal_buffer.data(), (size_t)memfile_hdr.data_size, memfile_hdr.hdr_size);

              // store clock
              last_sample_clock = memfile_hdr.clock;

              // release access
              m_memfile.ReleaseReadAccess();

              // send ack event
              if (m_timeout_ack != 0)
              {
                gSetEvent(m_event_ack);
              }

              // process payload
              if (!m_ecal_buffer.empty())
              {
#ifndef NDEBUG
                // log it
                Logging::Log(log_level_debug3, std::string(topic_name_ + "::MemFile Read (" + std::to_string(m_ecal_buffer.size()) + " Bytes)"));
#endif
                // add sample to data reader (and call user callback function)
                if (g_subgate()) g_subgate()->ApplySample(topic_name_, topic_id_, m_ecal_buffer.data(), m_ecal_buffer.size(), (long long)memfile_hdr.id, (long long)memfile_hdr.clock, (long long)memfile_hdr.time, (size_t)memfile_hdr.hash, eCAL::pb::tl_ecal_shm);
              }
            }
          }
        }
        // reset timeout
        m_timeout_read = 0;
      }
      else
      {
        // increase timeout
        m_timeout_read += 10*evt_timeout;
      }
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
    m_is_running = false; //-V1020
  }

  bool CMemFileObserver::ReadFileHeader(SMemFileHeader& memfile_hdr)
  {
    // retrieve size of received buffer
    size_t buffer_size = m_memfile.DataSize();

    // do we have at least the first two bytes ? (hdr_size)
    if (buffer_size >= 2)
    {
      // read received header's size
      m_memfile.Read(&memfile_hdr, 2, 0);
      uint16_t rcv_hdr_size = memfile_hdr.hdr_size;
      // if the header size exceeds current header version size -> limit it to that one
      uint16_t hdr_bytes2copy = std::min(rcv_hdr_size, static_cast<uint16_t>(sizeof(SMemFileHeader)));
      if (hdr_bytes2copy <= buffer_size)
      {
        // now read all we can get from the received header
        m_memfile.Read(&memfile_hdr, hdr_bytes2copy, 0);
        return true;
      }
    }
    return false;
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

    std::lock_guard<std::mutex> lock(m_observer_pool_sync);

    // stop and delete them all
    for (auto & observer : m_observer_pool)
    {
      observer.second->Stop();
      delete observer.second;
      observer.second = nullptr;
    }

    // clear pool
    m_observer_pool.clear();

    m_created = false;
  }

  bool CMemFileThreadPool::AssignThread(const std::string& topic_id_, const std::string& memfile_event_, const std::string& memfile_name_, const std::string& topic_name_)
  {
    if(!m_created)            return(false);
    if(memfile_name_.empty()) return(false);

    std::lock_guard<std::mutex> lock(m_observer_pool_sync);

    // first remove outdated / stopped observer finally from the thread pool
    for(auto observer = m_observer_pool.begin(); observer != m_observer_pool.end();)
    {
      if(observer->second->IsStopped())
      {
#ifndef NDEBUG
        // log it
        Logging::Log(log_level_debug2, std::string(observer->first + "::CMemFileThreadPool::AssignThread - REMOVED"));
#endif
        observer = m_observer_pool.erase(observer);
      }
      else
      {
        observer++;
      }
    }

    // if the observer is existing reset its timeout
    // this should avoid that an observer will timeout in the case that
    // there are no incomming data but the registration layer
    // confirms that there are still existing publisher connections
    auto observer = m_observer_pool.find(memfile_event_);
    if(observer != m_observer_pool.end())
    {
      observer->second->ResetTimeout();
      return(true);
    }
    // okay, we need to start a new observer
    else
    {
      auto* thread = new CMemFileObserver();
      thread->Start(topic_name_, topic_id_, memfile_name_, memfile_event_, eCALPAR(CMN, REGISTRATION_TO));
      m_observer_pool[memfile_event_] = thread;
#ifndef NDEBUG
      // log it
      Logging::Log(log_level_debug2, std::string(memfile_name_ + "::CMemFileThreadPool::AssignThread - ADD"));
#endif
      return(true);
    }
  }
}
