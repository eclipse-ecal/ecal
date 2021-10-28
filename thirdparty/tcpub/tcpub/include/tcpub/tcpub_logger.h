// Copyright (c) Continental. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for details.

#pragma once

#include <string>
#include <functional>
#include <iostream>

#include <stdint.h>

namespace tcpub
{
  namespace logger
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

    typedef std::function<void(const LogLevel, const std::string&)> logger_t;

    static const logger_t default_logger
          = [](const LogLevel log_level, const std::string& message)
              {
                switch (log_level)
                {
                case LogLevel::DebugVerbose:
                  std::cout << "[TCPub] [Debug+]  " + message + "\n";
                  break;
                case LogLevel::Debug:
                  std::cout << "[TCPub] [Debug]   " + message + "\n";
                  break;
                case LogLevel::Info:
                  std::cout << "[TCPub] [Info]    " + message + "\n";
                  break;
                case LogLevel::Warning:
                  std::cerr << "[TCPub] [Warning] " + message + "\n";
                  break;
                case LogLevel::Error:
                  std::cerr << "[TCPub] [Error]   " + message + "\n";
                  break;
                case LogLevel::Fatal:
                  std::cerr << "[TCPub] [Fatal]   " + message + "\n";
                  break;
                default:
                  break;
                }
              };

    static const logger_t logger_no_verbose_debug
          = [](const LogLevel log_level, const std::string& message)
              {
                switch (log_level)
                {
                case LogLevel::DebugVerbose:
                  break;
                case LogLevel::Debug:
                  std::cout << "[TCPub] [Debug]   " + message + "\n";
                  break;
                case LogLevel::Info:
                  std::cout << "[TCPub] [Info]    " + message + "\n";
                  break;
                case LogLevel::Warning:
                  std::cerr << "[TCPub] [Warning] " + message + "\n";
                  break;
                case LogLevel::Error:
                  std::cerr << "[TCPub] [Error]   " + message + "\n";
                  break;
                case LogLevel::Fatal:
                  std::cerr << "[TCPub] [Fatal]   " + message + "\n";
                  break;
                default:
                  break;
                }
              };
  }
}
