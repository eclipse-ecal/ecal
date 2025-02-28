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
  Registration::SAttributes BuildRegistrationAttributes(const eCAL::Configuration& config_, int process_id_)
  {
    const auto& reg_config_    = config_.registration;
    const auto& tl_udp_config_ = config_.transport_layer.udp;

    Registration::SAttributes attr;
    
    attr.timeout              = std::chrono::milliseconds(reg_config_.registration_timeout);
    attr.refresh              = reg_config_.registration_refresh;
    attr.communication_mode   = config_.communication_mode;
    attr.loopback             = reg_config_.loopback;
    attr.host_name            = eCAL::Process::GetHostName();
    attr.shm_transport_domain = reg_config_.shm_transport_domain;

    attr.process_id        = process_id_;

    attr.shm.domain        = reg_config_.local.shm.domain;
    attr.shm.queue_size    = reg_config_.local.shm.queue_size;
     
    switch (config_.communication_mode)
    {
      case eCAL::eCommunicationMode::network:
        attr.udp.port       = reg_config_.network.udp.port;
        attr.udp.group      = tl_udp_config_.network.group;
        attr.udp.ttl        = tl_udp_config_.network.ttl;
        attr.transport_type = reg_config_.network.transport_type;
        break;
      case eCAL::eCommunicationMode::local:
        attr.udp.port       = reg_config_.local.udp.port;
        attr.udp.group      = tl_udp_config_.local.group;
        attr.udp.ttl        = tl_udp_config_.local.ttl;
        attr.transport_type = reg_config_.local.transport_type;
        break;
      default:
        break;
    }

    attr.udp.sendbuffer    = tl_udp_config_.send_buffer;
    attr.udp.receivebuffer = tl_udp_config_.receive_buffer;
    
    return attr;
  }
}