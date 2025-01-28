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

namespace eCAL
{
  Registration::SAttributes BuildRegistrationAttributes(const eCAL::Registration::Configuration& reg_config_, const eCAL::TransportLayer::UDP::Configuration& tl_udp_confi_, int process_id_)
  {
    Registration::SAttributes attr;
    
    attr.timeout              = std::chrono::milliseconds(reg_config_.registration_timeout);
    attr.refresh              = reg_config_.registration_refresh;
    attr.network_enabled      = reg_config_.network_enabled;
    attr.loopback             = reg_config_.loopback;
    attr.host_name            = eCAL::Process::GetHostName();
    attr.shm_transport_domain = reg_config_.shm_transport_domain;

    attr.process_id        = process_id_;

    attr.shm_enabled       = reg_config_.layer.shm.enable;
    attr.udp_enabled       = reg_config_.layer.udp.enable;
    
    attr.shm.domain        = reg_config_.layer.shm.domain;
    attr.shm.queue_size    = reg_config_.layer.shm.queue_size;
     
    attr.udp.port          = reg_config_.layer.udp.port;
    attr.udp.sendbuffer    = tl_udp_confi_.send_buffer;
    attr.udp.receivebuffer = tl_udp_confi_.receive_buffer;
    attr.udp.mode          = tl_udp_confi_.mode;

    attr.udp.network.group = tl_udp_confi_.network.group;
    attr.udp.network.ttl   = tl_udp_confi_.network.ttl;

    attr.udp.local.group   = tl_udp_confi_.local.group;
    attr.udp.local.ttl     = tl_udp_confi_.local.ttl;
    
    return attr;
  }
}