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
 * @file   ecal_application_config.h
 * @brief  eCAL configuration for applications
**/

#pragma once

#include <string>
#include <ecal/ecal_os.h>
#include <cstddef>

namespace eCAL
{
  namespace Application
  {
    namespace Sys
    {
      struct Configuration
      {
        std::string filter_excl { "^eCALSysClient$|^eCALSysGUI$|^eCALSys$*" }; //!< Apps blacklist to be excluded when importing tasks from cloud
      };
    }

    namespace Startup
    {
      struct Configuration
      {
        std::string terminal_emulator { "" };                                  //!< Linux only command for starting applications with an external terminal emulator
      };
    }

    struct Configuration
    {
      Sys::Configuration     sys;
      Startup::Configuration startup;
    };
  }
}