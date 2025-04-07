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

#include <ecal/ecal.h>

#include <ecal_c/config/configuration.h>

#include <map>

namespace
{
  /*
   *  Assigment and conversion functions
   *  from C++ to C
   * 
   */

  enum eCAL_TransportLayer_eType Convert_TransportLayer_eType(eCAL::TransportLayer::eType type_)
  {
    static const std::map<eCAL::TransportLayer::eType, enum eCAL_TransportLayer_eType> transport_layer_type_map
    {
      {eCAL::TransportLayer::eType::none, eCAL_TransportLayer_eType_none},
      {eCAL::TransportLayer::eType::shm, eCAL_TransportLayer_eType_shm},
      {eCAL::TransportLayer::eType::udp_mc, eCAL_TransportLayer_eType_udp_mc},
      {eCAL::TransportLayer::eType::tcp, eCAL_TransportLayer_eType_tcp}
    };
    return transport_layer_type_map.at(type_);
  }

  enum eCAL_Registration_Local_eTransportType Convert_Registration_Local_eTransportType(eCAL::Registration::Local::eTransportType transport_type_)
  {
    static const std::map<eCAL::Registration::Local::eTransportType, enum eCAL_Registration_Local_eTransportType> transport_type_map
    {
      {eCAL::Registration::Local::eTransportType::shm, eCAL_Registration_Local_eTransportType_shm},
      {eCAL::Registration::Local::eTransportType::udp, eCAL_Registration_Local_eTransportType_udp},
    };
    return transport_type_map.at(transport_type_);
  }

  enum eCAL_Registration_Network_eTransportType Convert_Registration_Network_eTransportType(eCAL::Registration::Network::eTransportType transport_type_)
  {
    static const std::map<eCAL::Registration::Network::eTransportType, enum eCAL_Registration_Network_eTransportType> transport_type_map
    {
      {eCAL::Registration::Network::eTransportType::udp, eCAL_Registration_Network_eTransportType_udp}
    };
    return transport_type_map.at(transport_type_);
  }

  enum eCAL_Types_UdpConfigVersion Convert_Types_UdpConfigVersion(eCAL::Types::UdpConfigVersion udp_config_version_)
  {
    static const std::map<eCAL::Types::UdpConfigVersion, enum eCAL_Types_UdpConfigVersion> udp_config_version_map
    {
      {eCAL::Types::UdpConfigVersion::V1, eCAL_Types_UdpConfigVersion_V1},
      {eCAL::Types::UdpConfigVersion::V2, eCAL_Types_UdpConfigVersion_V2},
    };
    return udp_config_version_map.at(udp_config_version_);
  }

  enum eCAL_eCommunicationMode Convert_eCommunicationMode(eCAL::eCommunicationMode communication_mode_)
  {
    static const std::map<eCAL::eCommunicationMode, enum eCAL_eCommunicationMode> communication_mode_map
    {
      {eCAL::eCommunicationMode::local, eCAL_eCommunicationMode_local},
      {eCAL::eCommunicationMode::network, eCAL_eCommunicationMode_network},
    };
    return communication_mode_map.at(communication_mode_);
  }

  eCAL_Logging_Filter Convert_Logging_Filter(eCAL::Logging::Filter filter_)
  {
    eCAL_Logging_Filter filter_c{ 0 };
    static const std::map<eCAL::Logging::eLogLevel, enum eCAL_Logging_eLogLevel> log_level_map
    {
        {eCAL::Logging::log_level_none, eCAL_Logging_log_level_none},
        {eCAL::Logging::log_level_all, eCAL_Logging_log_level_all, },
        {eCAL::Logging::log_level_info, eCAL_Logging_log_level_info},
        {eCAL::Logging::log_level_warning, eCAL_Logging_log_level_warning},
        {eCAL::Logging::log_level_error, eCAL_Logging_log_level_error},
        {eCAL::Logging::log_level_fatal, eCAL_Logging_log_level_fatal},
        {eCAL::Logging::log_level_debug1, eCAL_Logging_log_level_debug1},
        {eCAL::Logging::log_level_debug2, eCAL_Logging_log_level_debug2},
        {eCAL::Logging::log_level_debug3, eCAL_Logging_log_level_debug3},
        {eCAL::Logging::log_level_debug4, eCAL_Logging_log_level_debug4}
    };

    decltype(filter_) bit_mask = 1 << 0;
    for (std::size_t i = 0; i < sizeof(decltype(filter_)) * 8; ++i)
    {
      filter_c |= log_level_map.at(static_cast<eCAL::Logging::eLogLevel>(bit_mask & filter_));
      bit_mask <<= 1;
    }
    return filter_c;
  }

