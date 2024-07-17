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
 * @file   transport_layer.h
 * @brief  eCAL configuration for the transport layer
**/

#pragma once

#include <ecal/types/ecal_custom_data_types.h>
#include "layer.h"

namespace eCAL
{
  namespace TransportLayer
  {
    namespace UDP
    {
      struct Configuration : eCAL::Layer::UDP::Configuration
      {
        Types::IpAddressV4                       mask{};                        /*!< v1: Mask maximum number of dynamic multicast group (Default: 0.0.0.1-0.0.0.255)
                                                                                     v2: masks are now considered like routes masking (Default: 255.0.0.0-255.255.255.255)*/
      };
    }

    namespace TCP
    {
      struct Configuration : eCAL::Layer::TCP::Configuration
      {

      };
    }

    namespace SHM 
    {
      struct Configuration : eCAL::Layer::SHM::Configuration
      {

      };
    }


    struct Configuration
    {
      UDP::Configuration        udp;
      TCP::Configuration        tcp;
      SHM::Configuration        shm;
      // bool                   network_enabled{};               /*!<eCAL components communicate over network boundaries*/

      // std::string            host_group_name{};               
    };
  }
}