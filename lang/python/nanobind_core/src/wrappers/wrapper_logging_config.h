/* ========================= eCAL LICENSE =================================
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
 * ========================= eCAL LICENSE =================================
*/

/**
 * @file   wrapper_publisher_config.h
 * @brief  Nanobind wrapper for structs of Publisher config
**/

#pragma once

#include <ecal/ecal.h>
#include <ecal/config/logging.h>
#include <ecal/ecal_os.h>
#include <ecal/types/ecal_custom_data_types.h>

#include <stdint.h>
#include <string>
#include <cstddef>
#include <nanobind/nanobind.h>

namespace eCAL
{
    namespace Logging
    {
        namespace Sinks
        {
            namespace Console
            {
                struct CNBConsoleConfiguration
                {
                    bool                enable{ true };                               //!< Enable console logging (Default: true)
                    eCAL_Logging_Filter filter_log_con{ log_level_error | log_level_fatal };  /*!< Log messages logged to console (all, info, warning, error, fatal, debug1, debug2, debug3, debug4)
                                                                                                    (Default: info, warning, error, fatal)*/
                };
            }

            namespace File
            {
                struct CNBFileConfiguration
                {
                    bool                enable{ false };             //!< Enable file logging (Default: false)
                    std::string         path{ "" };                //!< Path to log file (Default: "")
                    eCAL_Logging_Filter filter_log_file{ log_level_none };    /*!< Log messages logged into file system (all, info, warning, error, fatal, debug1, debug2, debug3, debug4)
                                                                                    (Default: info, warning, error, fatal)*/
                };
            }

            namespace UDP
            {
                struct CNBUDPConfiguration
                {
                    bool                enable{ true };               //!< Enable UDP logging (Default: false)
                    unsigned int        port{ 14001 };              //!< UDP port number (Default: 14001)
                    eCAL_Logging_Filter filter_log_udp{ log_level_default };  //!< Log messages logged via udp network (Default: info, warning, error, fatal)
                };
            }

            struct CNBSinksConfiguration
            {
                // CNBLoggingConfiguration();

                Console::CNBConsoleConfiguration console;
                File::CNBFileConfiguration       file;
                UDP::CNBUDPConfiguration         udp;
            };
        }

        struct CNBLoggingConfiguration
        {
            Sinks::Configuration sinks;
        };
    }
}
