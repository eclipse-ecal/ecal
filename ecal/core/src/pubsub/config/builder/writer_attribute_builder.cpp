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
#include "ecal/config.h"

namespace eCAL
{
  eCALWriter::SAttributes BuildWriterAttributes(const std::string& topic_name_, const eCAL::Configuration& config_)
  {
    const auto& publisher_config        = config_.publisher;
    const auto& transport_tlayer_config = config_.transport_layer;
    const auto& registration_config     = config_.registration;

    eCALWriter::SAttributes attributes;

    attributes.network_enabled         = config_.communication_mode == eCAL::eCommunicationMode::network;
    attributes.loopback                = registration_config.loopback;

    attributes.layer_priority_local    = publisher_config.layer_priority_local;
    attributes.layer_priority_remote   = publisher_config.layer_priority_remote;

    attributes.host_name            = Process::GetHostName();
    attributes.shm_transport_domain = Process::GetShmTransportDomain();
    attributes.process_id           = Process::GetProcessID();
    attributes.process_name         = Process::GetProcessName();
    attributes.unit_name            = Process::GetUnitName();
    attributes.topic_name           = topic_name_;

    attributes.shm.enable                  = publisher_config.layer.shm.enable;
    attributes.shm.acknowledge_timeout_ms  = publisher_config.layer.shm.acknowledge_timeout_ms;
    attributes.shm.memfile_buffer_count    = publisher_config.layer.shm.memfile_buffer_count;
    attributes.shm.memfile_min_size_bytes  = publisher_config.layer.shm.memfile_min_size_bytes;
    attributes.shm.memfile_reserve_percent = publisher_config.layer.shm.memfile_reserve_percent;
    attributes.shm.zero_copy_mode          = publisher_config.layer.shm.zero_copy_mode;

    attributes.udp.enable        = publisher_config.layer.udp.enable;
    attributes.udp.broadcast     = config_.communication_mode == eCAL::eCommunicationMode::local;
    attributes.udp.port          = transport_tlayer_config.udp.port;
    attributes.udp.send_buffer   = transport_tlayer_config.udp.send_buffer;
    attributes.udp.group         = transport_tlayer_config.udp.network.group;
    attributes.udp.ttl           = transport_tlayer_config.udp.network.ttl;

    attributes.tcp.enable           = publisher_config.layer.tcp.enable;
    attributes.tcp.thread_pool_size = transport_tlayer_config.tcp.number_executor_writer;
    
    return attributes;
  }
}