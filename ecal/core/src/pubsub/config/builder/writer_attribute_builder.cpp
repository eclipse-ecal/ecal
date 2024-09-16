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

#include "writer_attribute_builder.h"

namespace eCAL
{
  eCALWriter::SAttributes BuildWriterAttributes(const Publisher::Configuration& pub_config_, const eCAL::TransportLayer::UDP::Configuration& tl_udp_config_, const eCAL::Registration::Configuration& reg_config_)
  {
    eCALWriter::SAttributes attributes;

    attributes.network_enabled         = reg_config_.network_enabled;
    attributes.loopback                = reg_config_.loopback;

    attributes.share_topic_type        = pub_config_.share_topic_type;
    attributes.share_topic_description = pub_config_.share_topic_description;
    attributes.layer_priority_local    = pub_config_.layer_priority_local;
    attributes.layer_priority_remote   = pub_config_.layer_priority_remote;

    attributes.shm.enable                  = pub_config_.layer.shm.enable;
    attributes.shm.acknowledge_timeout_ms  = pub_config_.layer.shm.acknowledge_timeout_ms;
    attributes.shm.memfile_buffer_count    = pub_config_.layer.shm.memfile_buffer_count;
    attributes.shm.memfile_min_size_bytes  = pub_config_.layer.shm.memfile_min_size_bytes;
    attributes.shm.memfile_reserve_percent = pub_config_.layer.shm.memfile_reserve_percent;

    attributes.udp.enable        = pub_config_.layer.udp.enable;
    attributes.udp.port          = tl_udp_config_.port;
    attributes.udp.sendbuffer    = tl_udp_config_.send_buffer;
    attributes.udp.receivebuffer = tl_udp_config_.receive_buffer;
    attributes.udp.mode          = tl_udp_config_.mode;

    attributes.udp.network.group = tl_udp_config_.network.group;
    attributes.udp.network.ttl   = tl_udp_config_.network.ttl;

    attributes.udp.local.group   = tl_udp_config_.local.group;
    attributes.udp.local.ttl     = tl_udp_config_.local.ttl;

    attributes.tcp.enable = pub_config_.layer.tcp.enable;
    
    return attributes;
  }
}