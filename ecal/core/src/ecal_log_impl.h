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

#include "io/udp_sender.h"

#include <ecal/ecal.h>
#include <ecal/ecal_os.h>

#include <ecal/ecal_log_level.h>

#include "ecal_global_accessors.h"

#include <mutex>
#include <memory>
#include <chrono>

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
    void SetLogLevel(const eCAL_Logging_eLogLevel level_);

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
    void Log(const eCAL_Logging_eLogLevel level_, const std::string& msg_);

    /**
      * @brief Log a message (with the current log level).
      *
      * @param msg_   The message.
    **/
    void Log(const std::string& msg_);

    /**
      * @brief Mark the start of the user core process.
      *
    **/
    void StartCoreTimer();

    /**
      * @brief Mark the stop of the user core process.
      *
    **/
    void StopCoreTimer();

    /**
      * @brief Set the current measured core time in s (for user implemented measuring).
      *
    **/
    void SetCoreTime(const std::chrono::duration<double>& time_);

    /**
      * @brief Returns the current measured core time in s.
      *
    **/
    std::chrono::duration<double> GetCoreTime();

  private:
    char ParseLogLevel(const std::string& filter_);

    CLog(const CLog&);                 // prevent copy-construction
    CLog& operator=(const CLog&);      // prevent assignment

    std::mutex                   m_log_sync;

    bool                         m_created;
    std::unique_ptr<CUDPSender>  m_udp_sender;

    std::string                  m_hname;
    int                          m_pid;
    std::string                  m_pname;

    std::string                  m_logfile_name;
    FILE*                        m_logfile;

    eCAL_Logging_eLogLevel       m_level;
    char                         m_filter_mask_con;
    char                         m_filter_mask_file;
    char                         m_filter_mask_udp;

    std::chrono::duration<double> m_core_time;

    std::chrono::steady_clock::time_point m_core_time_start;
  };
}
