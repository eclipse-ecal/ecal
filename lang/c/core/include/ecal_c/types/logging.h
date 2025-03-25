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
  * @file   types/logging.h
  * @brief  eCAL logging types
 **/

#ifndef ecal_c_types_logging_h_included
#define ecal_c_types_logging_h_included

#include <stdint.h>
#include <ecal_c/log_level.h>

struct eCAL_Logging_SLogMessage
{
  int64_t     time;              // time
  const char* host_name;         // host name
  int32_t     process_id;        // process id
  const char* process_name;      // process name
  const char* unit_name;         // unit name
  enum eCAL_Logging_eLogLevel level; // message level
  const char* content;               // message content
};

struct  eCAL_Logging_SLogging
{
  struct eCAL_Logging_SLogMessage*  log_messages;            // log messages
  size_t log_messages_length;                         // array of log messages
};

#endif /*ecal_c_types_logging_h_included*/