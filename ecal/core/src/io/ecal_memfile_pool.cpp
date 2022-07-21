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
#include <ecal/ecal_config.h>

#include "ecal_def.h"
#include "ecal_config_reader_hlp.h"
#include "pubsub/ecal_subgate.h"

#include "ecal_memfile_pool.h"

#include <chrono>
#include <cstring>
#include <mutex>

namespace eCAL
{
  class CAcknowledgeStrategy
  {
  public:
    virtual ~CAcknowledgeStrategy() = default;
    virtual void Acknowledge() = 0;
  };

  class CEventAcknowledgeStrategy : public CAcknowledgeStrategy
  {
  public:
    CEventAcknowledgeStrategy(const std::string& event_name) :
      m_event(event_name)
    {}

    void Acknowledge() override
    {
      m_event.Set();
    }

  private:
    CEvent m_event;
  };

  class CDummyAcknowledgeStrategy : public CAcknowledgeStrategy
  {
    void Acknowledge() override
    {

    }
  };

  namespace
  {
    bool ReadFileHeader(CMemoryFile& memfile, SMemFileHeader& mfile_hdr_)
    {
      // retrieve size of received buffer
      size_t buffer_size = memfile.CurDataSize();

      // do we have at least the first two bytes ? (hdr_size)
      if (buffer_size >= 2)
      {
        // read received header's size
        memfile.Read(&mfile_hdr_, 2, 0);
        uint16_t rcv_hdr_size = mfile_hdr_.hdr_size;
        // if the header size exceeds current header version size -> limit it to that one
        uint16_t hdr_bytes2copy = std::min(rcv_hdr_size, static_cast<uint16_t>(sizeof(SMemFileHeader)));
        if (hdr_bytes2copy <= buffer_size)
        {
          // now read all we can get from the received header
          memfile.Read(&mfile_hdr_, hdr_bytes2copy, 0);
          return true;
        }
      }
      return false;
    }


}

  class CMemfileReadAccess
  {
  public:
    static std::unique_ptr< CMemfileReadAccess> getAccess(CMemoryFile& memory_file)
    {
      if (memory_file.GetReadAccess(5))
      {
        return std::unique_ptr< CMemfileReadAccess>(new CMemfileReadAccess(memory_file));
      }
      else
      {
        return nullptr;
      }
    }

    SMemFileHeader FileHeader()
    {
      return m_header;
    }

    bool Read(MemoryReadFunction read_function)
    {
      const void* buf(nullptr);
      if (m_memfile.GetReadAddress(buf, m_header.data_size) > 0)
      {
        // calculate data buffer offset
        const char* data_buf = static_cast<const char*>(buf) + m_header.hdr_size;
        read_function(m_header, data_buf);
        return true;
      }
      else
      {
        return false;
      }
    }

    ~CMemfileReadAccess()
    {
      m_memfile.ReleaseReadAccess();
    }

  private:
    CMemfileReadAccess(CMemoryFile& memory_file)
      : m_memfile(memory_file)
    {
      ReadFileHeader(m_memfile, m_header);
    }

    CMemoryFile& m_memfile;
    SMemFileHeader m_header;
  };

  class CMemoryCopyingStrategy
  {
  public:
    CMemoryCopyingStrategy(const MemoryReadFunction& read_function) : m_read_function(read_function) {}

    virtual void OnMemoryFileRead(const SMemFileHeader& header, const char* buffer) = 0;
    virtual void OnPostProcessRead() = 0;

  protected:
    MemoryReadFunction m_read_function;
  };

  class CZeroCopyStrategy : public CMemoryCopyingStrategy
  {
  public:
    CZeroCopyStrategy(MemoryReadFunction read_function) : CMemoryCopyingStrategy(read_function) {}

    void OnMemoryFileRead(const SMemFileHeader& header, const char* buffer) override
    {
      m_read_function(header, buffer);
    }
    // no need for a post_processing
    void OnPostProcessRead() override
    {
    }
  };

  class COneCopyStrategy : public CMemoryCopyingStrategy
  {
  public:
    COneCopyStrategy(MemoryReadFunction read_function) : CMemoryCopyingStrategy(read_function) {}

    void OnMemoryFileRead(const SMemFileHeader& header, const char* buffer) override
    {
      m_header = header;
      m_ecal_buffer.resize(header.data_size);
      std::memcpy(m_ecal_buffer.data(), buffer, header.data_size);
    }
    void OnPostProcessRead() override
    {
      m_read_function(m_header, m_ecal_buffer.data());
    }

  private:
    SMemFileHeader m_header;
    std::vector<char>  m_ecal_buffer;

  };


