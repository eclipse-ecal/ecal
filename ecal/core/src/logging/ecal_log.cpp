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

/**
 * @brief  eCAL logging class
**/

#include <ecal/ecal.h>
#include <string>

#include "ecal_log_provider.h"
#include "ecal_log_receiver.h"
#include "ecal_global_accessors.h"

namespace eCAL
{
  namespace Logging
  {
    /**
     * @brief Log a message.
     *
     * @param msg_   The log message string.
    **/
    void Log(eLogLevel level_, const std::string& msg_)
    {
      if(g_log_provider() != nullptr) g_log_provider()->Log(level_, msg_);
    }

    /**
     * @brief Get logging as serialized protobuf string.
     *
     * @param [out] log_  String to store the logging information.
     *
     * @return True if succeeded.
    **/
    bool GetLogging(std::string& log_)
    {
      if (g_log_udp_receiver() == nullptr)
        return false;
      return g_log_udp_receiver()->GetLogging(log_);
    }

    /**
     * @brief Get logging as struct.
     *
     * @param [out] log_  Target struct to store the logging information.
     *
     * @return True if succeeded.
    **/
    bool GetLogging(Logging::SLogging& log_)
    {
      if (g_log_udp_receiver() == nullptr)
        return false;
      g_log_udp_receiver()->GetLogging(log_);
      return true;
    }
  }
}
