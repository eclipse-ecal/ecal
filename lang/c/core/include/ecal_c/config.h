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

 /**
  * @file   config.h
  * @brief  eCAL configuration access
 **/

#ifndef ecal_c_config_h_included
#define ecal_c_config_h_included

#include <ecal_c/export.h>
#include <ecal_c/types/custom_data_types.h>
#include <ecal_c/config/configuration.h>
#include <ecal_c/config/publisher.h>
#include <ecal_c/config/subscriber.h>

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

  /////////////////////////////////////
  // configuration
  /////////////////////////////////////
  ECALC_API const eCAL_Configuration* eCAL_GetConfiguration();
  ECALC_API const struct eCAL_Subscriber_Configuration* eCAL_GetSubscriberConfiguration();
  ECALC_API const struct eCAL_Publisher_Configuration* eCAL_GetPublisherConfiguration();

  /////////////////////////////////////
  // common
  /////////////////////////////////////
  ECALC_API const char* eCAL_Config_GetLoadedEcalYamlPath();
  ECALC_API int eCAL_Config_GetRegistrationTimeoutMs();
  ECALC_API int eCAL_Config_GetRegistrationRefreshMs();

  /////////////////////////////////////
  // network
  /////////////////////////////////////
  ECALC_API int eCAL_Config_IsNetworkEnabled();
  ECALC_API int eCAL_Config_IsShmRegistrationEnabled();
  ECALC_API enum eCAL_Types_UdpConfigVersion eCAL_Config_GetUdpMulticastConfigVersion();
  ECALC_API const char* eCAL_Config_GetUdpMulticastGroup();
  ECALC_API const char* eCAL_Config_GetUdpMulticastMask();
  ECALC_API int eCAL_Config_GetUdpMulticastPort();
  ECALC_API int eCAL_Config_GetUdpMulticastTtl();
  ECALC_API int eCAL_Config_GetUdpMulticastSndBufSizeBytes();
  ECALC_API int eCAL_Config_GetUdpMulticastRcvBufSizeBytes();
  ECALC_API int eCAL_Config_IsUdpMulticastJoinAllIfEnabled();
  ECALC_API int eCAL_Config_IsUdpMulticastRecEnabled();
  ECALC_API int eCAL_Config_IsShmRecEnabled();
  ECALC_API int eCAL_Config_IsTcpRecEnabled();
  ECALC_API int eCAL_Config_IsNpcapEnabled();
  ECALC_API size_t eCAL_Config_GetTcpPubsubReaderThreadpoolSize();
  ECALC_API size_t eCAL_Config_GetTcpPubsubWriterThreadpoolSize();
  ECALC_API int eCAL_Config_GetTcpPubsubMaxReconnectionAttemps();
  ECALC_API const char* eCAL_Config_GetShmTransportDomain();

  /////////////////////////////////////
  // time
  /////////////////////////////////////
  ECALC_API const char* eCAL_Config_GetTimesyncModuleName();
  ECALC_API const char* eCAL_Config_GetTimesyncModuleReplay();

  /////////////////////////////////////
  // process
  /////////////////////////////////////
  ECALC_API const char* eCAL_Config_GetTerminalEmulatorCommand();

  /////////////////////////////////////
  // sys
  /////////////////////////////////////
  ECALC_API const char* eCAL_Config_GetEcalSysFilterExcludeList();

  /////////////////////////////////////
  // subscriber
  /////////////////////////////////////
  ECALC_API int eCAL_Config_GetDropOutOfOrderMessages();

  /////////////////////////////////////
  // registration
  /////////////////////////////////////
  ECALC_API size_t eCAL_Config_GetShmMonitoringQueueSize();
  ECALC_API const char* eCAL_Config_GetShmMonitoringDomain();

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*ecal_c_config_h_included*/