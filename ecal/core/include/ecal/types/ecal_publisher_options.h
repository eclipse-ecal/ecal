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
 * @file   ecal_publisher_options.h
 * @brief  eCAL options for configuration of the publisher
**/

#pragma once

#include <ecal/ecal_tlayer.h>

namespace eCAL
{
  namespace Config
  {
    struct PublisherOptions
    {
      TLayer::eSendMode use_inproc;
      TLayer::eSendMode use_shm;
      TLayer::eSendMode use_tcp;
      TLayer::eSendMode use_udp_mc;
    };
  }
}