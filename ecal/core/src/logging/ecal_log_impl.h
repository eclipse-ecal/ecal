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
 * @brief  eCAL logging class
**/

#pragma once

#include "io/udp/ecal_udp_sample_receiver.h"
#include "io/udp/ecal_udp_sample_sender.h"

#include <cstddef>
#include <ecal/ecal.h>
#include <ecal/ecal_os.h>

#include <ecal/ecal_log_level.h>
#include <ecal/types/logging.h>

#include "ecal_global_accessors.h"

#include <atomic>
#include <list>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace eCAL
{
  class CLog
  {
  public:
    /**
      * @brief Constructor.
    **/
    CLog();

    /**
      * @brief Destructor.
    **/
    ~CLog();

    /**
     * @brief Create log class.
    **/
    void Create();

    /**
     * @brief Destroy log class.
    **/
    void Destroy();

    /**
      * @brief Sets the log level.
      *
      * @param level_ The level.
    **/
    void SetLogLevel(eCAL_Logging_eLogLevel level_);

    /**
      * @brief Set the current log level.
      *
      * @return   The current log level.
    **/
    eCAL_Logging_eLogLevel GetLogLevel();

    /**
      * @brief Log a message.
      *
      * @param level_  The level.
      * @param msg_    The message.
    **/
    void Log(eCAL_Logging_eLogLevel level_, const std::string& msg_);

    /**
      * @brief Log a message (with the current log level).
      *
      * @param msg_   The message.
    **/
    void Log(const std::string& msg_);

    void GetLogging(std::string& log_msg_list_string_);
    void GetLogging(Logging::SLogging& log_);

  private:
    bool HasSample(const std::string& sample_name_);
    bool ApplySample(const char* serialized_sample_data_, size_t serialized_sample_size_);

    CLog(const CLog&);                 // prevent copy-construction
    CLog& operator=(const CLog&);      // prevent assignment

    std::mutex                             m_log_mtx;

    std::atomic<bool>                      m_created;
    std::unique_ptr<UDP::CSampleSender>    m_udp_logging_sender;

    // log message list and log message serialization buffer
    Logging::SLogging                      m_log_msglist;
    std::vector<char>                      m_log_message_vec;

    // udp logging receiver
    std::shared_ptr<UDP::CSampleReceiver>  m_log_receiver;

    std::string                            m_hname;
    int                                    m_pid;
    std::string                            m_pname;

    std::string                            m_logfile_name;
    FILE*                                  m_logfile;

    eCAL_Logging_eLogLevel                 m_level;
    eCAL_Logging_Filter                    m_filter_mask_con;
    eCAL_Logging_Filter                    m_filter_mask_file;
    eCAL_Logging_Filter                    m_filter_mask_udp;
  };
}
