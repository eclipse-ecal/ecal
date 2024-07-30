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

namespace eCAL
{
  namespace Monitoring
  {
    struct Configuration
    {
      eCAL::Types::ConstrainedInteger<1000, 1000> timeout     { 5000U };     //!< Timeout for topic monitoring in ms (Default: 5000)
      std::string                                 filter_excl {  "^__.*$" }; //!< Topics blacklist as regular expression (will not be monitored) (Default: "^__.*$")
      std::string                                 filter_incl { "" };        //!< Topics whitelist as regular expression (will be monitored only) (Default: "")
    };
  }
}