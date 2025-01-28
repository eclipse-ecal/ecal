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

#include <ecal/config.h>
#include "ecal_config_internal.h"
#include <string>
#include <vector>


namespace eCAL
{
  namespace Config
  {
    /////////////////////////////////////
    // common
    /////////////////////////////////////
    
    std::string       GetLoadedEcalIniPath                 () { return GetConfiguration().GetConfigurationFilePath(); }
    int               GetRegistrationTimeoutMs             () { return GetConfiguration().registration.registration_timeout; }
    int               GetRegistrationRefreshMs             () { return GetConfiguration().registration.registration_refresh; }

    /////////////////////////////////////
    // network
    /////////////////////////////////////

    bool              IsNetworkEnabled                     () { return GetConfiguration().registration.network_enabled; }
    bool              IsShmRegistrationEnabled             () { return GetConfiguration().registration.layer.shm.enable; }

    Types::UdpConfigVersion  GetUdpMulticastConfigVersion  () { return GetConfiguration().transport_layer.udp.config_version; }

    std::string       GetUdpMulticastGroup                 () { return GetConfiguration().transport_layer.udp.network.group; }
    std::string       GetUdpMulticastMask                  () { return GetConfiguration().transport_layer.udp.mask; }
    int               GetUdpMulticastPort                  () { return GetConfiguration().transport_layer.udp.port; }
    int               GetUdpMulticastTtl                   () { return GetConfiguration().transport_layer.udp.network.ttl; }

    int               GetUdpMulticastSndBufSizeBytes       () { return GetConfiguration().transport_layer.udp.send_buffer; }
    int               GetUdpMulticastRcvBufSizeBytes       () { return GetConfiguration().transport_layer.udp.receive_buffer; }
    bool              IsUdpMulticastJoinAllIfEnabled       () { return GetConfiguration().transport_layer.udp.join_all_interfaces; }

    bool              IsUdpMulticastRecEnabled             () { return GetConfiguration().subscriber.layer.udp.enable; }
    bool              IsShmRecEnabled                      () { return GetConfiguration().subscriber.layer.shm.enable; }
    bool              IsTcpRecEnabled                      () { return GetConfiguration().subscriber.layer.tcp.enable; }

    bool              IsNpcapEnabled                       () { return GetConfiguration().transport_layer.udp.npcap_enabled; }

    size_t            GetTcpPubsubReaderThreadpoolSize     () { return GetConfiguration().transport_layer.tcp.number_executor_reader;}
    size_t            GetTcpPubsubWriterThreadpoolSize     () { return GetConfiguration().transport_layer.tcp.number_executor_writer;}
    int               GetTcpPubsubMaxReconnectionAttemps   () { return GetConfiguration().transport_layer.tcp.max_reconnections;}

    std::string       GetShmTransportDomain                () { return GetConfiguration().registration.shm_transport_domain; }
    
    /////////////////////////////////////
    // time
    /////////////////////////////////////
    
    std::string       GetTimesyncModuleName                () { return GetConfiguration().timesync.timesync_module_rt; }
    std::string       GetTimesyncModuleReplay              () { return GetConfiguration().timesync.timesync_module_replay; }

    /////////////////////////////////////
    // process
    /////////////////////////////////////
    
    std::string       GetTerminalEmulatorCommand           () { return GetConfiguration().application.startup.terminal_emulator; }

    /////////////////////////////////////
    // sys
    /////////////////////////////////////
    
    std::string       GetEcalSysFilterExcludeList          () { return GetConfiguration().application.sys.filter_excl; }

    /////////////////////////////////////
    // subscriber
    /////////////////////////////////////
    
    bool              GetDropOutOfOrderMessages          () { return GetConfiguration().subscriber.drop_out_of_order_messages; }

    /////////////////////////////////////
    // registration
    /////////////////////////////////////
    
    size_t            GetShmMonitoringQueueSize          () { return GetConfiguration().registration.layer.shm.queue_size; }
    std::string       GetShmMonitoringDomain             () { return GetConfiguration().registration.layer.shm.domain;}

  }

  const TransportLayer::Configuration& GetTransportLayerConfiguration()
  {
    return GetConfiguration().transport_layer;
  }

  const Registration::Configuration& GetRegistrationConfiguration()
  {
    return GetConfiguration().registration;
  }

  const Logging::Configuration& GetLoggingConfiguration()
  {
    return GetConfiguration().logging;
  }

  const Subscriber::Configuration& GetSubscriberConfiguration()
  {
    return GetConfiguration().subscriber;
  }

  const Publisher::Configuration& GetPublisherConfiguration()
  {
    return GetConfiguration().publisher;
  }

  const Time::Configuration& GetTimesyncConfiguration()
  {
    return GetConfiguration().timesync;
  }

  const Application::Configuration& GetApplicationConfiguration()
  {
    return GetConfiguration().application;
  }
}