  void Assign_Application_Configuration(struct eCAL_Application_Configuration* configuration_c_, const eCAL::Application::Configuration& configuration_)
  {
    configuration_c_->sys.filter_excl = configuration_.sys.filter_excl.c_str();
    configuration_c_->startup.terminal_emulator = configuration_.startup.terminal_emulator.c_str();
  }

  void Assign_Logging_Configuration(struct eCAL_Logging_Configuration* configuration_c_, const eCAL::Logging::Configuration& configuration_)
  {
    // Assign Provider::Configuration
    configuration_c_->provider.console.enable = configuration_.provider.console.enable;
    configuration_c_->provider.console.log_level = Convert_Logging_Filter(configuration_.provider.console.log_level);

    configuration_c_->provider.file.enable = configuration_.provider.file.enable;
    configuration_c_->provider.file.log_level = Convert_Logging_Filter(configuration_.provider.file.log_level);

    configuration_c_->provider.udp.enable = configuration_.provider.udp.enable;
    configuration_c_->provider.udp.log_level = Convert_Logging_Filter(configuration_.provider.udp.log_level);

    configuration_c_->provider.file_config.path = configuration_.provider.file_config.path.c_str();
    configuration_c_->provider.udp_config.port = configuration_.provider.udp_config.port;

    // Assign Receiver::Configuration
    configuration_c_->receiver.enable = configuration_.receiver.enable;
    configuration_c_->receiver.udp_config.port = configuration_.receiver.udp_config.port;
  }

  void Assign_Publisher_Configuration(struct eCAL_Publisher_Configuration* configuration_c_, const eCAL::Publisher::Configuration& configuration_)
  {
    // Assign Layer::Configuration
    configuration_c_->layer.shm.enable = configuration_.layer.shm.enable;
    configuration_c_->layer.shm.zero_copy_mode = configuration_.layer.shm.zero_copy_mode;
    configuration_c_->layer.shm.acknowledge_timeout_ms = configuration_.layer.shm.acknowledge_timeout_ms;
    configuration_c_->layer.shm.memfile_buffer_count = configuration_.layer.shm.memfile_buffer_count;
    configuration_c_->layer.shm.memfile_min_size_bytes = configuration_.layer.shm.memfile_min_size_bytes;
    configuration_c_->layer.shm.memfile_reserve_percent = configuration_.layer.shm.memfile_reserve_percent;

    configuration_c_->layer.udp.enable = configuration_.layer.udp.enable;
    configuration_c_->layer.tcp.enable = configuration_.layer.tcp.enable;

    // Assign layer_priority_local
    configuration_c_->layer_priority_local_length = configuration_.layer_priority_local.size();
    for (size_t i = 0; i < configuration_.layer_priority_local.size(); ++i)
    {
      configuration_c_->layer_priority_local[i] = Convert_TransportLayer_eType(configuration_.layer_priority_local[i]);
    }

    // Assign layer_priority_remote
    configuration_c_->layer_priority_remote_length = configuration_.layer_priority_remote.size();
    for (size_t i = 0; i < configuration_.layer_priority_remote.size(); ++i)
    {
      configuration_c_->layer_priority_remote[i] = Convert_TransportLayer_eType(configuration_.layer_priority_remote[i]);
    }
  }

  void Assign_Registration_Configuration(struct eCAL_Registration_Configuration* configuration_c_, const eCAL::Registration::Configuration& configuration_)
  {
    // Assign general configuration
    configuration_c_->registration_timeout = configuration_.registration_timeout;
    configuration_c_->registration_refresh = configuration_.registration_refresh;
    configuration_c_->loopback = configuration_.loopback;
    configuration_c_->shm_transport_domain = configuration_.shm_transport_domain.c_str();

    // Assign Local::Configuration
    configuration_c_->local.transport_type = Convert_Registration_Local_eTransportType(configuration_.local.transport_type);

    configuration_c_->local.shm.domain = configuration_.local.shm.domain.c_str();
    configuration_c_->local.shm.queue_size = configuration_.local.shm.queue_size;

    configuration_c_->local.udp.port = configuration_.local.udp.port;

    // Assign Network::Configuration
    configuration_c_->network.transport_type = Convert_Registration_Network_eTransportType(configuration_.network.transport_type);

    configuration_c_->network.udp.port = configuration_.network.udp.port;
  }

