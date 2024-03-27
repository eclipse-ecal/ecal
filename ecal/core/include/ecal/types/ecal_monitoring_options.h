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
 * @file   ecal_monitoring_options.h
 * @brief  eCAL options for monitoring configuration
**/

#pragma once

#include "ecal_custom_data_types.h"

#include "../ecal_log_level.h"

namespace eCAL
{
  namespace Config
  {
    enum MonitoringMode
    {
      none = 0,
      udp_monitoring = 1 << 0,
      shm_monitoring = 1 << 1
    };

    using eCAL_MonitoringMode_Filter = char;

    struct UDPMonitoringOptions
    {
      // Here?
    };

    struct SHMMonitoringOptions
    {
      std::string shm_monitoring_domain;
      size_t      shm_monitoring_queue_size;  
    };

    struct MonitoringOptions
    {
      eCAL_MonitoringMode_Filter monitoring_mode;
      LimitSize<1000, 1000>      monitoring_timeout;
      bool                       network_monitoring_disabled;
      UDPMonitoringOptions       udp_options;
      SHMMonitoringOptions       shm_options;

      std::string         filter_excl;
      std::string         filter_incl;
      eCAL_Logging_Filter filter_log_con;
      eCAL_Logging_Filter filter_log_file;
      eCAL_Logging_Filter filter_log_udp;
    };
  }
}