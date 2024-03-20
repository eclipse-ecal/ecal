
.. _program_listing_file_ecal_core_include_ecal_ecal_config.h:

Program Listing for File ecal_config.h
======================================

|exhale_lsh| :ref:`Return to documentation for file <file_ecal_core_include_ecal_ecal_config.h>` (``ecal/core/include/ecal/ecal_config.h``)

.. |exhale_lsh| unicode:: U+021B0 .. UPWARDS ARROW WITH TIP LEFTWARDS

.. code-block:: cpp

   /* ========================= eCAL LICENSE =================================
    *
    * Copyright (C) 2016 - 2019 Continental Corporation
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
   
   #include <string>
   
   namespace eCAL
   {
     namespace Config
     {
       enum class UdpConfigVersion
       {
         V1 = 1, // Legacy
         V2 = 2
       };
   
       // common
   
       ECAL_API std::string       GetLoadedEcalIniPath                 ();
       ECAL_API int               GetRegistrationTimeoutMs             ();
       ECAL_API int               GetRegistrationRefreshMs             ();
   
       // network
   
       ECAL_API bool              IsNetworkEnabled                     ();
       ECAL_API UdpConfigVersion  GetUdpMulticastConfigVersion         ();
       ECAL_API std::string       GetUdpMulticastGroup                 ();
       ECAL_API std::string       GetUdpMulticastMask                  ();
       ECAL_API int               GetUdpMulticastPort                  ();
       ECAL_API int               GetUdpMulticastTtl                   ();
   
       ECAL_API int               GetUdpMulticastSndBufSizeBytes       ();
       ECAL_API int               GetUdpMulticastRcvBufSizeBytes       ();
   
       ECAL_API bool              IsUdpMulticastJoinAllIfEnabled       ();
   
       ECAL_API int               GetMaxUdpBandwidthBytesPerSecond     ();
   
       ECAL_API bool              IsUdpMulticastRecEnabled             ();
       ECAL_API bool              IsShmRecEnabled                      ();
       ECAL_API bool              IsTcpRecEnabled                      ();
       ECAL_API bool              IsInprocRecEnabled                   ();
   
       ECAL_API bool              IsNpcapEnabled                       ();
   
       ECAL_API int               GetTcpPubsubReaderThreadpoolSize     ();
       ECAL_API int               GetTcpPubsubWriterThreadpoolSize     ();
       ECAL_API int               GetTcpPubsubMaxReconnectionAttemps   ();
   
       ECAL_API std::string       GetHostGroupName                     ();
   
       // time
   
       ECAL_API std::string       GetTimesyncModuleName                ();
   
       // process
   
       ECAL_API std::string       GetTerminalEmulatorCommand           ();
   
       // monitoring
   
       ECAL_API int                 GetMonitoringTimeoutMs               ();
       ECAL_API std::string         GetMonitoringFilterExcludeList       ();
       ECAL_API std::string         GetMonitoringFilterIncludeList       ();
       ECAL_API eCAL_Logging_Filter GetConsoleLogFilter                  ();
       ECAL_API eCAL_Logging_Filter GetFileLogFilter                     ();
       ECAL_API eCAL_Logging_Filter GetUdpLogFilter                      ();
   
       // sys
   
       ECAL_API std::string         GetEcalSysFilterExcludeList          ();
   
       // publisher
       ECAL_API TLayer::eSendMode GetPublisherInprocMode               ();
       ECAL_API TLayer::eSendMode GetPublisherShmMode                  ();
       ECAL_API TLayer::eSendMode GetPublisherTcpMode                  ();
       ECAL_API TLayer::eSendMode GetPublisherUdpMulticastMode         ();
   
       ECAL_API size_t            GetMemfileMinsizeBytes               ();
       ECAL_API size_t            GetMemfileOverprovisioningPercentage ();
       ECAL_API int               GetMemfileAckTimeoutMs               ();
       ECAL_API bool              IsMemfileZerocopyEnabled             ();
       ECAL_API size_t            GetMemfileBufferCount                ();
   
       ECAL_API bool              IsTopicTypeSharingEnabled            ();
       ECAL_API bool              IsTopicDescriptionSharingEnabled     ();
   
       // service
       ECAL_API bool              IsServiceProtocolV0Enabled           ();
       ECAL_API bool              IsServiceProtocolV1Enabled           ();
   
       // experimental
       namespace Experimental
       {
         ECAL_API bool              IsShmMonitoringEnabled             ();
         ECAL_API bool              IsNetworkMonitoringDisabled        ();
         ECAL_API size_t            GetShmMonitoringQueueSize          ();
         ECAL_API std::string       GetShmMonitoringDomain             ();
         ECAL_API bool              GetDropOutOfOrderMessages          ();
       }
     }
   }
   //@}
