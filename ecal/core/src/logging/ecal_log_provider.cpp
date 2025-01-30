/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
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

#include "ecal_log_provider.h"
#include "serialization/ecal_serialize_logging.h"
#include "config/builder/udp_attribute_builder.h"

#include <ecal/time.h>
#include <ecal_utils/filesystem.h>
#include <ecal_utils/string.h>

#include <chrono>
#include <iostream>
#include <sstream>

#ifdef ECAL_OS_WINDOWS
#include "ecal_win_main.h"
#include <iomanip>

namespace
{
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
#include <ctime>

namespace
{
  std::string get_time_str()
  {
    char            fmt[64];
    struct timeval  tv;
    struct tm       *tm = nullptr;
    gettimeofday(&tv, nullptr);
    tm = localtime(&tv.tv_sec);
    if (tm != nullptr)
    {
      strftime(fmt, sizeof fmt, "%Y-%m-%d-%H-%M-%S", tm);
    }
    return(std::string(fmt));
  }
}
#endif

namespace
{
  void logWarningToConsole(const std::string& msg_)
  {
    std::cout << "[eCAL][Logging-Provider][Warning] " << msg_ << "\n";
  }

  void createLogHeader(std::stringstream& msg_stream, const eCAL::Logging::eLogLevel level_, const eCAL::Logging::SProviderAttributes& attr_, const eCAL::Time::ecal_clock::time_point& log_time_)
  {
    msg_stream << std::chrono::duration_cast<std::chrono::milliseconds>(log_time_.time_since_epoch()).count();
    msg_stream << " ms";
    msg_stream << " | ";
    msg_stream << attr_.host_name;
    msg_stream << " | ";
    msg_stream << attr_.unit_name;
    msg_stream << " | ";
    msg_stream << attr_.process_id;
    msg_stream << " | ";
    switch(level_)
    {
    case eCAL::Logging::log_level_none:
    case eCAL::Logging::log_level_all:
      break;
    case eCAL::Logging::log_level_info:
      msg_stream << "info";
      break;
    case eCAL::Logging::log_level_warning:
      msg_stream << "warning";
      break;
    case eCAL::Logging::log_level_error:
      msg_stream << "error";
      break;
    case eCAL::Logging::log_level_fatal:
      msg_stream << "fatal";
      break;
    case eCAL::Logging::log_level_debug1:
      msg_stream << "debug1";
      break;
    case eCAL::Logging::log_level_debug2:
      msg_stream << "debug2";
      break;
    case eCAL::Logging::log_level_debug3:
      msg_stream << "debug3";
      break;
    case eCAL::Logging::log_level_debug4:
      msg_stream << "debug4";
      break;
    }
    msg_stream << " | ";
  }

  bool isDirectoryOrCreate(const std::string& path_)
  {
    if (path_.empty()) return false;

    bool path_exists = EcalUtils::Filesystem::IsDir(path_, EcalUtils::Filesystem::Current);
    if (!path_exists)
    {
      path_exists = EcalUtils::Filesystem::MkDir(path_, EcalUtils::Filesystem::Current);
    }

    return path_exists;
  }
}

namespace eCAL
{
  namespace Logging
  {
    CLogProvider::CLogProvider(const SProviderAttributes& attr_)
    : m_created(false)
    , m_logfile(nullptr)
    , m_attributes(attr_)
    {
    }

    CLogProvider::~CLogProvider()
    {
      Stop();
    }

    void CLogProvider::Start()
    {
      // create log file if file logging is enabled
      if (m_attributes.file_sink.enabled)
      {
        if (!StartFileLogging())
        {
          logWarningToConsole("Logging for file enabled, but specified path to log is not valid or could not be created: " + m_attributes.file_config.path);
        }
      }

      // create udp logging sender if udp logging is enabled
      if (m_attributes.udp_sink.enabled)
      {
        // create udp logging sender
        if (!StartUDPLogging())
        {
          logWarningToConsole("Logging for udp enabled, but could not create udp logging sender.");
        }
      }

      m_created = true;
    }
  
    bool CLogProvider::StartFileLogging()
    {
      if (!isDirectoryOrCreate(m_attributes.file_config.path)) return false;
      
      const std::string tstring = get_time_str();
      
      std::string log_path = m_attributes.file_config.path;
      if (EcalUtils::Filesystem::ToUnixSeperators(log_path).back() == '/')
        log_path.pop_back();

      const std::string file_name = tstring + "_" + m_attributes.unit_name + "_" + std::to_string(m_attributes.process_id) + ".log";
      const std::vector<std::string> file_path_components = { log_path, file_name };
      
      m_logfile_name = EcalUtils::String::Join(std::string(1, EcalUtils::Filesystem::NativeSeparator()), file_path_components);
      m_logfile = fopen(m_logfile_name.c_str(), "w");

      if (m_logfile != nullptr)
      {
        std::cout << "[eCAL][Logging-Provider] Logfile created: " << m_logfile_name << "\n";
        return true;
      }

      return false;
    }

    bool CLogProvider::StartUDPLogging()
    {
      const eCAL::UDP::SSenderAttr attr = Logging::UDP::ConvertToIOUDPSenderAttributes(m_attributes.udp_config);
      m_udp_logging_sender = std::make_unique<eCAL::UDP::CSampleSender>(attr);

      return m_udp_logging_sender != nullptr;
    }

    void CLogProvider::Log(const eLogLevel level_, const std::string& msg_)
    {
      const std::lock_guard<std::mutex> lock(m_log_mtx);

      if(!m_created) return;
      if(msg_.empty()) return;

      const Filter log_con  = level_ & m_attributes.console_sink.filter_log;
      const Filter log_file = level_ & m_attributes.file_sink.filter_log;
      const Filter log_udp  = level_ & m_attributes.udp_sink.filter_log;
      if((log_con | log_file | log_udp) == 0) return;

      auto log_time = eCAL::Time::ecal_clock::now();

      const bool log_to_console = m_attributes.console_sink.enabled && log_con != 0;
      const bool log_to_file    = m_attributes.file_sink.enabled && log_file != 0;

      if (log_to_console || log_to_file)
      {
        std::stringstream string_stream;
        createLogHeader(string_stream, level_, m_attributes, log_time);
        string_stream << msg_;
      
        if(log_to_console)
        {
          std::cout << string_stream.str() << '\n';
        }

        if (log_to_file)
        {
          fprintf(m_logfile, "%s\n", string_stream.str().c_str());
          fflush(m_logfile);
        }
      }

      if(m_attributes.udp_sink.enabled && log_udp != 0 && m_udp_logging_sender)
      {
          // set up log message
          Logging::SLogMessage log_message;
          log_message.time    = std::chrono::duration_cast<std::chrono::microseconds>(log_time.time_since_epoch()).count();
          log_message.host_name   = m_attributes.host_name;
          log_message.process_id     = m_attributes.process_id;
          log_message.process_name   = m_attributes.process_name;
          log_message.unit_name   = m_attributes.unit_name;
          log_message.level   = level_;
          log_message.content = msg_;

          // sent it
          m_log_message_vec.clear();
          SerializeToBuffer(log_message, m_log_message_vec);
          m_udp_logging_sender->Send("_log_message_", m_log_message_vec);
      }
    }

    void CLogProvider::Stop()
    {
      if(!m_created) return;

      const std::lock_guard<std::mutex> lock(m_log_mtx);

      m_udp_logging_sender.reset();

      if(m_logfile != nullptr) fclose(m_logfile);
      m_logfile = nullptr;

      m_created = false;
    }
  }
}