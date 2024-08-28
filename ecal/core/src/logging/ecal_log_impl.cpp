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
 * @brief  eCAL logging class
**/

#include <ecal/ecal.h>
#include <ecal/ecal_os.h>
#include <ecal/ecal_config.h>

#include "ecal_log_impl.h"
#include "io/udp/ecal_udp_configurations.h"
#include "serialization/ecal_serialize_logging.h"

#include <mutex>
#include <cstdio>

#include <iostream>
#include <sstream>
#include <memory>
#include <string>
#include <iomanip>
#include <ctime>
#include <chrono>

#ifdef ECAL_OS_WINDOWS
#include "ecal_win_main.h"
#include <ecal_utils/filesystem.h>

namespace
{
  bool isDirectory(const std::string& path_)
  {
    if (path_.empty()) return false;

    return EcalUtils::Filesystem::IsDir(path_, EcalUtils::Filesystem::Current);
  }

  std::string get_time_str()
  {
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::stringstream tstream;
    tstream << std::put_time(&tm, "%Y-%m-%d-%H-%M-%S");
    return(tstream.str());
  }
}
#endif

#ifdef ECAL_OS_LINUX
#include <sys/stat.h>
#include <sys/time.h>

static bool isDirectory(const std::string& path_)
{
  if (path_.empty()) return false;

  struct stat st;
  if (stat(path_.c_str(), &st) == 0)
    return S_ISDIR(st.st_mode);

  return false;
}

static std::string get_time_str()
{
  char            fmt[64];
  struct timeval  tv;
  struct tm       *tm;
  gettimeofday(&tv, NULL);
  if ((tm = localtime(&tv.tv_sec)) != NULL)
  {
    strftime(fmt, sizeof fmt, "%Y-%m-%d-%H-%M-%S", tm);
  }
  return(std::string(fmt));
}
#endif

namespace eCAL
{
  CLog::CLog() :
          m_created(false),
          m_pid(0),
          m_logfile(nullptr),
          m_level(log_level_none),
          m_filter_mask_con(log_level_info | log_level_warning | log_level_error | log_level_fatal),
          m_filter_mask_file(log_level_info | log_level_warning | log_level_error | log_level_fatal | log_level_debug1 | log_level_debug2),
          m_filter_mask_udp(log_level_info | log_level_warning | log_level_error | log_level_fatal | log_level_debug1 | log_level_debug2)
  {
  }

  CLog::~CLog()
  {
    Stop();
  }

  void CLog::Start()
  {
    m_hname = Process::GetHostName();
    m_pid   = Process::GetProcessID();
    m_pname = Process::GetProcessName();
    m_level = log_level_info;

    // parse logging filter strings
    m_filter_mask_con  = Config::GetConsoleLogFilter();
    m_filter_mask_file = Config::GetFileLogFilter();
    m_filter_mask_udp  = Config::GetUdpLogFilter();

    // create log file
    if(m_filter_mask_file != 0)
    {
      // check ECAL_DATA
      const std::string ecal_log_path = Util::GeteCALLogPath();
      if (!isDirectory(ecal_log_path)) return;

      const std::string tstring = get_time_str();

      m_logfile_name = ecal_log_path + tstring + "_" + eCAL::Process::GetUnitName() + "_" + std::to_string(m_pid) + ".log";
      m_logfile = fopen(m_logfile_name.c_str(), "w");
    }

    if(m_filter_mask_udp != 0)
    {
      // set logging send network attributes
      eCAL::UDP::SSenderAttr attr;
      attr.address   = UDP::GetLoggingAddress();
      attr.port      = UDP::GetLoggingPort();
      attr.ttl       = UDP::GetMulticastTtl();
      attr.broadcast = UDP::IsBroadcast();
      attr.loopback  = true;
      attr.sndbuf    = UDP::GetSendBufferSize();

      // create udp logging sender
      m_udp_logging_sender = std::make_unique<UDP::CSampleSender>(attr);
    }

    // set logging receive network attributes
    eCAL::UDP::SReceiverAttr attr;
    attr.address   = UDP::GetLoggingAddress();
    attr.port      = UDP::GetLoggingPort();
    attr.broadcast = UDP::IsBroadcast();
    attr.loopback  = true;
    attr.rcvbuf    = UDP::GetReceiveBufferSize();

    // start logging receiver
    m_log_receiver = std::make_shared<UDP::CSampleReceiver>(attr, std::bind(&CLog::HasSample, this, std::placeholders::_1), std::bind(&CLog::ApplySample, this, std::placeholders::_1, std::placeholders::_2));

    m_created = true;
  }

  void CLog::Stop()
  {
    if(!m_created) return;

    const std::lock_guard<std::mutex> lock(m_log_mtx);

    m_udp_logging_sender.reset();

    if(m_logfile != nullptr) fclose(m_logfile);
    m_logfile = nullptr;

    m_created = false;
  }

