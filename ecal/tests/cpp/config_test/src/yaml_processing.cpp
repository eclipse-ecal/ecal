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

#include <ecal/core.h>
#include <ecal/config.h>
#include "ini_file.h"

#include <gtest/gtest.h>

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <stdio.h>
#include <string>
#include <vector>

#ifdef ECAL_CORE_CONFIGURATION
  #include "configuration_reader.h"
#endif
#include "default_configuration.h"
#include "ecal_def.h"


TEST(core_cpp_config /*unused*/, yaml_processing_comparison /*unused*/)
{
    eCAL::Configuration config;

    // Create config with every value set differently than the default values
    config.communication_mode = eCAL::eCommunicationMode::network;

    config.registration.registration_refresh = 500;
    config.registration.registration_timeout = 2000;
    config.registration.loopback = false;
    config.registration.shm_transport_domain = "shm_transport_domain";
    config.registration.local.transport_type = eCAL::Registration::Local::eTransportType::shm;
    config.registration.local.shm.domain = "ecal_don";
    config.registration.local.shm.queue_size = 2048;
    config.registration.local.udp.port = 15000;
    // There is unfortunately only one transport type
    config.registration.network.transport_type = eCAL::Registration::Network::eTransportType::udp;
    config.registration.network.udp.port = 16000;
    
    config.transport_layer.udp.config_version = eCAL::Types::UdpConfigVersion::V1;
    config.transport_layer.udp.port = 17000;
    config.transport_layer.udp.mask = "255.254.254.242";
    config.transport_layer.udp.send_buffer = 6242880;
    config.transport_layer.udp.receive_buffer = 6242881;
    config.transport_layer.udp.join_all_interfaces = true;
    config.transport_layer.udp.npcap_enabled = true;
    config.transport_layer.udp.local.group = "129.255.255.254";
    config.transport_layer.udp.local.ttl = 7;
    config.transport_layer.udp.network.group = "238.1.2.3";
    config.transport_layer.udp.network.ttl = 8;
    config.transport_layer.tcp.number_executor_reader = 9;
    config.transport_layer.tcp.number_executor_writer = 10;
    config.transport_layer.tcp.max_reconnections = 11;

    config.publisher.layer.shm.enable = false;
    config.publisher.layer.shm.zero_copy_mode = true;
    config.publisher.layer.shm.acknowledge_timeout_ms = 12;
    config.publisher.layer.shm.memfile_buffer_count = 13;
    config.publisher.layer.shm.memfile_min_size_bytes = 8192;
    config.publisher.layer.shm.memfile_reserve_percent = 14;
    config.publisher.layer.udp.enable = false;
    config.publisher.layer.tcp.enable = false;
    config.publisher.layer_priority_local = {eCAL::TransportLayer::eType::tcp, eCAL::TransportLayer::eType::shm, eCAL::TransportLayer::eType::udp_mc};
    config.publisher.layer_priority_remote = {eCAL::TransportLayer::eType::tcp, eCAL::TransportLayer::eType::udp_mc};

    config.subscriber.layer.shm.enable = false;
    config.subscriber.layer.udp.enable = false;
    config.subscriber.layer.tcp.enable = true;
    config.subscriber.drop_out_of_order_messages = false;

    config.timesync.timesync_module_replay = "my_replay";
    config.timesync.timesync_module_rt = "my_rt";

    config.application.startup.terminal_emulator = "term_emulator";
    config.application.sys.filter_excl = "filter_excl";
    
    config.logging.provider.console.enable = false;
    config.logging.provider.console.log_level = eCAL::Logging::eLogLevel::log_level_debug1;
    config.logging.provider.file.enable = true;
    config.logging.provider.file.log_level = eCAL::Logging::eLogLevel::log_level_debug2 | eCAL::Logging::eLogLevel::log_level_debug3;
    config.logging.provider.udp.enable = false;
    config.logging.provider.udp.log_level = eCAL::Logging::eLogLevel::log_level_debug4;
    config.logging.provider.file_config.path = "file_config_path";
    config.logging.provider.udp_config.port = 18000;
    config.logging.receiver.enable = true;
    config.logging.receiver.udp_config.port = 19000;

    const auto yaml_string = eCAL::Config::getConfigAsYamlSS(config);
    eCAL::Configuration config_from_yaml;
    eCAL::Config::YamlStringToConfig(yaml_string.str(), config_from_yaml);

    EXPECT_EQ(config.communication_mode, config_from_yaml.communication_mode);
    EXPECT_EQ(config.registration.registration_refresh, config_from_yaml.registration.registration_refresh);
    EXPECT_EQ(config.registration.registration_timeout, config_from_yaml.registration.registration_timeout);
    EXPECT_EQ(config.registration.loopback, config_from_yaml.registration.loopback);
    EXPECT_EQ(config.registration.shm_transport_domain, config_from_yaml.registration.shm_transport_domain);
    EXPECT_EQ(config.registration.local.transport_type, config_from_yaml.registration.local.transport_type);
    EXPECT_EQ(config.registration.local.shm.domain, config_from_yaml.registration.local.shm.domain);
    EXPECT_EQ(config.registration.local.shm.queue_size, config_from_yaml.registration.local.shm.queue_size);
    EXPECT_EQ(config.registration.local.udp.port, config_from_yaml.registration.local.udp.port);
    EXPECT_EQ(config.registration.network.transport_type, config_from_yaml.registration.network.transport_type);
    EXPECT_EQ(config.registration.network.udp.port, config_from_yaml.registration.network.udp.port);
    EXPECT_EQ(config.transport_layer.udp.config_version, config_from_yaml.transport_layer.udp.config_version);
    EXPECT_EQ(config.transport_layer.udp.port, config_from_yaml.transport_layer.udp.port);
    EXPECT_EQ(config.transport_layer.udp.mask, config_from_yaml.transport_layer.udp.mask);
    EXPECT_EQ(config.transport_layer.udp.send_buffer, config_from_yaml.transport_layer.udp.send_buffer);
    EXPECT_EQ(config.transport_layer.udp.receive_buffer, config_from_yaml.transport_layer.udp.receive_buffer);
    EXPECT_EQ(config.transport_layer.udp.join_all_interfaces, config_from_yaml.transport_layer.udp.join_all_interfaces);
    EXPECT_EQ(config.transport_layer.udp.npcap_enabled, config_from_yaml.transport_layer.udp.npcap_enabled);
    EXPECT_EQ(config.transport_layer.udp.local.group, config_from_yaml.transport_layer.udp.local.group);
    EXPECT_EQ(config.transport_layer.udp.local.ttl, config_from_yaml.transport_layer.udp.local.ttl);
    EXPECT_EQ(config.transport_layer.udp.network.group, config_from_yaml.transport_layer.udp.network.group);
    EXPECT_EQ(config.transport_layer.udp.network.ttl, config_from_yaml.transport_layer.udp.network.ttl);
    EXPECT_EQ(config.transport_layer.tcp.number_executor_reader, config_from_yaml.transport_layer.tcp.number_executor_reader);
    EXPECT_EQ(config.transport_layer.tcp.number_executor_writer, config_from_yaml.transport_layer.tcp.number_executor_writer);
    EXPECT_EQ(config.transport_layer.tcp.max_reconnections, config_from_yaml.transport_layer.tcp.max_reconnections);
    EXPECT_EQ(config.publisher.layer.shm.enable, config_from_yaml.publisher.layer.shm.enable);
    EXPECT_EQ(config.publisher.layer.shm.zero_copy_mode, config_from_yaml.publisher.layer.shm.zero_copy_mode);
    EXPECT_EQ(config.publisher.layer.shm.acknowledge_timeout_ms, config_from_yaml.publisher.layer.shm.acknowledge_timeout_ms);
    EXPECT_EQ(config.publisher.layer.shm.memfile_buffer_count, config_from_yaml.publisher.layer.shm.memfile_buffer_count);
    EXPECT_EQ(config.publisher.layer.shm.memfile_min_size_bytes, config_from_yaml.publisher.layer.shm.memfile_min_size_bytes);
    EXPECT_EQ(config.publisher.layer.shm.memfile_reserve_percent, config_from_yaml.publisher.layer.shm.memfile_reserve_percent);
    EXPECT_EQ(config.publisher.layer.udp.enable, config_from_yaml.publisher.layer.udp.enable);
    EXPECT_EQ(config.publisher.layer.tcp.enable, config_from_yaml.publisher.layer.tcp.enable);
    EXPECT_EQ(config.publisher.layer_priority_local, config_from_yaml.publisher.layer_priority_local);
    EXPECT_EQ(config.publisher.layer_priority_remote, config_from_yaml.publisher.layer_priority_remote);
    EXPECT_EQ(config.subscriber.layer.shm.enable, config_from_yaml.subscriber.layer.shm.enable);
    EXPECT_EQ(config.subscriber.layer.udp.enable, config_from_yaml.subscriber.layer.udp.enable);
    EXPECT_EQ(config.subscriber.layer.tcp.enable, config_from_yaml.subscriber.layer.tcp.enable);
    EXPECT_EQ(config.subscriber.drop_out_of_order_messages, config_from_yaml.subscriber.drop_out_of_order_messages);
    EXPECT_EQ(config.timesync.timesync_module_replay, config_from_yaml.timesync.timesync_module_replay);
    EXPECT_EQ(config.timesync.timesync_module_rt, config_from_yaml.timesync.timesync_module_rt);
    EXPECT_EQ(config.application.startup.terminal_emulator, config_from_yaml.application.startup.terminal_emulator);
    EXPECT_EQ(config.application.sys.filter_excl, config_from_yaml.application.sys.filter_excl);
    EXPECT_EQ(config.logging.provider.console.enable, config_from_yaml.logging.provider.console.enable);
    EXPECT_EQ(config.logging.provider.console.log_level, config_from_yaml.logging.provider.console.log_level);
    EXPECT_EQ(config.logging.provider.file.enable, config_from_yaml.logging.provider.file.enable);
    EXPECT_EQ(config.logging.provider.file.log_level, config_from_yaml.logging.provider.file.log_level);
    EXPECT_EQ(config.logging.provider.udp.enable, config_from_yaml.logging.provider.udp.enable);
    EXPECT_EQ(config.logging.provider.udp.log_level, config_from_yaml.logging.provider.udp.log_level);
    EXPECT_EQ(config.logging.provider.file_config.path, config_from_yaml.logging.provider.file_config.path);
    EXPECT_EQ(config.logging.provider.udp_config.port, config_from_yaml.logging.provider.udp_config.port);
    EXPECT_EQ(config.logging.receiver.enable, config_from_yaml.logging.receiver.enable);
    EXPECT_EQ(config.logging.receiver.udp_config.port, config_from_yaml.logging.receiver.udp_config.port);
}