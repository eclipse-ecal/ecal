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
 * @brief  tcp_pubsub logger
**/

#pragma once

#include <ecal/ecal_log.h>
#include <tcp_pubsub/tcp_pubsub_logger.h>

namespace eCAL
{
  static void TcpPubsubLogger(const tcp_pubsub::logger::LogLevel log_level_, const std::string& message_)
  {
    switch (log_level_)
    {
    case tcp_pubsub::logger::LogLevel::DebugVerbose:
      eCAL::Logging::Log(eCAL_Logging_eLogLevel::log_level_debug4, std::string("CTCPReaderLayer - TCPPubSub (DebugVerbose) -") + message_);
      break;
    case tcp_pubsub::logger::LogLevel::Debug:
      eCAL::Logging::Log(eCAL_Logging_eLogLevel::log_level_debug3, std::string("CTCPReaderLayer - TCPPubSub (Debug) -") + message_);
      break;
    case tcp_pubsub::logger::LogLevel::Info:
      eCAL::Logging::Log(eCAL_Logging_eLogLevel::log_level_info, std::string("CTCPReaderLayer - TCPPubSub (Info) -") + message_);
      break;
    case tcp_pubsub::logger::LogLevel::Warning:
      eCAL::Logging::Log(eCAL_Logging_eLogLevel::log_level_warning, std::string("CTCPReaderLayer - TCPPubSub (Warning) -") + message_);
      break;
    case tcp_pubsub::logger::LogLevel::Error:
      eCAL::Logging::Log(eCAL_Logging_eLogLevel::log_level_error, std::string("CTCPReaderLayer - TCPPubSub (Error) -") + message_);
      break;
    case tcp_pubsub::logger::LogLevel::Fatal:
      eCAL::Logging::Log(eCAL_Logging_eLogLevel::log_level_fatal, std::string("CTCPReaderLayer - TCPPubSub (Fatal) -") + message_);
      break;
    default:
      break;
    }
  }
}