  void Assign_Subscriber_Configuration(struct eCAL_Subscriber_Configuration* configuration_c_, const eCAL::Subscriber::Configuration& configuration_)
  {
    // Assign Layer::Configuration
    configuration_c_->layer.shm.enable = configuration_.layer.shm.enable;
    configuration_c_->layer.udp.enable = configuration_.layer.udp.enable;
    configuration_c_->layer.tcp.enable = configuration_.layer.tcp.enable;

    // Assign Subscriber configuration
    configuration_c_->drop_out_of_order_messages = configuration_.drop_out_of_order_messages;
  }

  void Assign_Time_Configuration(struct eCAL_Time_Configuration* configuration_c_, const eCAL::Time::Configuration& configuration_)
  {
    // Assign Time::Configuration
    configuration_c_->timesync_module_rt = configuration_.timesync_module_rt.c_str();
    configuration_c_->timesync_module_replay = configuration_.timesync_module_replay.c_str();
  }

  void Assign_TransportLayer_Configuration(struct eCAL_TransportLayer_Configuration* configuration_c_, const eCAL::TransportLayer::Configuration& configuration_)
  {
    // Assign UDP::Configuration
    configuration_c_->udp.config_version = Convert_Types_UdpConfigVersion(configuration_.udp.config_version);
    configuration_c_->udp.port = configuration_.udp.port;
    std::strncpy(configuration_c_->udp.mask, configuration_.udp.mask.Get().c_str(), sizeof(configuration_c_->udp.mask));
    configuration_c_->udp.send_buffer = configuration_.udp.send_buffer;
    configuration_c_->udp.receive_buffer = configuration_.udp.receive_buffer;
    configuration_c_->udp.join_all_interfaces = configuration_.udp.join_all_interfaces;
    configuration_c_->udp.npcap_enabled = configuration_.udp.npcap_enabled;

    strncpy(configuration_c_->udp.network.group, configuration_.udp.network.group.Get().c_str(), sizeof(configuration_c_->udp.network.group));
    configuration_c_->udp.network.ttl = configuration_.udp.network.ttl;

    strncpy(configuration_c_->udp.local.group, configuration_.udp.local.group.Get().c_str(), sizeof(configuration_c_->udp.local.group));
    configuration_c_->udp.local.ttl = configuration_.udp.local.ttl;

    // Assign TCP::Configuration
    configuration_c_->tcp.number_executor_reader = configuration_.tcp.number_executor_reader;
    configuration_c_->tcp.number_executor_writer = configuration_.tcp.number_executor_writer;
    configuration_c_->tcp.max_reconnections = configuration_.tcp.max_reconnections;
  }

  void Assign_Configuration(eCAL_Configuration* configuration_c_, const eCAL::Configuration& configuration_)
  {
    // Assign TransportLayer::Configuration
    Assign_TransportLayer_Configuration(&configuration_c_->transport_layer, configuration_.transport_layer);

    // Assign Registration::Configuration
    Assign_Registration_Configuration(&configuration_c_->registration, configuration_.registration);

    // Assign Subscriber::Configuration
    Assign_Subscriber_Configuration(&configuration_c_->subscriber, configuration_.subscriber);

    // Assign Publisher::Configuration
    Assign_Publisher_Configuration(&configuration_c_->publisher, configuration_.publisher);

    // Assign Time::Configuration
    Assign_Time_Configuration(&configuration_c_->timesync, configuration_.timesync);

    // Assign Application::Configuration
    Assign_Application_Configuration(&configuration_c_->application, configuration_.application);

    // Assign Logging::Configuration
    Assign_Logging_Configuration(&configuration_c_->logging, configuration_.logging);

    // Assign communication_mode
    configuration_c_->communication_mode = Convert_eCommunicationMode(configuration_.communication_mode);
  }

  /*
 *  Assigment and conversion functions
 *  from C to C++
 *
 */

