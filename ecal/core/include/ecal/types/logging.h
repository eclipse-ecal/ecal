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
 * @file   logging.h
 * @brief  eCAL logging struct interface
**/

#pragma once

#include <ecal/ecal_log_level.h>

#include <cstdint>
#include <list>
#include <string>

namespace eCAL
{
  namespace Logging
  {
    struct SLogMessage
    {
      int64_t                time  = 0;               // time
      std::string            hname;                   // host name
      int32_t                pid   = 0;               // process id
      std::string            pname;                   // process name
      std::string            uname;                   // unit name
      eCAL_Logging_eLogLevel level = log_level_none;  // message level
      std::string            content;                 // message content
    };

    struct SLogging
    {
      std::list<SLogMessage>  log_messages;            // log messages
    };
  }
}
