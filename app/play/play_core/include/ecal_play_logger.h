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

#pragma once

#include <string>
#include <spdlog/fmt/ostr.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_sinks.h>
#ifdef SPDLOG_VER_MAJOR
#include <spdlog/sinks/rotating_file_sink.h>
#endif

#include <iostream>

#include "ecal_play_globals.h"

#include <ecal/ecal.h>

class EcalPlayLogger
{
public:
  static std::shared_ptr<spdlog::logger> Instance()
  {
    static EcalPlayLogger ecalplay_logger;
    return ecalplay_logger.m_logger;
  }

private:
  std::shared_ptr<spdlog::logger> m_logger;

  EcalPlayLogger() {
    static const int MAXIMUM_ROTATING_FILES = 5;
    static const int FIVE_MEGABYTES = 5 * 1024 * 1024;

    auto ecal_data_path = eCAL::Util::GeteCALLogPath();
    std::string log_filename = ecal_data_path + EcalPlayGlobals::ECAL_PLAY_NAME + ".log";

    // create console logger and rotating file logger with maximum size 5MB and maximum 5 rotating files
    try
    {
      std::vector<spdlog::sink_ptr> sinks;
      sinks.push_back(std::make_shared<spdlog::sinks::stdout_sink_mt>());
      sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>(log_filename, FIVE_MEGABYTES, MAXIMUM_ROTATING_FILES));

      auto combined_logger = std::make_shared<spdlog::logger>(EcalPlayGlobals::ECAL_PLAY_NAME, begin(sinks), end(sinks));
      combined_logger->set_pattern("%v");
      combined_logger->set_level(spdlog::level::info);

#ifndef NDEBUG
      combined_logger->set_level(spdlog::level::debug);
#endif // !NDEBUG

      spdlog::register_logger(combined_logger);
    }
    catch (const spdlog::spdlog_ex& ex)
    {
      std::cout << "Log failed(eCALPlayCore): " << ex.what() << std::endl;
    }

    m_logger = spdlog::get(EcalPlayGlobals::ECAL_PLAY_NAME);

    if (!m_logger)
    {
      // if file logging fails, enable only console logging
      try
      {
        auto sink = std::make_shared<spdlog::sinks::stdout_sink_mt>();
        auto console_logger = std::make_shared<spdlog::logger>(EcalPlayGlobals::ECAL_PLAY_NAME, sink);
        spdlog::register_logger(console_logger);
      }
      catch (const spdlog::spdlog_ex& ex)
      {
        std::cout << "Console logging disabled:" << ex.what() << std::endl;
      }

      m_logger = spdlog::get(EcalPlayGlobals::ECAL_PLAY_NAME);
    }
  }

  EcalPlayLogger(const EcalPlayLogger&) = delete;
  EcalPlayLogger& operator = (const EcalPlayLogger&) = delete;
};