  eCAL::TransportLayer::eType Convert_TransportLayer_eType(enum eCAL_TransportLayer_eType type_)
  {
    static const std::map<enum eCAL_TransportLayer_eType, eCAL::TransportLayer::eType> transport_layer_type_map
    {
      {eCAL_TransportLayer_eType_none, eCAL::TransportLayer::eType::none},
      {eCAL_TransportLayer_eType_shm, eCAL::TransportLayer::eType::shm},
      {eCAL_TransportLayer_eType_udp_mc, eCAL::TransportLayer::eType::udp_mc},
      {eCAL_TransportLayer_eType_tcp, eCAL::TransportLayer::eType::tcp}
    };
    return transport_layer_type_map.at(type_);
  }

  eCAL::Registration::Local::eTransportType Convert_Registration_Local_eTransportType(enum eCAL_Registration_Local_eTransportType transport_type_)
  {
    static const std::map<enum eCAL_Registration_Local_eTransportType, eCAL::Registration::Local::eTransportType> transport_type_map
    {
      {eCAL_Registration_Local_eTransportType_shm, eCAL::Registration::Local::eTransportType::shm},
      {eCAL_Registration_Local_eTransportType_udp, eCAL::Registration::Local::eTransportType::udp},
    };
    return transport_type_map.at(transport_type_);
  }

  eCAL::Registration::Network::eTransportType Convert_Registration_Network_eTransportType(enum eCAL_Registration_Network_eTransportType transport_type_)
  {
    static const std::map<enum eCAL_Registration_Network_eTransportType, eCAL::Registration::Network::eTransportType> transport_type_map
    {
      {eCAL_Registration_Network_eTransportType_udp, eCAL::Registration::Network::eTransportType::udp}
    };
    return transport_type_map.at(transport_type_);
  }

  eCAL::Types::UdpConfigVersion Convert_Types_UdpConfigVersion(enum eCAL_Types_UdpConfigVersion udp_config_version_)
  {
    static const std::map<enum eCAL_Types_UdpConfigVersion, eCAL::Types::UdpConfigVersion> udp_config_version_map
    {
      {eCAL_Types_UdpConfigVersion_V1, eCAL::Types::UdpConfigVersion::V1},
      {eCAL_Types_UdpConfigVersion_V2, eCAL::Types::UdpConfigVersion::V2},
    };
    return udp_config_version_map.at(udp_config_version_);
  }

  eCAL::eCommunicationMode Convert_eCommunicationMode(enum eCAL_eCommunicationMode communication_mode_)
  {
    static const std::map<enum eCAL_eCommunicationMode, eCAL::eCommunicationMode> communication_mode_map
    {
      {eCAL_eCommunicationMode_local, eCAL::eCommunicationMode::local},
      {eCAL_eCommunicationMode_network, eCAL::eCommunicationMode::network},
    };
    return communication_mode_map.at(communication_mode_);
  }

  eCAL::Logging::Filter Convert_Logging_FilterC(eCAL_Logging_Filter filter_c_)
  {
    eCAL::Logging::Filter filter{ 0 };
    static const std::map<enum eCAL_Logging_eLogLevel, eCAL::Logging::eLogLevel> log_level_map
    {
      {eCAL_Logging_log_level_none, eCAL::Logging::log_level_none},
      {eCAL_Logging_log_level_all, eCAL::Logging::log_level_all},
      {eCAL_Logging_log_level_info, eCAL::Logging::log_level_info},
      {eCAL_Logging_log_level_warning, eCAL::Logging::log_level_warning},
      {eCAL_Logging_log_level_error, eCAL::Logging::log_level_error},
      {eCAL_Logging_log_level_fatal, eCAL::Logging::log_level_fatal},
      {eCAL_Logging_log_level_debug1, eCAL::Logging::log_level_debug1},
      {eCAL_Logging_log_level_debug2, eCAL::Logging::log_level_debug2},
      {eCAL_Logging_log_level_debug3, eCAL::Logging::log_level_debug3},
      {eCAL_Logging_log_level_debug4, eCAL::Logging::log_level_debug4}
    };
    decltype(filter_c_) bit_mask = 1 << 0;
    for (std::size_t i = 0; i < sizeof(decltype(filter_c_)) * 8; ++i)
    {
      filter|= log_level_map.at(static_cast<enum eCAL_Logging_eLogLevel>(bit_mask & filter_c_));
      bit_mask <<= 1;
    }
    return filter;
  }

