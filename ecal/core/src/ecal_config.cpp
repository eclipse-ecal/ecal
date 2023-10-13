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

#include <ecal/ecal_config.h>
#include <ecal/ecal_log_level.h>

#include "ecal_config_reader.h"
#include "ecal_config_reader_hlp.h"
#include "ecal_def.h"


namespace
{
  void tokenize(const std::string& str, std::vector<std::string>& tokens,
    const std::string& delimiters = " ", bool trimEmpty = false)
  {
    std::string::size_type pos, lastPos = 0;

    for (;;)
    {
      pos = str.find_first_of(delimiters, lastPos);
      if (pos == std::string::npos)
      {
        pos = str.length();
        if (pos != lastPos || !trimEmpty)
        {
          tokens.emplace_back(std::string(str.data() + lastPos, pos - lastPos));
        }
        break;
      }
      else
      {
        if (pos != lastPos || !trimEmpty)
        {
          tokens.emplace_back(std::string(str.data() + lastPos, pos - lastPos));
        }
      }
      lastPos = pos + 1;
    }
  }


  eCAL_Logging_Filter ParseLogLevel(const std::string& filter_)
  {
    // tokenize it
    std::vector<std::string> token_filter_;
    tokenize(filter_, token_filter_, " ,;");
    // create excluding filter list
    char filter_mask = log_level_none;
    for (auto& it : token_filter_)
    {
      if (it == "all")     filter_mask |= log_level_all;
      if (it == "info")    filter_mask |= log_level_info;
      if (it == "warning") filter_mask |= log_level_warning;
      if (it == "error")   filter_mask |= log_level_error;
      if (it == "fatal")   filter_mask |= log_level_fatal;
      if (it == "debug1")  filter_mask |= log_level_debug1;
      if (it == "debug2")  filter_mask |= log_level_debug2;
      if (it == "debug3")  filter_mask |= log_level_debug3;
      if (it == "debug4")  filter_mask |= log_level_debug4;
    }
    return(filter_mask);
  }

}


namespace eCAL
{
  namespace Config
  {
    /////////////////////////////////////
    // common
    /////////////////////////////////////
    
    ECAL_API std::string       GetLoadedEcalIniPath                 () { return g_default_ini_file; }
    ECAL_API int               GetRegistrationTimeoutMs             () { return eCALPAR(CMN, REGISTRATION_TO); }
    ECAL_API int               GetRegistrationRefreshMs             () { return eCALPAR(CMN, REGISTRATION_REFRESH); }

    /////////////////////////////////////
    // network
    /////////////////////////////////////

    ECAL_API bool              IsNetworkEnabled                     () { return eCALPAR(NET, ENABLED); }

    ECAL_API UdpConfigVersion  GetUdpMulticastConfigVersion()
    {
      const std::string udp_config_version_string = eCALPAR(NET, UDP_MULTICAST_CONFIG_VERSION);
      if (udp_config_version_string == "v1")
        return UdpConfigVersion::V1;
      if (udp_config_version_string == "v2")
        return UdpConfigVersion::V2;
      // TODO: Log error. However not sure if logging is initialized at this place.
      return UdpConfigVersion::V1;
    }

    ECAL_API std::string       GetUdpMulticastGroup                 () { return eCALPAR(NET, UDP_MULTICAST_GROUP); }
    ECAL_API std::string       GetUdpMulticastMask                  () { return eCALPAR(NET, UDP_MULTICAST_MASK); }
    ECAL_API int               GetUdpMulticastPort                  () { return eCALPAR(NET, UDP_MULTICAST_PORT); }
    ECAL_API int               GetUdpMulticastTtl                   () { return eCALPAR(NET, UDP_MULTICAST_TTL); }

    ECAL_API int               GetUdpMulticastSndBufSizeBytes       () { return eCALPAR(NET, UDP_MULTICAST_SNDBUF); }
    ECAL_API int               GetUdpMulticastRcvBufSizeBytes       () { return eCALPAR(NET, UDP_MULTICAST_RCVBUF); }
    ECAL_API bool              IsUdpMulticastJoinAllIfEnabled       () { return eCALPAR(NET, UDP_MULTICAST_JOIN_ALL_IF_ENABLED); }


    ECAL_API int               GetMaxUdpBandwidthBytesPerSecond     () { return eCALPAR(NET, BANDWIDTH_MAX_UDP); }

    ECAL_API bool              IsUdpMulticastRecEnabled             () { return eCALPAR(NET, UDP_MC_REC_ENABLED); }
    ECAL_API bool              IsShmRecEnabled                      () { return eCALPAR(NET, SHM_REC_ENABLED); }
    ECAL_API bool              IsTcpRecEnabled                      () { return eCALPAR(NET, TCP_REC_ENABLED); }
    ECAL_API bool              IsInprocRecEnabled                   () { return eCALPAR(NET, INPROC_REC_ENABLED); }

    ECAL_API bool              IsNpcapEnabled                       () { return eCALPAR(NET, NPCAP_ENABLED); }