  void CLog::SetLogLevel(const eCAL_Logging_eLogLevel level_)
  {
    const std::lock_guard<std::mutex> lock(m_log_mtx);
    m_level = level_;
  }

  eCAL_Logging_eLogLevel CLog::GetLogLevel()
  {
    const std::lock_guard<std::mutex> lock(m_log_mtx);
    return(m_level);
  }

  void CLog::Log(const eCAL_Logging_eLogLevel level_, const std::string& msg_)
  {
    const std::lock_guard<std::mutex> lock(m_log_mtx);

    if(!m_created) return;
    if(msg_.empty()) return;

    const eCAL_Logging_Filter log_con  = level_ & m_filter_mask_con;
    const eCAL_Logging_Filter log_file = level_ & m_filter_mask_file;
    const eCAL_Logging_Filter log_udp  = level_ & m_filter_mask_udp;
    if((log_con | log_file | log_udp) == 0) return;

    auto log_time = eCAL::Time::ecal_clock::now();

    if(log_con != 0)
    {
      std::cout << msg_ << '\n';
    }

    if((log_file != 0) && (m_logfile != nullptr))
    {
      std::stringstream msg_stream;
      msg_stream << std::chrono::duration_cast<std::chrono::milliseconds>(log_time.time_since_epoch()).count();
      msg_stream << " ms";
      msg_stream << " | ";
      msg_stream << m_hname;
      msg_stream << " | ";
      msg_stream << eCAL::Process::GetUnitName();
      msg_stream << " | ";
      msg_stream << m_pid;
      msg_stream << " | ";
      switch(level_)
      {
      case log_level_none:
      case log_level_all:
        break;
      case log_level_info:
        msg_stream << "info";
        break;
      case log_level_warning:
        msg_stream << "warning";
        break;
      case log_level_error:
        msg_stream << "error";
        break;
      case log_level_fatal:
        msg_stream << "fatal";
        break;
      case log_level_debug1:
        msg_stream << "debug1";
        break;
      case log_level_debug2:
        msg_stream << "debug2";
        break;
      case log_level_debug3:
        msg_stream << "debug3";
        break;
      case log_level_debug4:
        msg_stream << "debug4";
        break;
      }
      msg_stream << " | ";
      msg_stream << msg_;

      fprintf(m_logfile, "%s\n", msg_stream.str().c_str());
      fflush(m_logfile);
    }

    if((log_udp != 0) && m_udp_logging_sender)
    {
      // set up log message
      Logging::SLogMessage log_message;
      log_message.time    = std::chrono::duration_cast<std::chrono::microseconds>(log_time.time_since_epoch()).count();
      log_message.hname   = m_hname;
      log_message.pid     = m_pid;
      log_message.pname   = m_pname;
      log_message.uname   = eCAL::Process::GetUnitName();
      log_message.level   = level_;
      log_message.content = msg_;

      // sent it
      m_log_message_vec.clear();
      SerializeToBuffer(log_message, m_log_message_vec);
      m_udp_logging_sender->Send("_log_message_", m_log_message_vec);
    }
  }

  void CLog::Log(const std::string& msg_)
  {
    Log(m_level, msg_);
  }

  void CLog::GetLogging(std::string& log_msg_list_string_)
  {
    // clear target list string
    log_msg_list_string_.clear();

    // serialize message list
    {
      const std::lock_guard<std::mutex> lock(m_log_mtx);
      // serialize message list to target list string
      SerializeToBuffer(m_log_msglist, log_msg_list_string_);
      // clear message list
      m_log_msglist.log_messages.clear();
    }
  }

  void CLog::GetLogging(Logging::SLogging& log_)
  {
    // copy message list
    {
      const std::lock_guard<std::mutex> lock(m_log_mtx);
      log_ = m_log_msglist;
      // clear message list
      m_log_msglist.log_messages.clear();
    }
  }

  bool CLog::HasSample(const std::string& sample_name_)
  {
    return (sample_name_ == "_log_message_");
  }

  bool CLog::ApplySample(const char* serialized_sample_data_, size_t serialized_sample_size_)
  {
    // TODO: Limit maximum size of collected log messages !
    Logging::SLogMessage log_message;
    if (DeserializeFromBuffer(serialized_sample_data_, serialized_sample_size_, log_message))
    {
      // in "network mode" we accept all log messages
      // in "local mode" we accept log messages from this host only
      if ((m_hname == log_message.hname) || Config::IsNetworkEnabled())
      {
        const std::lock_guard<std::mutex> lock(m_log_mtx);
        m_log_msglist.log_messages.emplace_back(log_message);
      }
      return true;
    }
    return false;
  }
}
