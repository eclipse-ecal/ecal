/* =========================== LICENSE =================================
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
 * =========================== LICENSE =================================
 */

 /**
  * @file   config/logging.h
  * @brief  eCAL configuration for logging
 **/

#ifndef ecal_c_config_logging_h_included
#define ecal_c_config_logging_h_included

#include <ecal_c/log_level.h>

struct eCAL_Logging_Provider_Sink
{
  int enable;                //!< Enable sink
  eCAL_Logging_Filter log_level; //!< Log messages logged (all, info, warning, error, fatal, debug1, debug2, debug3, debug4)
};

struct eCAL_Logging_Provider_File_Configuration
{
  const char* path;                //!< Path to log file (Default: "")
};

struct eCAL_Logging_Provider_UDP_Configuration
{
  unsigned int port;         //!< UDP port number (Default: 14001)
};

struct eCAL_Logging_Provider_Configuration
{
  struct eCAL_Logging_Provider_Sink console; //!< default: true, log_level_warning | log_level_error | log_level_fatal
  struct eCAL_Logging_Provider_Sink file;    //!< default: false, log_level_none
  struct eCAL_Logging_Provider_Sink udp;     //!< default: true, log_level_info | log_level_warning | log_level_error | log_level_fatal

  struct eCAL_Logging_Provider_File_Configuration file_config;
  struct eCAL_Logging_Provider_UDP_Configuration udp_config;
};

struct eCAL_Logging_Receiver_UDP_Configuration
{
  unsigned int port;         //!< UDP port number (Default: 14001)
};

struct eCAL_Logging_Receiver_Configuration
{
  int enable;                //!< Enable UDP receiver (Default: false)
  struct eCAL_Logging_Receiver_UDP_Configuration udp_config;
};

struct eCAL_Logging_Configuration
{
  struct eCAL_Logging_Provider_Configuration provider;
  struct eCAL_Logging_Receiver_Configuration receiver;
};

#endif /* ecal_c_config_logging_h_included */