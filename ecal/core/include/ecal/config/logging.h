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

namespace
{
  // After switchting to c++17, this can be replaced by an inline constexpr
  static const eCAL_Logging_Filter log_level_default = log_level_info | log_level_warning | log_level_error | log_level_fatal;
}

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
          bool                enable         { true };                               //!< Enable console logging (Default: true)
          eCAL_Logging_Filter filter_log_con { log_level_error | log_level_fatal };  /*!< Log messages logged to console (all, info, warning, error, fatal, debug1, debug2, debug3, debug4)
                                                                                          (Default: info, warning, error, fatal)*/
        };
      }

      namespace File
      {
        struct Configuration
        {
          bool                enable          { false };             //!< Enable file logging (Default: false)
          std::string         path            { "" };                //!< Path to log file (Default: "")
          eCAL_Logging_Filter filter_log_file { log_level_none };    /*!< Log messages logged into file system (all, info, warning, error, fatal, debug1, debug2, debug3, debug4)
                                                                          (Default: info, warning, error, fatal)*/
        };
      }

      namespace UDP
      {
        struct Configuration
        {
          bool                enable         { true };               //!< Enable UDP logging (Default: false)
          unsigned int        port           { 14001 };              //!< UDP port number (Default: 14001)
          eCAL_Logging_Filter filter_log_udp { log_level_default };  //!< Log messages logged via udp network (Default: info, warning, error, fatal)
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