  void Assign_Application_Configuration(eCAL::Application::Configuration& configuration_, const struct eCAL_Application_Configuration* configuration_c_)
  {
    configuration_.sys.filter_excl = configuration_c_->sys.filter_excl != NULL ? configuration_c_->sys.filter_excl : "";
    configuration_.startup.terminal_emulator = configuration_c_->startup.terminal_emulator != NULL ? configuration_c_->startup.terminal_emulator : "";
  }

  void Assign_Logging_Configuration(eCAL::Logging::Configuration& configuration_, const struct eCAL_Logging_Configuration* configuration_c_)
  {
    // Assign Provider::Configuration
    configuration_.provider.console.enable = static_cast<bool>(configuration_c_->provider.console.enable);
    configuration_.provider.console.log_level = Convert_Logging_FilterC(configuration_c_->provider.console.log_level);

    configuration_.provider.file.enable = static_cast<bool>(configuration_c_->provider.file.enable);
    configuration_.provider.file.log_level = Convert_Logging_FilterC(configuration_c_->provider.file.log_level);

    configuration_.provider.udp.enable = static_cast<bool>(configuration_c_->provider.udp.enable);
    configuration_.provider.udp.log_level = Convert_Logging_FilterC(configuration_c_->provider.udp.log_level);

    configuration_.provider.file_config.path = configuration_c_->provider.file_config.path != NULL ? configuration_c_->provider.file_config.path : "";
    configuration_.provider.udp_config.port = configuration_c_->provider.udp_config.port;

    // Assign Receiver::Configuration
    configuration_.receiver.enable = static_cast<bool>(configuration_c_->receiver.enable);
    configuration_.receiver.udp_config.port = configuration_c_->receiver.udp_config.port;
  }

  void Assign_Publisher_Configuration(eCAL::Publisher::Configuration& configuration_, const struct eCAL_Publisher_Configuration* configuration_c_)
  {
    // Assign Layer::Configuration
    configuration_.layer.shm.enable = static_cast<bool>(configuration_c_->layer.shm.enable);
    configuration_.layer.shm.zero_copy_mode = static_cast<bool>(configuration_c_->layer.shm.zero_copy_mode);
    configuration_.layer.shm.acknowledge_timeout_ms = configuration_c_->layer.shm.acknowledge_timeout_ms;
    configuration_.layer.shm.memfile_buffer_count = configuration_c_->layer.shm.memfile_buffer_count;
    configuration_.layer.shm.memfile_min_size_bytes = configuration_c_->layer.shm.memfile_min_size_bytes;
    configuration_.layer.shm.memfile_reserve_percent = configuration_c_->layer.shm.memfile_reserve_percent;

    configuration_.layer.udp.enable = static_cast<bool>(configuration_c_->layer.udp.enable);
    configuration_.layer.tcp.enable = static_cast<bool>(configuration_c_->layer.tcp.enable);

    // Assign layer_priority_local
    configuration_.layer_priority_local.resize(configuration_c_->layer_priority_local_length);
    for (size_t i = 0; i < configuration_c_->layer_priority_local_length; ++i)
    {
      configuration_.layer_priority_local[i] = Convert_TransportLayer_eType(configuration_c_->layer_priority_local[i]);
    }

    // Assign layer_priority_remote
    configuration_.layer_priority_remote.resize(configuration_c_->layer_priority_remote_length);
    for (size_t i = 0; i < configuration_c_->layer_priority_remote_length; ++i)
    {
      configuration_.layer_priority_remote[i] = Convert_TransportLayer_eType(configuration_c_->layer_priority_remote[i]);
    }
  }

  void Assign_Registration_Configuration(eCAL::Registration::Configuration& configuration_, const struct eCAL_Registration_Configuration* configuration_c_)
  {
    // Assign general configuration
    configuration_.registration_timeout = configuration_c_->registration_timeout;
    configuration_.registration_refresh = configuration_c_->registration_refresh;
    configuration_.loopback = static_cast<bool>(configuration_c_->loopback);
    configuration_.shm_transport_domain = configuration_c_->shm_transport_domain != NULL ? configuration_c_->shm_transport_domain : "";

    // Assign Local::Configuration
    configuration_.local.transport_type = Convert_Registration_Local_eTransportType(configuration_c_->local.transport_type);
    configuration_.local.shm.domain = configuration_c_->local.shm.domain != NULL ? configuration_c_->local.shm.domain : "";
    configuration_.local.shm.queue_size = configuration_c_->local.shm.queue_size;
    configuration_.local.udp.port = configuration_c_->local.udp.port;

    // Assign Network::Configuration
    configuration_.network.transport_type = Convert_Registration_Network_eTransportType(configuration_c_->network.transport_type);
    configuration_.network.udp.port = configuration_c_->network.udp.port;
  }

