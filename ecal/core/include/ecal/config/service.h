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
 * @file   ecal_service_config.h
 * @brief  eCAL configuration for services
**/

#pragma once

#include <string>

namespace eCAL
{
  namespace Service
  {
    struct Configuration
    {
      bool protocol_v0 { false }; //!< Support service protocol v0, eCAL 5.11 and older (Default: false)
      bool protocol_v1 { true };  //!< Support service protocol v1, eCAL 5.12 and newer (Default: true)
    };
  }
}