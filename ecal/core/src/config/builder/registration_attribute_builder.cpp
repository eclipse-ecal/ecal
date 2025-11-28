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

#include "registration_attribute_builder.h"
#include "ecal/process.h"
#include "ecal/config.h"

namespace eCAL
{
  Registration::SAttributes BuildRegistrationAttributes(const eCAL::Configuration& config_, ProcessID process_id_)
  {
    const auto& reg_config    = config_.registration;
    const auto& tl_udp_config = config_.transport_layer.udp;

    Registration::SAttributes attr;
    
    attr.network_enabled      = config_.communication_mode == eCAL::eCommunicationMode::network;
    attr.timeout              = std::chrono::milliseconds(reg_config.registration_timeout);
    attr.refresh              = reg_config.registration_refresh;
    attr.loopback             = reg_config.loopback;
    attr.host_name            = eCAL::Process::GetHostName();
    attr.shm_transport_domain = reg_config.shm_transport_domain;

    attr.process_id        = process_id_;

    attr.shm.domain        = reg_config.local.shm.domain;
    attr.shm.queue_size    = reg_config.local.shm.queue_size;
     
    switch (config_.communication_mode)
    {
      case eCAL::eCommunicationMode::network:
        attr.udp.port       = reg_config.network.udp.port;
        attr.udp.group      = tl_udp_config.network.group;
        attr.udp.ttl        = tl_udp_config.network.ttl;
        attr.udp.broadcast  = false;
        // Only udp transport type is supported for network communication right now
        attr.transport_mode = Registration::eTransportMode::udp;
        break;
      case eCAL::eCommunicationMode::local:
        attr.udp.port       = reg_config.local.udp.port;
        attr.udp.group      = tl_udp_config.local.group;
        attr.udp.ttl        = tl_udp_config.local.ttl;
        attr.udp.broadcast  = true;
        switch (reg_config.local.transport_type)
        {
          case Registration::Local::eTransportType::udp:
            attr.transport_mode = Registration::eTransportMode::udp;
            break;
          case Registration::Local::eTransportType::shm:
            attr.transport_mode = Registration::eTransportMode::shm;
            break;
          default:
            break;
        }
        break;
      default:
        break;
    }

    attr.udp.sendbuffer    = tl_udp_config.send_buffer;
    attr.udp.receivebuffer = tl_udp_config.receive_buffer;
    
    return attr;
  }
}