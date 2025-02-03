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

#pragma once

#include <cstddef>
#include <ecal/log_level.h>
#include <ecal/os.h>
#include "ecal/config/configuration.h"

#include <string>

//@{ 
namespace eCAL
{
  ECAL_API const Configuration&              GetConfiguration                 ();
  ECAL_API const Subscriber::Configuration&  GetSubscriberConfiguration       ();
  ECAL_API const Publisher::Configuration&   GetPublisherConfiguration        ();

  namespace Config
  {
    /////////////////////////////////////
    // common
    /////////////////////////////////////

    ECAL_API std::string              GetLoadedEcalIniPath                 ();
    ECAL_API int                      GetRegistrationTimeoutMs             ();
    ECAL_API int                      GetRegistrationRefreshMs             ();

    /////////////////////////////////////
    // network
    /////////////////////////////////////

    ECAL_API bool                     IsNetworkEnabled                     ();
    ECAL_API bool                     IsShmRegistrationEnabled             ();
    ECAL_API Types::UdpConfigVersion  GetUdpMulticastConfigVersion         ();
    ECAL_API std::string              GetUdpMulticastGroup                 ();
    ECAL_API std::string              GetUdpMulticastMask                  ();
    ECAL_API int                      GetUdpMulticastPort                  ();
    ECAL_API int                      GetUdpMulticastTtl                   ();

    ECAL_API int                      GetUdpMulticastSndBufSizeBytes       ();
    ECAL_API int                      GetUdpMulticastRcvBufSizeBytes       ();

    ECAL_API bool                     IsUdpMulticastJoinAllIfEnabled       ();

    ECAL_API bool                     IsUdpMulticastRecEnabled             ();
    ECAL_API bool                     IsShmRecEnabled                      ();
    ECAL_API bool                     IsTcpRecEnabled                      ();

    ECAL_API bool                     IsNpcapEnabled                       ();

    ECAL_API size_t                   GetTcpPubsubReaderThreadpoolSize     ();
    ECAL_API size_t                   GetTcpPubsubWriterThreadpoolSize     ();
    ECAL_API int                      GetTcpPubsubMaxReconnectionAttemps   ();

    ECAL_API std::string              GetShmTransportDomain                ();

    /////////////////////////////////////
    // time
    /////////////////////////////////////

    ECAL_API std::string              GetTimesyncModuleName                ();
    ECAL_API std::string              GetTimesyncModuleReplay              ();
    
    /////////////////////////////////////
    // process
    /////////////////////////////////////

    ECAL_API std::string              GetTerminalEmulatorCommand           ();

    /////////////////////////////////////
    // sys
    /////////////////////////////////////

    ECAL_API std::string              GetEcalSysFilterExcludeList          ();
    
    /////////////////////////////////////
    // subscriber
    /////////////////////////////////////
    
    ECAL_API bool                     GetDropOutOfOrderMessages            ();

    /////////////////////////////////////
    // registration
    /////////////////////////////////////
    
    ECAL_API size_t                   GetShmMonitoringQueueSize            ();
    ECAL_API std::string              GetShmMonitoringDomain               ();
  }
}
//@}
