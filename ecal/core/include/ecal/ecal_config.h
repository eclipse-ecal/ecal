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

#include <cstddef>
#include <ecal/ecal_log_level.h>
#include <ecal/ecal_os.h>
#include <ecal/ecal_tlayer.h>
#include "ecal/config/configuration.h"

#include <string>

//@{ 
namespace eCAL
{
  ECAL_API Configuration& GetConfiguration();
  
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
    ECAL_API size_t                   GetTcpPubsubMaxReconnectionAttemps   ();

    ECAL_API int                      GetTcpPubReaderThreadpoolSize     ();
    ECAL_API int                      GetTcpPubWriterThreadpoolSize     ();

    ECAL_API int                      GetTcpSubReaderThreadpoolSize     ();
    ECAL_API int                      GetTcpSubWriterThreadpoolSize     ();
    ECAL_API int                      GetTcpSubMaxReconnectionAttemps   ();

    ECAL_API std::string              GetHostGroupName                     ();

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
    // monitoring
    /////////////////////////////////////

    ECAL_API int                      GetMonitoringTimeoutMs               ();
    ECAL_API std::string              GetMonitoringFilterExcludeList       ();
    ECAL_API std::string              GetMonitoringFilterIncludeList       ();
    ECAL_API eCAL_Logging_Filter      GetConsoleLogFilter                  ();
    ECAL_API eCAL_Logging_Filter      GetFileLogFilter                     ();
    ECAL_API eCAL_Logging_Filter      GetUdpLogFilter                      ();

    /////////////////////////////////////
    // sys
    /////////////////////////////////////

    ECAL_API std::string              GetEcalSysFilterExcludeList          ();

    /////////////////////////////////////
    // publisher
    /////////////////////////////////////
    ECAL_API bool                     IsTopicTypeSharingEnabled            ();
    ECAL_API bool                     IsTopicDescriptionSharingEnabled     ();

    /////////////////////////////////////
    // service
    /////////////////////////////////////
    ECAL_API bool                     IsServiceProtocolV0Enabled           ();
    ECAL_API bool                     IsServiceProtocolV1Enabled           ();

    /////////////////////////////////////
    // experimental
    /////////////////////////////////////
    namespace Experimental
    {
      ECAL_API size_t                 GetShmMonitoringQueueSize          ();
      ECAL_API std::string            GetShmMonitoringDomain             ();
      ECAL_API bool                   GetDropOutOfOrderMessages          ();
    }
  }
}
//@}