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

#include "writer_attribute_builder.h"
#include "ecal/process.h"

namespace eCAL
{
  eCALWriter::SAttributes BuildWriterAttributes(const std::string& topic_name_, const Publisher::Configuration& pub_config_, const eCAL::TransportLayer::Configuration& tl_config_, const eCAL::Registration::Configuration& reg_config_)
  {
    eCALWriter::SAttributes attributes;

    attributes.network_enabled         = reg_config_.network_enabled;
    attributes.loopback                = reg_config_.loopback;

    attributes.layer_priority_local    = pub_config_.layer_priority_local;
    attributes.layer_priority_remote   = pub_config_.layer_priority_remote;

    attributes.host_name            = Process::GetHostName();
    attributes.shm_transport_domain = Process::GetShmTransportDomain();
    attributes.process_id           = Process::GetProcessID();
    attributes.process_name         = Process::GetProcessName();

    attributes.unit_name       = Process::GetUnitName();
    attributes.topic_name      = topic_name_;

    attributes.shm.enable                  = pub_config_.layer.shm.enable;
    attributes.shm.acknowledge_timeout_ms  = pub_config_.layer.shm.acknowledge_timeout_ms;
    attributes.shm.memfile_buffer_count    = pub_config_.layer.shm.memfile_buffer_count;
    attributes.shm.memfile_min_size_bytes  = pub_config_.layer.shm.memfile_min_size_bytes;
    attributes.shm.memfile_reserve_percent = pub_config_.layer.shm.memfile_reserve_percent;
    attributes.shm.zero_copy_mode          = pub_config_.layer.shm.zero_copy_mode;

    attributes.udp.enable        = pub_config_.layer.udp.enable;
    attributes.udp.port          = tl_config_.udp.port;
    attributes.udp.send_buffer   = tl_config_.udp.send_buffer;
    attributes.udp.mode          = tl_config_.udp.mode;

    attributes.udp.network.group = tl_config_.udp.network.group;
    attributes.udp.network.ttl   = tl_config_.udp.network.ttl;

    attributes.udp.local.group   = tl_config_.udp.local.group;
    attributes.udp.local.ttl     = tl_config_.udp.local.ttl;

    attributes.tcp.enable           = pub_config_.layer.tcp.enable;
    attributes.tcp.thread_pool_size = tl_config_.tcp.number_executor_writer;
    
    return attributes;
  }
}