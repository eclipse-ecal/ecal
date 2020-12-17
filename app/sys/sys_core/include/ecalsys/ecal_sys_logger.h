/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2020 Continental Corporation
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

#pragma once

#include <string>

#include <spdlog/fmt/ostr.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_sinks.h>
#ifdef SPDLOG_VER_MAJOR
#include <spdlog/sinks/rotating_file_sink.h>
#endif
#include <iostream>

#include <memory>

#include "ecalsys/esys_defs.h"

#include <ecal/ecal.h>

class EcalSysLogger
{
public:
  static void Log(const std::string& message, spdlog::level::level_enum priority = spdlog::level::info) {
    GetInstance().Log_(message, priority);
  }

private:
  std::shared_ptr<spdlog::logger> m_logger;

  EcalSysLogger() {
    static const int MAXIMUM_ROTATING_FILES = 5;
    static const int FIVE_MEGABYTES = 5 * 1024 * 1024;

    auto ecal_data_path = eCAL::Util::GeteCALLogPath();
    std::string log_filename = ecal_data_path + ECAL_SYS_LIB_NAME + ".log";

    // create console logger and rotating file logger with maximum size 5MB and maximum 5 rotating files
    try
    {
      std::vector<spdlog::sink_ptr> sinks;
      sinks.push_back(std::make_shared<spdlog::sinks::stdout_sink_mt>());
      sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>(log_filename, FIVE_MEGABYTES, MAXIMUM_ROTATING_FILES));

      auto combined_logger = std::make_shared<spdlog::logger>(ECAL_SYS_LIB_NAME, begin(sinks), end(sinks));
      combined_logger->set_pattern("%v");
      combined_logger->set_level(spdlog::level::info);

#ifndef NDEBUG
      combined_logger->set_level(spdlog::level::debug);
#endif // !NDEBUG

      spdlog::register_logger(combined_logger);
    }
    catch (const spdlog::spdlog_ex& ex)
    {
      std::cout << "Log failed(eCALSysCore): " << ex.what() << std::endl;
    }

    m_logger = spdlog::get(ECAL_SYS_LIB_NAME);

    if (!m_logger)
    {
      // if file logging fails, enable only console logging
      try
      {
        auto sink = std::make_shared<spdlog::sinks::stdout_sink_mt>();
        auto console_logger = std::make_shared<spdlog::logger>(ECAL_SYS_LIB_NAME, sink);
        spdlog::register_logger(console_logger);
      }
      catch (const spdlog::spdlog_ex& ex)
      {
        std::cout << "Console logging disabled:" << ex.what() << std::endl;
      }

      m_logger = spdlog::get(ECAL_SYS_LIB_NAME);
    }
  }

  EcalSysLogger(const EcalSysLogger&) = delete;
  EcalSysLogger& operator = (const EcalSysLogger&) = delete;

  static EcalSysLogger& GetInstance() {
    static EcalSysLogger instance;
    return instance;
  }

  bool Log_(const std::string& message, spdlog::level::level_enum priority) const
  {
    if (!m_logger) return false;

    bool retVal = true;

    try
    {
      switch (priority)
      {
      case spdlog::level::critical:
        m_logger->critical(message);
        break;
      case spdlog::level::debug:
        m_logger->debug(message);
        break;
      case spdlog::level::err:
        m_logger->error(message);
        break;
      case spdlog::level::info:
        m_logger->info(message);
        break;
      case spdlog::level::trace:
        m_logger->trace(message);
        break;
      case spdlog::level::warn:
        m_logger->warn(message);
        break;
      default:
        m_logger->info(message);
        break;
      }
    }
    catch (const spdlog::spdlog_ex& ex)
    {
      retVal = false;
      std::cout << "Log failed(eCALSysCore): " << ex.what() << std::endl;
    }

    return retVal;
  }
};