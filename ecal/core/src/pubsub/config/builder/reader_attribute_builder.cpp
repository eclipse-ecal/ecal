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

#include "reader_attribute_builder.h"
#include "ecal/process.h"

namespace eCAL
{
  eCALReader::SAttributes BuildReaderAttributes(const std::string& topic_name_, const Subscriber::Configuration& sub_config_, const eCAL::TransportLayer::Configuration& tl_config_, const eCAL::Registration::Configuration& reg_config_)
  {
    eCALReader::SAttributes attributes;

    attributes.network_enabled            = reg_config_.network_enabled;
    attributes.loopback                   = reg_config_.loopback;
    attributes.drop_out_of_order_messages = sub_config_.drop_out_of_order_messages;
    attributes.registration_timeout_ms    = reg_config_.registration_timeout;
    attributes.topic_name                 = topic_name_;
    attributes.host_name                  = Process::GetHostName();
    attributes.shm_transport_domain       = Process::GetShmTransportDomain();
    attributes.process_id                 = Process::GetProcessID();
    attributes.process_name               = Process::GetProcessName();
    attributes.unit_name                  = Process::GetUnitName();

    attributes.udp.enable        = sub_config_.layer.udp.enable;
    attributes.udp.mode          = tl_config_.udp.mode;
    attributes.udp.port          = tl_config_.udp.port;
    attributes.udp.receivebuffer = tl_config_.udp.receive_buffer;

    attributes.udp.local.group   = tl_config_.udp.local.group;

    attributes.udp.network.group = tl_config_.udp.network.group;

    attributes.tcp.enable                    = sub_config_.layer.tcp.enable;
    attributes.tcp.thread_pool_size          = tl_config_.tcp.number_executor_reader;
    attributes.tcp.max_reconnection_attempts = tl_config_.tcp.max_reconnections;
    
    attributes.shm.enable = sub_config_.layer.shm.enable;
    
    return attributes;
  }
}