/* =========================== LICENSE =================================
 *
 * Copyright (C) 2016 - 2024 Continental Corporation
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
 * =========================== LICENSE =================================
 */

/**
 * @file   ecal_logging_config.h
 * @brief  eCAL configuration for logging
**/

#pragma once

#include <ecal/ecal_log_level.h>

#include <vector>
#include <string>

namespace eCAL
{
  namespace Logging
  {
    namespace Sinks
    {
      namespace Console
      {
        struct Configuration
        {
          bool enable{};                                //!< Enable console logging (Default: false)
          eCAL_Logging_Filter filter_log_con{};         /*!< Log messages logged to console (all, info, warning, error, fatal, debug1, debug2, debug3, debug4)
                                                         (Default: info, warning, error, fatal)*/
        };
      }

      namespace File
      {
        struct Configuration
        {
          bool enable{};                                //!< Enable file logging (Default: false)
          std::string path{};                           //!< Path to log file (Default: "")
          eCAL_Logging_Filter filter_log_file{};        /*!< Log messages logged into file system (all, info, warning, error, fatal, debug1, debug2, debug3, debug4)
                                                         (Default: info, warning, error, fatal)*/
        };
      }

      namespace UDP
      {
        struct Configuration
        {
          bool enable{};                                //!< Enable UDP logging (Default: false)
          unsigned int port{};                          //!< UDP port number (Default: 14001)
          eCAL_Logging_Filter filter_log_udp{};         //!< Log messages logged via udp network (Default: info, warning, error, fatal)
        };
      }

      struct Configuration
      {
        Console::Configuration console;
        File::Configuration    file;
        UDP::Configuration     udp;
      };
    }
    
    struct Configuration
    {
      Sinks::Configuration sinks;
    };
  }
}