/* ========================= eCAL LICENSE =================================
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
 * ========================= eCAL LICENSE =================================
*/

#pragma once

#include <string>
#include "ecal/types/ecal_custom_data_types.h"

namespace eCAL
{
  namespace eCALReader
  {
    struct SUDPModeAttributes
    {
      std::string group;
      int         ttl;
    };

    struct SUDPAttributes
    {
      Types::UDPMode mode;
      int            port;
      int            sendbuffer;
      int            receivebuffer;
      SUDPModeAttributes   network;
      SUDPModeAttributes   local;
    };

    struct SAttributes
    {
      bool   enable_tcp;
      bool   enable_udp;
      bool   enable_shm;
      bool   network_enabled;
      bool   drop_out_of_order_messages;
      bool   loopback;
      int    max_reconnection_attempts;
      size_t thread_pool_size;

      SUDPAttributes udp;
    };
  }
}