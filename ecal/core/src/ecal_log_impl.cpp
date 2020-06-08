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

#include <ecal/ecal.h>
#include <ecal/ecal_os.h>

#include "ecal_def.h"
#include "ecal_config_hlp.h"

#include "ecal_def.h"

#include "ecal_log_impl.h"

#include <mutex>
#include <stdio.h>

#include <iostream>
#include <sstream>
#include <atomic>
#include <memory>
#include <string>
#include <vector>
#include <cstdlib>
#include <iomanip>
#include <ctime>
#include <chrono>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100 4127 4146 4800) // disable proto warnings
#endif
#include "ecal/pb/monitoring.pb.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#ifdef ECAL_OS_WINDOWS
#include "ecal_win_main.h"

static bool isDirectory(const std::string& path_)
{
  if (path_.empty()) return false;

  DWORD attr = GetFileAttributes(path_.c_str());
  if (attr == INVALID_FILE_ATTRIBUTES)
    return false;
  return (attr & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

static std::string get_time_str()
{
  auto t = std::time(nullptr);
  auto tm = *std::localtime(&t);
  std::stringstream tstream;
  tstream << std::put_time(&tm, "%Y-%m-%d-%H-%M-%S");
  return(tstream.str());
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

static void tokenize(const std::string& str, std::vector<std::string>& tokens,
                     const std::string& delimiters = " ", bool trimEmpty = false)
{
  std::string::size_type pos, lastPos = 0;

  for(;;)
  {
    pos = str.find_first_of(delimiters, lastPos);
    if(pos == std::string::npos)
    {
      pos = str.length();
      if(pos != lastPos || !trimEmpty)
      {
        tokens.emplace_back(std::string(str.data()+lastPos, pos-lastPos));
      }
      break;
    }
    else
    {
      if(pos != lastPos || !trimEmpty)
      {
        tokens.emplace_back(std::string(str.data()+lastPos, pos-lastPos ));
      }
    }
    lastPos = pos + 1;
  }
}

namespace eCAL
{
  CLog::CLog() :
          m_created(false),
          m_udp_sender(new CUDPSender()),
          m_pid(0),
          m_logfile(nullptr),
          m_level(log_level_none),
          m_filter_mask_con(log_level_info | log_level_warning | log_level_error | log_level_fatal),
          m_filter_mask_file(log_level_info | log_level_warning | log_level_error | log_level_fatal | log_level_debug1 | log_level_debug2),
          m_filter_mask_udp(log_level_info | log_level_warning | log_level_error | log_level_fatal | log_level_debug1 | log_level_debug2),
          m_core_time_start(std::chrono::nanoseconds(0))
  {
    m_core_time = std::chrono::duration<double>(-1.0);
  }

  CLog::~CLog()
  {
    Destroy();
  }

  void CLog::Create()
  {
    m_hname = Process::GetHostName();
    m_pid   = Process::GetProcessID();
    m_pname = Process::GetProcessName();
    m_level = log_level_info;

    // parse logging filter strings
    m_filter_mask_con  = ParseLogLevel(eCALPAR(MON, LOG_FILTER_CON));
    m_filter_mask_file = ParseLogLevel(eCALPAR(MON, LOG_FILTER_FILE));
    m_filter_mask_udp  = ParseLogLevel(eCALPAR(MON, LOG_FILTER_UDP));

    // create log file
    if(m_filter_mask_file)
    {
      // check ECAL_DATA
      std::string ecal_log_path = Util::GeteCALLogPath();
      if (!isDirectory(ecal_log_path)) return;

      std::string tstring = get_time_str();

      m_logfile_name = ecal_log_path + tstring + "_" + eCAL::Process::GetUnitName() + "_" + std::to_string(m_pid) + ".log";
      m_logfile = fopen(m_logfile_name.c_str(), "w");
    }

    // create log udp sender
    if(m_filter_mask_udp)
    {
      SSenderAttr attr;
      bool local_only = !eCALPAR(NET, ENABLED);
      // for local only communication we switch to local broadcasting to bypass vpn's or firewalls
      if (local_only)
      {
        attr.ipaddr    = "127.255.255.255";
        attr.broadcast = true;
      }
      else
      {
        attr.ipaddr    = eCALPAR(NET, UDP_MULTICAST_GROUP);
        attr.broadcast = false;
      }
      attr.port     = eCALPAR(NET, UDP_MULTICAST_PORT) + NET_UDP_MULTICAST_PORT_LOG_OFF;
      attr.loopback = true;
      attr.ttl      = eCALPAR(NET, UDP_MULTICAST_TTL);
      attr.sndbuf   = eCALPAR(NET, UDP_MULTICAST_SNDBUF);

      m_udp_sender->Create(attr);
    }

    m_created = true;
  }

  void CLog::Destroy()
  {
    if(!m_created) return;

    m_udp_sender->Destroy();

    if(m_logfile) fclose(m_logfile);
    m_logfile = nullptr;

    m_created = false;
  }

  char CLog::ParseLogLevel(const std::string& filter_)
  {
    // tokenize it
    std::vector<std::string> token_filter_;
    tokenize(filter_, token_filter_, " ,;");
    // create excluding filter list
    char filter_mask = log_level_none;
    for(auto& it : token_filter_)
    {
      if(it == "all")     filter_mask |= log_level_all;
      if(it == "info")    filter_mask |= log_level_info;
      if(it == "warning") filter_mask |= log_level_warning;
      if(it == "error")   filter_mask |= log_level_error;
      if(it == "fatal")   filter_mask |= log_level_fatal;
      if(it == "debug1")  filter_mask |= log_level_debug1;
      if(it == "debug2")  filter_mask |= log_level_debug2;
      if(it == "debug3")  filter_mask |= log_level_debug3;
      if(it == "debug4")  filter_mask |= log_level_debug4;
    }
    return(filter_mask);
  }

  void CLog::SetLogLevel(const eCAL_Logging_eLogLevel level_)
  { 
    std::lock_guard<std::mutex> lock(m_log_sync);
    m_level = level_;
  };

  eCAL_Logging_eLogLevel CLog::GetLogLevel()
  { 
    std::lock_guard<std::mutex> lock(m_log_sync);
    return(m_level);
  };

  void CLog::Log(const eCAL_Logging_eLogLevel level_, const std::string& msg_)
  {
    std::lock_guard<std::mutex> lock(m_log_sync);

    if(!m_created) return;
    if(msg_.empty()) return;

    char log_con  = level_ & m_filter_mask_con;
    char log_file = level_ & m_filter_mask_file;
    char log_udp  = level_ & m_filter_mask_udp;
    if(!(log_con | log_file | log_udp)) return;

    static eCAL::pb::LogMessage ecal_msg;
    static std::string        ecal_msg_s;
    auto                      log_time = eCAL::Time::ecal_clock::now();

    if(log_con)
    {
      std::cout << msg_ << std::endl;
    }

    if(log_file && m_logfile)
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

    if(log_udp && m_udp_sender)
    {
      ecal_msg.Clear();
      ecal_msg.set_time(std::chrono::duration_cast<std::chrono::microseconds>(log_time.time_since_epoch()).count());
      ecal_msg.set_hname(m_hname);
      ecal_msg.set_pid(m_pid);
      ecal_msg.set_pname(m_pname);
      ecal_msg.set_uname(eCAL::Process::GetUnitName());
      ecal_msg.set_level(level_);
      ecal_msg.set_content(msg_);

      ecal_msg_s = ecal_msg.SerializeAsString();
      if(!ecal_msg_s.empty())
      {
        m_udp_sender->Send((void*)ecal_msg_s.data(), ecal_msg_s.size());
      }
    }
  }

  void CLog::Log(const std::string& msg_)
  {
    Log(m_level, msg_);
  }

  void CLog::StartCoreTimer()
  {
    std::lock_guard<std::mutex> lock(m_log_sync);

    m_core_time_start = std::chrono::steady_clock::now();
  }

  void CLog::StopCoreTimer()
  {
    std::lock_guard<std::mutex> lock(m_log_sync);

    m_core_time = std::chrono::steady_clock::now() - m_core_time_start;
  }

  void CLog::SetCoreTime(const std::chrono::duration<double>& time_)
  {
    std::lock_guard<std::mutex> lock(m_log_sync);

    m_core_time = time_;
  }

  std::chrono::duration<double> CLog::GetCoreTime()
  {
    std::lock_guard<std::mutex> lock(m_log_sync);

    return(m_core_time);
  }
}