  void Assign_Subscriber_Configuration(eCAL::Subscriber::Configuration& configuration_, const struct eCAL_Subscriber_Configuration* configuration_c_)
  {
    // Assign Layer::Configuration
    configuration_.layer.shm.enable = static_cast<bool>(configuration_c_->layer.shm.enable);
    configuration_.layer.udp.enable = static_cast<bool>(configuration_c_->layer.udp.enable);
    configuration_.layer.tcp.enable = static_cast<bool>(configuration_c_->layer.tcp.enable);

    // Assign Subscriber configuration
    configuration_.drop_out_of_order_messages = static_cast<bool>(configuration_c_->drop_out_of_order_messages);
  }

  void Assign_Time_Configuration(eCAL::Time::Configuration& configuration_, const struct eCAL_Time_Configuration* configuration_c_)
  {
    // Assign Time::Configuration
    configuration_.timesync_module_rt = configuration_c_->timesync_module_rt != NULL ? configuration_c_->timesync_module_rt : "";
    configuration_.timesync_module_replay = configuration_c_->timesync_module_replay != NULL ? configuration_c_->timesync_module_replay : "";
  }

  void Assign_TransportLayer_Configuration(eCAL::TransportLayer::Configuration& configuration_, const struct eCAL_TransportLayer_Configuration* configuration_c_)
  {
    // Assign UDP::Configuration
    configuration_.udp.config_version = Convert_Types_UdpConfigVersion(configuration_c_->udp.config_version);
    configuration_.udp.port = configuration_c_->udp.port;
    configuration_.udp.mask = configuration_c_->udp.mask;
    configuration_.udp.send_buffer = configuration_c_->udp.send_buffer;
    configuration_.udp.receive_buffer = configuration_c_->udp.receive_buffer;
    configuration_.udp.join_all_interfaces = static_cast<bool>(configuration_c_->udp.join_all_interfaces);
    configuration_.udp.npcap_enabled = static_cast<bool>(configuration_c_->udp.npcap_enabled);

    configuration_.udp.network.group = configuration_c_->udp.network.group;
    configuration_.udp.network.ttl = configuration_c_->udp.network.ttl;

    configuration_.udp.local.group = configuration_c_->udp.local.group;
    configuration_.udp.local.ttl = configuration_c_->udp.local.ttl;

    // Assign TCP::Configuration
    configuration_.tcp.number_executor_reader = configuration_c_->tcp.number_executor_reader;
    configuration_.tcp.number_executor_writer = configuration_c_->tcp.number_executor_writer;
    configuration_.tcp.max_reconnections = configuration_c_->tcp.max_reconnections;
  }

  void Assign_Configuration(eCAL::Configuration& configuration_, const eCAL_Configuration* configuration_c_)
  {
    // Assign TransportLayer::Configuration
    Assign_TransportLayer_Configuration(configuration_.transport_layer, &configuration_c_->transport_layer);

    // Assign Registration::Configuration
    Assign_Registration_Configuration(configuration_.registration, &configuration_c_->registration);

    // Assign Subscriber::Configuration
    Assign_Subscriber_Configuration(configuration_.subscriber, &configuration_c_->subscriber);

    // Assign Publisher::Configuration
    Assign_Publisher_Configuration(configuration_.publisher, &configuration_c_->publisher);

    // Assign Time::Configuration
    Assign_Time_Configuration(configuration_.timesync, &configuration_c_->timesync);

    // Assign Application::Configuration
    Assign_Application_Configuration(configuration_.application, &configuration_c_->application);

    // Assign Logging::Configuration
    Assign_Logging_Configuration(configuration_.logging, &configuration_c_->logging);

    // Assign communication_mode
    configuration_.communication_mode = Convert_eCommunicationMode(configuration_c_->communication_mode);
  }
}