  CMemFileReader::CMemFileReader(const CMemFileReaderOptions& options)
    : m_reader_state(ReaderState::WAITING_ON_SIGNAL)
    , m_event_data_sent(std::make_unique<CEvent>(options.event_data_sent_name))
  {
    m_memfile.Create(options.memory_file_name.c_str(), false);

    if (options.memfile_acknowledge)
    {
      m_acknowledge_strategy = std::make_unique<CEventAcknowledgeStrategy>(options.event_data_sent_name + "_ack");
    }
    else
    {
      m_acknowledge_strategy = std::make_unique<CDummyAcknowledgeStrategy>();
    }

    m_zero_copy_strategy = std::make_shared<CZeroCopyStrategy>(options.memory_processing_function);
    m_one_copy_strategy = std::make_shared<COneCopyStrategy>(options.memory_processing_function);
  }

  void CMemFileReader::ReadMemfile()
  {
    auto read_access = CMemfileReadAccess::getAccess(m_memfile);
    if (read_access)
    {
      auto header = read_access->FileHeader();
      if (!m_header_tracker.IsFileUpdated(header))
      {
        return;
      }

      std::shared_ptr<CMemoryCopyingStrategy> memory_copying_strategy;
      if (header.options.zero_copy)
        memory_copying_strategy = m_zero_copy_strategy;
      else
        memory_copying_strategy = m_one_copy_strategy;

      // Read from file
      read_access->Read([memory_copying_strategy](const SMemFileHeader& header, const char* buffer) {memory_copying_strategy->OnMemoryFileRead(header, buffer); });
      // Release read access
      read_access.reset();
      // update with the new header
      m_header_tracker.Update(header);
      // Acknowledge the read
      m_acknowledge_strategy->Acknowledge();
      // Do post processing (actual callback call, depending on strategy)
      memory_copying_strategy->OnPostProcessRead();
    }
  }

  void CMemFileReader::operator()()
  {
    switch (m_reader_state) {
    case ReaderState::WAITING_ON_SIGNAL:
      if (m_event_data_sent->Wait(20))
      {
        m_reader_state = ReaderState::READING_MEMFILE;
      }
      break;
    case ReaderState::READING_MEMFILE:
      ReadMemfile();
      m_reader_state = ReaderState::WAITING_ON_SIGNAL;
      break;
    }
  }



  ////////////////////////////////////////
  // CMemFileThreadPool
  ////////////////////////////////////////
  CMemFileThreadPool::CMemFileThreadPool()
    : m_created(false)
    , m_observer_pool_sync(std::make_shared<std::mutex>())
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

    // lock pool
    std::lock_guard<std::mutex> lock(*m_observer_pool_sync);

    // stop all running observers
    for (auto& observer : m_observer_pool) {
      std::shared_ptr< CThreadedTimerWrapper<CMemFileReader>> observer_ptr(observer.second);
      if (observer_ptr)
      {
        observer_ptr->Stop();
      }
    }

    // clear pool (and destroy all)
    m_observer_pool.clear();

    m_created = false;
  }

  bool CMemFileThreadPool::ObserveFile(const std::chrono::milliseconds& timeout_, const CMemFileReader::CMemFileReaderOptions& memfile_options_)
  {
    if(!m_created)            return(false);
    if(memfile_options_.memory_file_name.empty()) return(false);

    // lock pool
    std::lock_guard<std::mutex> lock(*m_observer_pool_sync);
    
    // if the observer is existing reset its timeout
    // this should avoid that an observer will timeout in the case that
    // there are no incomming data but the registration layer
    // confirms that there are still existing (sleepy) shm writer on this host
    auto observer_it = m_observer_pool.find(memfile_options_.memory_file_name);
    std::shared_ptr< CThreadedTimerWrapper<CMemFileReader>> observer_ptr(nullptr);
    if(observer_it != m_observer_pool.end())
    {
      observer_ptr = observer_it->second.lock();
    }

    if (observer_ptr)
    {
      observer_ptr->KeepAlive();
    }
    // okay, we need to start a new observer
    else
    {
      auto memfile_reader = std::make_shared< CMemFileReader >(memfile_options_);
      auto observer_weak_ptr = CThreadedTimerWrapper<CMemFileReader>::CreateCThreadedTimerWrapper(timeout_, memfile_reader);
      m_observer_pool[memfile_options_.memory_file_name] = observer_weak_ptr;

#ifndef NDEBUG
      // log it
      Logging::Log(log_level_debug2, std::string("CMemFileThreadPool::ObserveFile " + memfile_options_.memory_file_name + " added"));
#endif
    }
    return true;
  }
}
