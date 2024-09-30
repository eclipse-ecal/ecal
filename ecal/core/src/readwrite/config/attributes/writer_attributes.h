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
#include <cstddef>
#include <vector>

#include <ecal/ecal_tlayer.h>

#include "ecal/types/ecal_custom_data_types.h"

namespace eCAL
{
  namespace eCALWriter
  {
    struct SUDPModeAttributes
    {
      std::string group;
      int         ttl;
    };

    struct SUDPAttributes
    {
      bool           enable;
      Types::UDPMode mode;
      int            port;
      int            send_buffer;
      SUDPModeAttributes   network;
      SUDPModeAttributes   local;
    };

    struct STCPAttributes
    {
      bool   enable;
      size_t thread_pool_size;
    };

    struct SSHMAttributes
    {
      bool         enable;
      bool         zero_copy_mode;
      unsigned int acknowledge_timeout_ms;
      unsigned int memfile_buffer_count;
      unsigned int memfile_min_size_bytes;
      unsigned int memfile_reserve_percent;
    };


    struct SAttributes
    {
      using LayerPriorityVector = std::vector<TLayer::eTransportLayer>;
      LayerPriorityVector  layer_priority_local;
      LayerPriorityVector  layer_priority_remote;
      
      bool                 share_topic_type;
      bool                 share_topic_description;

      bool                 network_enabled;
      bool                 loopback;

      std::string          host_name;
      std::string          host_group_name;
      int                  process_id;
      std::string          process_name;

      std::string          unit_name;
      std::string          topic_name;

      SUDPAttributes     udp;
      STCPAttributes     tcp;
      SSHMAttributes     shm;
    };
  }
}