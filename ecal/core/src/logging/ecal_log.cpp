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
#include <string>

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
     * @brief Sets the file log filter.
     * 
     * @param filter_ The filter;
     */
    void SetFileLogFilter(eCAL_Logging_Filter filter_)
    {
      if(g_log() != nullptr) g_log()->SetFileLogFilter(filter_);
    }

    /**
     * @brief Sets the udp log filter.
     * 
     * @param filter_ The filter;
     */
    void SetUDPLogFilter(eCAL_Logging_Filter filter_)
    {
      if(g_log() != nullptr) g_log()->SetUDPLogFilter(filter_);
    }

    /**
     * @brief Sets the console log filter.
     * 
     * @param filter_ The filter;
     */
    void SetConsoleLogFilter(eCAL_Logging_Filter filter_)
    {
      if(g_log() != nullptr) g_log()->SetConsoleLogFilter(filter_);
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
     * @return  Logging buffer length or zero if failed.
    **/
    int GetLogging(std::string& log_)
    {
      if (g_log() != nullptr) g_log()->GetLogging(log_);
      return static_cast<int>(log_.size());
    }

    /**
     * @brief Get logging as struct.
     *
     * @param [out] log_  Target struct to store the logging information.
     *
     * @return Number of struct elements if succeeded.
    **/
    int GetLogging(Logging::SLogging& log_)
    {
      if (g_log() != nullptr) g_log()->GetLogging(log_);
      return static_cast<int>(log_.log_messages.size());
    }
  }
}
