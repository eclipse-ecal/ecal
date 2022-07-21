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

#include "ecal_memfile.h"
#include "ecal_memfile_header.h"

#include <mutex>
#include <atomic>
#include <map>
#include <memory>
#include <thread>

#include <ecal_threaded_timer_wrapper.h>

namespace eCAL
{
  using MemoryReadFunction = std::function<void(const SMemFileHeader& header, const char* buffer)>;

  class CEvent;
  class CAcknowledgeStrategy;
  class CZeroCopyStrategy;
  class COneCopyStrategy;
  class CMemfileReadAccess;

  class CMemfileHeaderUpdated
  {
  public:
    CMemfileHeaderUpdated() : m_last_sample_clock(0) {}

    bool IsFileUpdated(const SMemFileHeader& header) const
    {
      return header.clock > m_last_sample_clock;
    }

    void Update(const SMemFileHeader& header)
    {
      m_last_sample_clock = header.clock;
    }

  private:
    uint64_t m_last_sample_clock;
  };

  class CMemFileReader
  {
  public:
    enum class ReaderState
    {
      WAITING_ON_SIGNAL = 0,
      READING_MEMFILE = 1
    };

    struct CMemFileReaderOptions
    {
      std::string memory_file_name;
      std::string event_data_sent_name;
      MemoryReadFunction memory_processing_function;
      bool memfile_acknowledge;
    };

    CMemFileReader(const CMemFileReaderOptions& options);
    void ReadMemfile();
    void operator()();


  private:
    CMemFileReader::ReaderState            m_reader_state;

    CMemoryFile                            m_memfile;


    std::unique_ptr<CEvent>                m_event_data_sent;
    std::unique_ptr<CAcknowledgeStrategy>  m_acknowledge_strategy;
    std::shared_ptr<CZeroCopyStrategy>     m_zero_copy_strategy;
    std::shared_ptr<COneCopyStrategy>      m_one_copy_strategy;

    std::unique_ptr<CMemfileReadAccess>    m_read_access;

    CMemfileHeaderUpdated                  m_header_tracker;
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

    bool ObserveFile(const std::chrono::milliseconds& timeout_, const CMemFileReader::CMemFileReaderOptions& memfile_options);

  private:
    std::atomic<bool>                                                             m_created;
    std::shared_ptr<std::mutex>                                                   m_observer_pool_sync;
    std::map<std::string, std::weak_ptr<CThreadedTimerWrapper<CMemFileReader>>>   m_observer_pool;
  };
}
