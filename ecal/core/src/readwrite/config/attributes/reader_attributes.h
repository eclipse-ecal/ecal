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

#pragma once

#include <string>
#include <cstddef>
#include "ecal/types/custom_data_types.h"

namespace eCAL
{
  namespace eCALReader
  {
    struct SUDPModeAttributes
    {
      std::string group;
    };

    struct SUDPAttributes
    {
      bool           enable;
      Types::UDPMode mode;
      int            port;
      int            receivebuffer;
      SUDPModeAttributes   network;
      SUDPModeAttributes   local;
    };

    struct STCPAttributes
    {
      bool   enable;
      size_t thread_pool_size;
      int    max_reconnection_attempts;
    };

    struct SSHMAttributes
    {
      bool enable;
    };

    struct SAttributes
    {
      bool         network_enabled;
      bool         drop_out_of_order_messages;
      bool         loopback;
      unsigned int registration_timeout_ms;

      SUDPAttributes udp;
      STCPAttributes tcp;
      SSHMAttributes shm;

      std::string topic_name;
      std::string host_name;
      std::string shm_transport_domain;
      int         process_id;
      std::string process_name;
      std::string unit_name;
      bool        share_topic_type;
      bool        share_topic_description;
    };
  }
}