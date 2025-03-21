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

#include <ecal_c/config.h>
#include <ecal/ecal.h>

#include "common.h"
#include "configuration.h"

#include <algorithm>

extern "C"
{
  ECALC_API const eCAL_Configuration* eCAL_GetConfiguration()
  {
    static eCAL_Configuration configuration;
    Assign_Configuration(&configuration, eCAL::GetConfiguration());
    return &configuration;
  }

  ECALC_API const struct eCAL_Subscriber_Configuration* eCAL_GetSubscriberConfiguration()
  {
    static struct eCAL_Subscriber_Configuration subscriber_configuration;
    Assign_Subscriber_Configuration(&subscriber_configuration, eCAL::GetSubscriberConfiguration());
    return &subscriber_configuration;
  }

  ECALC_API const struct eCAL_Publisher_Configuration* eCAL_GetPublisherConfiguration()
  {
    static struct eCAL_Publisher_Configuration publisher_configuration;
    Assign_Publisher_Configuration(&publisher_configuration, eCAL::GetPublisherConfiguration());
    return &publisher_configuration;
  }

  ECALC_API const char* eCAL_Config_GetLoadedEcalYamlPath()
  {
    return eCAL::Config::GetLoadedEcalIniPath().c_str();
  }

  ECALC_API int eCAL_Config_GetRegistrationTimeoutMs()
  {
    return eCAL::Config::GetRegistrationTimeoutMs();
  }

  ECALC_API int eCAL_Config_GetRegistrationRefreshMs()
  {
    return eCAL::Config::GetRegistrationRefreshMs();
  }

  ECALC_API int eCAL_Config_IsNetworkEnabled()
  {
    return static_cast<int>(eCAL::Config::IsNetworkEnabled());
  }

  ECALC_API int eCAL_Config_IsShmRegistrationEnabled()
  {
    return static_cast<int>(eCAL::Config::IsShmRegistrationEnabled());
  }

  ECALC_API enum eCAL_Types_UdpConfigVersion eCAL_Config_GetUdpMulticastConfigVersion()
  {
    static const std::map<eCAL::Types::UdpConfigVersion, eCAL_Types_UdpConfigVersion> udp_config_version_map
    {
      {eCAL::Types::UdpConfigVersion::V1, eCAL_Types_UdpConfigVersion_V1},
      {eCAL::Types::UdpConfigVersion::V2, eCAL_Types_UdpConfigVersion_V2}
    };
    return udp_config_version_map.at(eCAL::Config::GetUdpMulticastConfigVersion());
  }

  // Network
  ECALC_API const char* eCAL_Config_GetUdpMulticastGroup()
  {
    return eCAL::Config::GetUdpMulticastGroup().c_str();
  }

  ECALC_API const char* eCAL_Config_GetUdpMulticastMask()
  {
    return eCAL::Config::GetUdpMulticastMask().c_str();
  }

  ECALC_API int eCAL_Config_GetUdpMulticastPort()
  {
    return eCAL::Config::GetUdpMulticastPort();
  }

  ECALC_API int eCAL_Config_GetUdpMulticastTtl()
  {
    return eCAL::Config::GetUdpMulticastTtl();
  }

  ECALC_API int eCAL_Config_GetUdpMulticastSndBufSizeBytes()
  {
    return eCAL::Config::GetUdpMulticastSndBufSizeBytes();
  }

  ECALC_API int eCAL_Config_GetUdpMulticastRcvBufSizeBytes()
  {
    return eCAL::Config::GetUdpMulticastRcvBufSizeBytes();
  }

  ECALC_API int eCAL_Config_IsUdpMulticastJoinAllIfEnabled()
  {
    return static_cast<int>(eCAL::Config::IsUdpMulticastJoinAllIfEnabled());
  }

  ECALC_API int eCAL_Config_IsUdpMulticastRecEnabled()
  {
    return static_cast<int>(eCAL::Config::IsUdpMulticastRecEnabled());
  }

  ECALC_API int eCAL_Config_IsShmRecEnabled()
  {
    return static_cast<int>(eCAL::Config::IsShmRecEnabled());
  }

  ECALC_API int eCAL_Config_IsTcpRecEnabled()
  {
    return static_cast<int>(eCAL::Config::IsTcpRecEnabled());
  }

  ECALC_API int eCAL_Config_IsNpcapEnabled()
  {
    return static_cast<int>(eCAL::Config::IsNpcapEnabled());
  }

  ECALC_API size_t eCAL_Config_GetTcpPubsubReaderThreadpoolSize()
  {
    return eCAL::Config::GetTcpPubsubReaderThreadpoolSize();
  }

  ECALC_API size_t eCAL_Config_GetTcpPubsubWriterThreadpoolSize()
  {
    return eCAL::Config::GetTcpPubsubWriterThreadpoolSize();
  }

  ECALC_API int eCAL_Config_GetTcpPubsubMaxReconnectionAttemps()
  {
    return eCAL::Config::GetTcpPubsubMaxReconnectionAttemps();
  }

  ECALC_API const char* eCAL_Config_GetShmTransportDomain()
  {
    return eCAL::Config::GetShmTransportDomain().c_str();
  }

  // Time
  ECALC_API const char* eCAL_Config_GetTimesyncModuleName()
  {
    return eCAL::Config::GetTimesyncModuleName().c_str();
  }

  ECALC_API const char* eCAL_Config_GetTimesyncModuleReplay()
  {
    return eCAL::Config::GetTimesyncModuleReplay().c_str();
  }

  // Process
  ECALC_API const char* eCAL_Config_GetTerminalEmulatorCommand()
  {
    return eCAL::Config::GetTerminalEmulatorCommand().c_str();
  }

  // Sys
  ECALC_API const char* eCAL_Config_GetEcalSysFilterExcludeList()
  {
    return eCAL::Config::GetEcalSysFilterExcludeList().c_str();
  }

  // Subscriber
  ECALC_API int eCAL_Config_GetDropOutOfOrderMessages()
  {
    return static_cast<int>(eCAL::Config::GetDropOutOfOrderMessages());
  }

  // Registration
  ECALC_API size_t eCAL_Config_GetShmMonitoringQueueSize()
  {
    return eCAL::Config::GetShmMonitoringQueueSize();
  }

  ECALC_API const char* eCAL_Config_GetShmMonitoringDomain()
  {
    return eCAL::Config::GetShmMonitoringDomain().c_str();
  }
}