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
 * @file   ecal_monitoring_config.h
 * @brief  eCAL configuration for monitoring
**/

#pragma once

#include <ecal/types/ecal_custom_data_types.h>

#include <optional>

namespace eCAL
{
  namespace MonitoringOptional
  {
    namespace Types
    {
      enum Mode
      {
        none = 0,
        udp_monitoring = 1 << 0,
        shm_monitoring = 1 << 1
      };

      using Mode_Filter = char;
    }

    namespace UDP
    {
      struct Configuration
      {
      };
    }

    namespace SHM
    {
      struct Configuration
      {
        std::optional<std::string> shm_monitoring_domain{};                                  //!< Domain name for shared memory based monitoring/registration (Default: ecal_mon)
        std::optional<size_t>      shm_monitoring_queue_size{};                              //!< Queue size of monitoring/registration events (Default: 1024)
      };
    }

    struct Configuration
    {
      std::optional<Types::Mode_Filter>                           monitoring_mode{};         //!< Specify which monitoring is enabled (Default: none)
      std::optional<eCAL::Types::ConstrainedInteger<1000, 1000>>  monitoring_timeout{};      //!< Timeout for topic monitoring in ms (Default: 5000)
      std::optional<bool>                                         network_monitoring{};      //!< Enable distribution of monitoring/registration information via network (Default: true)
      std::optional<UDP::Configuration>                           udp_options{};
      std::optional<SHM::Configuration>                           shm_options{};

      std::optional<std::string>         filter_excl{};                                      //!< Topics blacklist as regular expression (will not be monitored) (Default: "__.*")
      std::optional<std::string>         filter_incl{};                                      //!< Topics whitelist as regular expression (will be monitored only) (Default: "")
    };
  }
}