    ECAL_API int               GetTcpPubsubReaderThreadpoolSize     () { return eCALPAR(NET, TCP_PUBSUB_NUM_EXECUTOR_READER); }
    ECAL_API int               GetTcpPubsubWriterThreadpoolSize     () { return eCALPAR(NET, TCP_PUBSUB_NUM_EXECUTOR_WRITER); }
    ECAL_API int               GetTcpPubsubMaxReconnectionAttemps   () { return eCALPAR(NET, TCP_PUBSUB_MAX_RECONNECTIONS); }

    ECAL_API std::string       GetHostGroupName                     () { return eCALPAR(NET, HOST_GROUP_NAME); }
    
    /////////////////////////////////////
    // time
    /////////////////////////////////////
    
    ECAL_API std::string       GetTimesyncModuleName                () { return eCALPAR(TIME, SYNC_MOD_RT); }

    /////////////////////////////////////
    // process
    /////////////////////////////////////
    
    ECAL_API std::string       GetTerminalEmulatorCommand           () { return eCALPAR(PROCESS, TERMINAL_EMULATOR); }

    /////////////////////////////////////
    // monitoring
    /////////////////////////////////////
    
    ECAL_API int                 GetMonitoringTimeoutMs               () { return eCALPAR(MON, TIMEOUT); }
    ECAL_API std::string         GetMonitoringFilterExcludeList       () { return eCALPAR(MON, FILTER_EXCL); }
    ECAL_API std::string         GetMonitoringFilterIncludeList       () { return eCALPAR(MON, FILTER_INCL); }
    ECAL_API eCAL_Logging_Filter GetConsoleLogFilter                  () { return ParseLogLevel(eCALPAR(MON, LOG_FILTER_CON)); }
    ECAL_API eCAL_Logging_Filter GetFileLogFilter                     () { return ParseLogLevel(eCALPAR(MON, LOG_FILTER_FILE)); }
    ECAL_API eCAL_Logging_Filter GetUdpLogFilter                      () { return ParseLogLevel(eCALPAR(MON, LOG_FILTER_UDP)); }

    /////////////////////////////////////
    // sys
    /////////////////////////////////////
    
    ECAL_API std::string       GetEcalSysFilterExcludeList            () { return eCALPAR(SYS, FILTER_EXCL); }

    /////////////////////////////////////
    // publisher
    /////////////////////////////////////
    
    ECAL_API TLayer::eSendMode GetPublisherUdpMulticastMode         () { return TLayer::eSendMode(eCALPAR(PUB, USE_UDP_MC)); }
    ECAL_API TLayer::eSendMode GetPublisherShmMode                  () { return TLayer::eSendMode(eCALPAR(PUB, USE_SHM)); }
    ECAL_API TLayer::eSendMode GetPublisherTcpMode                  () { return TLayer::eSendMode(eCALPAR(PUB, USE_TCP)); }
    ECAL_API TLayer::eSendMode GetPublisherInprocMode               () { return TLayer::eSendMode(eCALPAR(PUB, USE_INPROC)); }

    ECAL_API size_t            GetMemfileMinsizeBytes               () { return static_cast<size_t>(eCALPAR(PUB, MEMFILE_MINSIZE)); }
    ECAL_API size_t            GetMemfileOverprovisioningPercentage () { return static_cast<size_t>(eCALPAR(PUB, MEMFILE_RESERVE)); }
    ECAL_API int               GetMemfileAckTimeoutMs               () { return eCALPAR(PUB, MEMFILE_ACK_TO); }
    ECAL_API bool              IsMemfileZerocopyEnabled             () { return (eCALPAR(PUB, MEMFILE_ZERO_COPY) != 0); }
    ECAL_API size_t            GetMemfileBufferCount                () { return static_cast<size_t>(eCALPAR(PUB, MEMFILE_BUF_COUNT)); }

    ECAL_API bool              IsTopicTypeSharingEnabled            () { return (eCALPAR(PUB, SHARE_TTYPE) != 0); }
    ECAL_API bool              IsTopicDescriptionSharingEnabled     () { return (eCALPAR(PUB, SHARE_TDESC) != 0); }

    /////////////////////////////////////
    // service
    /////////////////////////////////////
    ECAL_API bool              IsServiceProtocolV0Enabled           () { return (eCALPAR(SERVICE, PROTOCOL_V0) != 0); }
    ECAL_API bool              IsServiceProtocolV1Enabled           () { return (eCALPAR(SERVICE, PROTOCOL_V1) != 0); }

    /////////////////////////////////////
    // experimemtal
    /////////////////////////////////////

    namespace Experimental
    {
      ECAL_API bool              IsShmMonitoringEnabled             () { return eCALPAR(EXP, SHM_MONITORING_ENABLED); }
      ECAL_API bool              IsNetworkMonitoringDisabled        () { return eCALPAR(EXP, NETWORK_MONITORING_DISABLED); }
      ECAL_API size_t            GetShmMonitoringQueueSize          () { return static_cast<size_t>(eCALPAR(EXP, SHM_MONITORING_QUEUE_SIZE)); }
      ECAL_API std::string       GetShmMonitoringDomain             () { return eCALPAR(EXP, SHM_MONITORING_DOMAIN);}
      ECAL_API bool              GetDropOutOfOrderMessages          () { return eCALPAR(EXP, DROP_OUT_OF_ORDER_MESSAGES); }
    }
  }
}
