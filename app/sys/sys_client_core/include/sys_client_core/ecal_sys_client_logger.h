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

#include <ecal/ecal.h>

namespace eCAL
{
  namespace sys_client
  {
    class EcalSysClientLogger
    {
    public:
      static std::shared_ptr<spdlog::logger> Instance()
      {
        static EcalSysClientLogger ecal_sys_logger;
        return ecal_sys_logger.m_logger;
      }

    private:
      std::shared_ptr<spdlog::logger> m_logger;

      const char* ecal_sys_client = "ecal_sys_client";

      EcalSysClientLogger() {
        static const int MAXIMUM_ROTATING_FILES = 5;
        static const int FIVE_MEGABYTES = 5 * 1024 * 1024;

        auto ecal_data_path = eCAL::Util::GeteCALLogPath();
        std::string log_filename = ecal_data_path + ecal_sys_client + ".log";

        // create console logger and rotating file logger with maximum size 5MB and maximum 5 rotating files
        try
        {
          std::vector<spdlog::sink_ptr> sinks;
          sinks.push_back(std::make_shared<spdlog::sinks::stdout_sink_mt>());
          sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>(log_filename, FIVE_MEGABYTES, MAXIMUM_ROTATING_FILES));

          auto combined_logger = std::make_shared<spdlog::logger>(ecal_sys_client, begin(sinks), end(sinks));
          combined_logger->set_pattern("%v");
          combined_logger->set_level(spdlog::level::info);

#ifndef NDEBUG
          combined_logger->set_level(spdlog::level::debug);
#endif // !NDEBUG

          spdlog::register_logger(combined_logger);
        }
        catch (const spdlog::spdlog_ex& ex)
        {
          std::cout << "Log failed(eCAL Sys CLient): " << ex.what() << std::endl;
        }

        m_logger = spdlog::get(ecal_sys_client);

        if (!m_logger)
        {
          // if file logging fails, enable only console logging
          try
          {
            auto sink = std::make_shared<spdlog::sinks::stdout_sink_mt>();
            auto console_logger = std::make_shared<spdlog::logger>(ecal_sys_client, sink);
            spdlog::register_logger(console_logger);
          }
          catch (const spdlog::spdlog_ex& ex)
          {
            std::cout << "Console logging disabled:" << ex.what() << std::endl;
          }

          m_logger = spdlog::get(ecal_sys_client);
        }
      }

      EcalSysClientLogger(const EcalSysClientLogger&) = delete;
      EcalSysClientLogger& operator = (const EcalSysClientLogger&) = delete;
    };
  }
}