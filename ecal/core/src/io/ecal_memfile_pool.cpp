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
    m_created(false),
    m_do_stop(false),
    m_is_observing(false),
    m_timeout_read(0),
    m_timeout_ack(eCALPAR(PUB, MEMFILE_ACK_TO))
  {
  }

  CMemFileObserver::~CMemFileObserver()
  {
    // stop if still running
    Stop();

    // and destroy
    Destroy();
  }

  bool CMemFileObserver::Create(const std::string& memfile_name_, const std::string& memfile_event_)
  {
    if (m_created) return false;

    // open memory file events
    gOpenEvent(&m_event_snd, memfile_event_);
    if (m_timeout_ack != 0)
    {
      gOpenEvent(&m_event_ack, memfile_event_ + "_ack");
    }

    // create memory file access
    m_memfile.Create(memfile_name_.c_str(), false);

    m_created = true;

#ifndef NDEBUG
    // log it
    Logging::Log(log_level_debug2, std::string("CMemFileObserver " + m_memfile.Name() + " created"));
#endif

    return true;
  }

  bool CMemFileObserver::Destroy()
  {
    if (!m_created) return false;

    // destroy memory file (access only)
    m_memfile.Destroy(false);

    // close memory file events
    gCloseEvent(m_event_snd);
    if (m_timeout_ack != 0)
    {
      gCloseEvent(m_event_ack);
    }

    m_created = false;

#ifndef NDEBUG
    // log it
    Logging::Log(log_level_debug2, std::string("CMemFileObserver " + m_memfile.Name() + " destroyed"));
#endif

    return true;
  }

  bool CMemFileObserver::Start(const std::string& topic_name_, const std::string& topic_id_, const int timeout_)
  {
    if (!m_created)     return false;
    if (m_is_observing) return false;

    // mark as running
    m_is_observing = true;

    // start observer thread
    m_thread = std::thread(&CMemFileObserver::Observe, this, topic_name_, topic_id_, timeout_);

#ifndef NDEBUG
    // log it
    Logging::Log(log_level_debug2, std::string("CMemFileObserver started (" + topic_name_ + ", " + topic_id_ + ")"));
#endif

    return true;
  }

  bool CMemFileObserver::Stop()
  {
    if (!m_created) return false;

    if (m_is_observing)
    {
      // signal observer to stop
      m_do_stop = true;

      // set sync event to unlock loop
      gSetEvent(m_event_snd);
    }

    // wait for finalization
    m_thread.join();

    return true;
  }

  bool CMemFileObserver::ResetTimeout()
  {
    if (!m_is_observing) return false;

    m_timeout_read = 0;
    
    return true;
  }

  void CMemFileObserver::Observe(const std::string& topic_name_, const std::string& topic_id_, const int timeout_max_)
  {
    // internal clock sample update checking
    uint64_t last_sample_clock(0);

    // runs as long as there is no timeout and no external stop request
    while((m_timeout_read < timeout_max_) && !m_do_stop)
    {
      // loop start in ms
      auto loop_start = eCAL::Time::GetMicroSeconds()/1000;

      // check for memory file update event from shm writer (20 ms)
      if(gWaitForEvent(m_event_snd, 20))
      {
        // last chance to stop ..
        if(m_do_stop) break;

        // try to open memory file (timeout 5 ms)
        if(m_memfile.GetReadAccess(5))
        {
          // read the file header
          SMemFileHeader mfile_hdr;
          ReadFileHeader(mfile_hdr);

          // check for new content
          if ((mfile_hdr.data_size == 0) || (mfile_hdr.clock <= last_sample_clock))
          {
            // release access and leave
            m_memfile.ReleaseReadAccess();
          }
          else
          {
            bool zero_copy_allowed = mfile_hdr.options.zero_copy != 0;
            // -------------------------------------------------------------------------
            // zero copy mode
            // -------------------------------------------------------------------------
            // That means we call the user callback (ApplySample) from within the opened memory file.
            // So we do not waste time by copying the payload in an intermediate buffer
            // but the file keeps opened and blocked until the callback returns.
            // Other subscriber can not access the content this time !
            // -------------------------------------------------------------------------
            if (zero_copy_allowed)
            {
              // acquire memory file payload pointer (no copying here)
              const void* buf(nullptr);
              if (m_memfile.GetReadAddress(buf, mfile_hdr.data_size, mfile_hdr.hdr_size) > 0)
              {
#ifndef NDEBUG
                // log it
                Logging::Log(log_level_debug3, std::string("CMemFileObserver GetReadAddress  " + m_memfile.Name() + " (" + std::to_string(mfile_hdr.data_size) + " Bytes)"));
#endif
                // add sample to data reader (and call user callback function)
                if (g_subgate()) g_subgate()->ApplySample(topic_name_, topic_id_, static_cast<const char*>(buf), mfile_hdr.data_size, (long long)mfile_hdr.id, (long long)mfile_hdr.clock, (long long)mfile_hdr.time, (size_t)mfile_hdr.hash, eCAL::pb::tl_ecal_shm);
              }

              // store clock
              last_sample_clock = mfile_hdr.clock;

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
            // we copy the data into the receive buffer (standard mode for eCAL < 5.10)
            // and close the file immediately
            else
            {
              // read payload
              m_ecal_buffer.resize((size_t)mfile_hdr.data_size);
              m_memfile.Read(m_ecal_buffer.data(), (size_t)mfile_hdr.data_size, mfile_hdr.hdr_size);

              // store clock
              last_sample_clock = mfile_hdr.clock;

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
                Logging::Log(log_level_debug3, std::string("CMemFileObserver Read " + m_memfile.Name() + " (" + std::to_string(m_ecal_buffer.size()) + " Bytes)"));
#endif
                // add sample to data reader (and call user callback function)
                if (g_subgate()) g_subgate()->ApplySample(topic_name_, topic_id_, m_ecal_buffer.data(), m_ecal_buffer.size(), (long long)mfile_hdr.id, (long long)mfile_hdr.clock, (long long)mfile_hdr.time, (size_t)mfile_hdr.hash, eCAL::pb::tl_ecal_shm);
              }
            }
          }
        }

        // reset timeout
        m_timeout_read = 0;
      }
      else
      {
        // increase timeout in ms
        m_timeout_read += eCAL::Time::GetMicroSeconds()/1000 - loop_start;
      }
    }

#ifndef NDEBUG
    // log it
    if(m_do_stop)
    {
      Logging::Log(log_level_debug2, std::string("CMemFileObserver " + m_memfile.Name() + " stopped"));
    }
    else
    {
      Logging::Log(log_level_debug2, std::string("CMemFileObserver " + m_memfile.Name() + " timeout"));
    }
#endif

    // mark as stopped
    m_is_observing = false; //-V1020
  }

  bool CMemFileObserver::ReadFileHeader(SMemFileHeader& mfile_hdr_)
  {
    // retrieve size of received buffer
    size_t buffer_size = m_memfile.DataSize();

    // do we have at least the first two bytes ? (hdr_size)
    if (buffer_size >= 2)
    {
      // read received header's size
      m_memfile.Read(&mfile_hdr_, 2, 0);
      uint16_t rcv_hdr_size = mfile_hdr_.hdr_size;
      // if the header size exceeds current header version size -> limit it to that one
      uint16_t hdr_bytes2copy = std::min(rcv_hdr_size, static_cast<uint16_t>(sizeof(SMemFileHeader)));
      if (hdr_bytes2copy <= buffer_size)
      {
        // now read all we can get from the received header
        m_memfile.Read(&mfile_hdr_, hdr_bytes2copy, 0);
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

    // stop and destroy them all
    for (auto & observer : m_observer_pool)
    {
      observer.second->Stop();
      observer.second->Destroy();
    }

    // clear pool
    m_observer_pool.clear();

    m_created = false;
  }

  bool CMemFileThreadPool::ObserveFile(const std::string& memfile_name_, const std::string& memfile_event_, const std::string& topic_name_, const std::string& topic_id_)
  {
    if(!m_created)            return(false);
    if(memfile_name_.empty()) return(false);

    std::lock_guard<std::mutex> lock(m_observer_pool_sync);

    // first remove outdated / finished observer finally from the thread pool
    for(auto observer = m_observer_pool.begin(); observer != m_observer_pool.end();)
    {
      if(!observer->second->IsObserving())
      {
#ifndef NDEBUG
        // log it
        Logging::Log(log_level_debug2, std::string("CMemFileThreadPool::ObserveFile " + observer->first + " removed"));
#endif

        observer->second->Stop();
        observer->second->Destroy();
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
    // confirms that there are still existing (sleepy) shm writer on this host
    auto observer = m_observer_pool.find(memfile_name_);
    if(observer != m_observer_pool.end())
    {
      observer->second->ResetTimeout();
      return(true);
    }
    // okay, we need to start a new observer
    else
    {
      auto thread = std::make_shared<CMemFileObserver>();
      thread->Create(memfile_name_, memfile_event_);
      thread->Start(topic_name_, topic_id_, eCALPAR(CMN, REGISTRATION_TO));
      m_observer_pool[memfile_name_] = thread;
#ifndef NDEBUG
      // log it
      Logging::Log(log_level_debug2, std::string("CMemFileThreadPool::ObserveFile " + memfile_name_ + " added"));
#endif
      return(true);
    }
  }
}
