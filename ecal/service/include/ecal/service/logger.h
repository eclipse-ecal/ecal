/* ========================= eCAL LICENSE ===== ============================
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
#include <functional>
#include <iostream>

namespace eCAL
{
  namespace service
  {
    enum class LogLevel
    {
      DebugVerbose,
      Debug,
      Info,
      Warning,
      Error,
      Fatal,
    };

    using LoggerT = std::function<void (const LogLevel, const std::string &)>;

    inline LoggerT default_logger(const std::string& node_name, LogLevel min_log_level = LogLevel::DebugVerbose)
    {
      return [node_name, min_log_level](const LogLevel log_level, const std::string& message)
                        {
                          if (log_level < min_log_level)
                            return;

                          switch (log_level)
                          {
                          case LogLevel::DebugVerbose:
                            std::cout << "[" + node_name + "] [Debug+]  " + message + "\n";
                            break;
                          case LogLevel::Debug:
                            std::cout << "[" + node_name + "] [Debug]   " + message + "\n";
                            break;
                          case LogLevel::Info:
                            std::cout << "[" + node_name + "] [Info]    " + message + "\n";
                            break;
                          case LogLevel::Warning:
                            std::cerr << "[" + node_name + "] [Warning] " + message + "\n";
                            break;
                          case LogLevel::Error:
                            std::cerr << "[" + node_name + "] [Error]   " + message + "\n";
                            break;
                          case LogLevel::Fatal:
                            std::cerr << "[" + node_name + "] [Fatal]   " + message + "\n";
                            break;
                          default:
                            break;
                          }
                        };
    }
  }
}