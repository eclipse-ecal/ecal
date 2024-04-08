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
 * @file   ecal_logging_options.h
 * @brief  eCAL options for logging configuration
**/

#pragma once

#include <ecal/ecal_log_level.h>

namespace eCAL
{
  namespace Config
  {
    struct LoggingOptions
    {
      eCAL_Logging_Filter filter_log_con;
      eCAL_Logging_Filter filter_log_file;
      eCAL_Logging_Filter filter_log_udp;
    };
  }
}