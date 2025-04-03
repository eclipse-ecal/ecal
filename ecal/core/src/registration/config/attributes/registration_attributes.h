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

#include <chrono>
#include <string>
#include <ecal/types/custom_data_types.h>
#include <ecal/config/configuration.h>

namespace eCAL
{
  namespace Registration
  {
    enum class eTransportMode
    {
      shm,
      udp
    };

    struct SUDPAttributes
    {
      bool           broadcast;
      int            port;
      int            sendbuffer;
      int            receivebuffer;
      std::string    group;
      int            ttl;
    };

    struct SSHMAttributes
    {
      std::string    domain;
      size_t         queue_size;
    };

    struct SAttributes
    {
      std::chrono::milliseconds timeout;
      eTransportMode            transport_mode;
      bool                      network_enabled;
      bool                      loopback;
      unsigned int              refresh;
      std::string               host_name;
      std::string               shm_transport_domain;
      int                       process_id;

      SUDPAttributes            udp;
      SSHMAttributes            shm;
    };
  }
}
