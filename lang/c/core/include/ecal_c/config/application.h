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
  * @file   config/application.h
  * @brief  eCAL configuration for applications
 **/

#ifndef ecal_c_config_application_h_included
#define ecal_c_config_application_h_included

struct eCAL_Application_Sys_Configuration
{
  const char* filter_excl; //!< Apps blacklist to be excluded when importing tasks from cloud
};

struct eCAL_Application_Startup_Configuration
{
  const char* terminal_emulator; //!< Linux only command for starting applications with an external terminal emulator
};

struct eCAL_Application_Configuration
{
  struct eCAL_Application_Sys_Configuration sys;
  struct eCAL_Application_Startup_Configuration startup;
};

#endif /* ecal_c_config_application_h_included */