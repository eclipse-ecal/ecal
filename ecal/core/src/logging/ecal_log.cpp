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

#include "ecal_log_impl.h"

namespace eCAL
{
  namespace Logging
  {
    /**
     * @brief Sets the log level.
     *
     * @param level_ The level.
    **/
    void SetLogLevel(const eCAL_Logging_eLogLevel level_)
    {
      if(g_log() != nullptr) g_log()->SetLogLevel(level_);
    }

    /**
     * @brief Get the current log level.
     *
     * @return   The current log level.
    **/
    eCAL_Logging_eLogLevel GetLogLevel()
    {
      if(g_log() != nullptr) return(g_log()->GetLogLevel());
      else                   return(log_level_none);
    }

    /**
     * @brief Log a message.
     *
     * @param msg_   The log message string.
    **/
    void Log(const std::string& msg_)
    {
      if(g_log() != nullptr) g_log()->Log(msg_);
    }

    /**
     * @brief Get logging as serialized protobuf string.
     *
     * @param [out] log_  String to store the logging information.
     *
     * @return  Monitoring buffer length or zero if failed.
    **/
    int GetLogging(std::string& log_)
    {
      if (g_log() != nullptr) g_log()->GetLogging(log_);
      return static_cast<int>(log_.size());
    }
  }
}
