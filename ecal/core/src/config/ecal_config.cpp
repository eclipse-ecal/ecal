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

#include <ecal/ecal_config.h>
#include <string>
#include <vector>

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
    
    ECAL_API std::string       GetLoadedEcalIniPath                 () { return GetCurrentConfig().loaded_ecal_ini_file; }
    ECAL_API int               GetRegistrationTimeoutMs             () { return GetCurrentConfig().registration_options.getTimeoutMS(); }
    ECAL_API int               GetRegistrationRefreshMs             () { return GetCurrentConfig().registration_options.getRefreshMS(); }

    /////////////////////////////////////
    // network
    /////////////////////////////////////

    ECAL_API bool              IsNetworkEnabled                     () { return GetCurrentConfig().transport_layer_options.network_enabled; }

    ECAL_API UdpConfigVersion  GetUdpMulticastConfigVersion         () { return GetCurrentConfig().transport_layer_options.mc_options.config_version; }

    ECAL_API std::string       GetUdpMulticastGroup                 () { return GetCurrentConfig().transport_layer_options.mc_options.group; }
    ECAL_API std::string       GetUdpMulticastMask                  () { return GetCurrentConfig().transport_layer_options.mc_options.mask; }
    ECAL_API int               GetUdpMulticastPort                  () { return GetCurrentConfig().transport_layer_options.mc_options.port; }
    ECAL_API int               GetUdpMulticastTtl                   () { return GetCurrentConfig().transport_layer_options.mc_options.ttl; }

    ECAL_API int               GetUdpMulticastSndBufSizeBytes       () { return GetCurrentConfig().transport_layer_options.mc_options.sndbuf; }
    ECAL_API int               GetUdpMulticastRcvBufSizeBytes       () { return GetCurrentConfig().transport_layer_options.mc_options.recbuf; }
    ECAL_API bool              IsUdpMulticastJoinAllIfEnabled       () { return GetCurrentConfig().transport_layer_options.mc_options.join_all_interfaces; }

    ECAL_API bool              IsUdpMulticastRecEnabled             () { return GetCurrentConfig().subscriber_options.udp.enable; }
    ECAL_API bool              IsShmRecEnabled                      () { return GetCurrentConfig().subscriber_options.shm.enable; }
    ECAL_API bool              IsTcpRecEnabled                      () { return GetCurrentConfig().subscriber_options.tcp.enable; }

    ECAL_API bool              IsNpcapEnabled                       () { return GetCurrentConfig().transport_layer_options.mc_options.npcap_enabled; }

    ECAL_API int               GetTcpPubsubReaderThreadpoolSize     () { return static_cast<int>(GetCurrentConfig().transport_layer_options.tcp_options.num_executor_reader); }
    ECAL_API int               GetTcpPubsubWriterThreadpoolSize     () { return static_cast<int>(GetCurrentConfig().transport_layer_options.tcp_options.num_executor_writer); }
    ECAL_API int               GetTcpPubsubMaxReconnectionAttemps   () { return static_cast<int>(GetCurrentConfig().transport_layer_options.tcp_options.max_reconnections); }

    ECAL_API std::string       GetHostGroupName                     () { return GetCurrentConfig().transport_layer_options.shm_options.host_group_name; }
    
    /////////////////////////////////////
    // time
    /////////////////////////////////////
    
    ECAL_API std::string       GetTimesyncModuleName                () { return GetCurrentConfig().timesync_options.timesync_module_rt; }
    ECAL_API std::string       GetTimesyncModuleReplay              () { return GetCurrentConfig().timesync_options.timesync_module_replay; }

    /////////////////////////////////////
    // process
    /////////////////////////////////////
    
    ECAL_API std::string       GetTerminalEmulatorCommand           () { return GetCurrentConfig().application_options.startup_options.terminal_emulator; }

    /////////////////////////////////////
    // monitoring
    /////////////////////////////////////
    
    ECAL_API int                 GetMonitoringTimeoutMs             () { return GetCurrentConfig().monitoring_options.monitoring_timeout; }
    ECAL_API std::string         GetMonitoringFilterExcludeList     () { return GetCurrentConfig().monitoring_options.filter_excl; }
    ECAL_API std::string         GetMonitoringFilterIncludeList     () { return GetCurrentConfig().monitoring_options.filter_incl; }
    ECAL_API eCAL_Logging_Filter GetConsoleLogFilter                () { return GetCurrentConfig().logging_options.filter_log_con; }
    ECAL_API eCAL_Logging_Filter GetFileLogFilter                   () { return GetCurrentConfig().logging_options.filter_log_file; }
    ECAL_API eCAL_Logging_Filter GetUdpLogFilter                    () { return GetCurrentConfig().logging_options.filter_log_udp; }

    /////////////////////////////////////
    // sys
    /////////////////////////////////////
    
    ECAL_API std::string       GetEcalSysFilterExcludeList          () { return GetCurrentConfig().application_options.sys_options.filter_excl; }

    /////////////////////////////////////
    // publisher
    /////////////////////////////////////
    
    ECAL_API bool              GetPublisherUdpMulticastMode         () { return GetCurrentConfig().publisher_options.udp.enable; }
    ECAL_API bool              GetPublisherShmMode                  () { return GetCurrentConfig().publisher_options.shm.enable; }
    ECAL_API bool              GetPublisherTcpMode                  () { return GetCurrentConfig().publisher_options.tcp.enable; }

    ECAL_API size_t            GetMemfileMinsizeBytes               () { return GetCurrentConfig().transport_layer_options.shm_options.memfile_minsize; }
    ECAL_API size_t            GetMemfileOverprovisioningPercentage () { return GetCurrentConfig().transport_layer_options.shm_options.memfile_reserve; }
    ECAL_API int               GetMemfileAckTimeoutMs               () { return GetCurrentConfig().transport_layer_options.shm_options.memfile_ack_timeout; }
    ECAL_API bool              IsMemfileZerocopyEnabled             () { return GetCurrentConfig().transport_layer_options.shm_options.memfile_zero_copy; }
    ECAL_API size_t            GetMemfileBufferCount                () { return GetCurrentConfig().transport_layer_options.shm_options.memfile_buffer_count; }

    ECAL_API bool              IsTopicTypeSharingEnabled            () { return GetCurrentConfig().registration_options.share_ttype; }
    ECAL_API bool              IsTopicDescriptionSharingEnabled     () { return GetCurrentConfig().registration_options.share_tdesc; }

    /////////////////////////////////////
    // service
    /////////////////////////////////////
    ECAL_API bool              IsServiceProtocolV0Enabled           () { return GetCurrentConfig().service_options.protocol_v0; }
    ECAL_API bool              IsServiceProtocolV1Enabled           () { return GetCurrentConfig().service_options.protocol_v1; }

    /////////////////////////////////////
    // experimemtal
    /////////////////////////////////////

    namespace Experimental
    {
      ECAL_API bool              IsShmMonitoringEnabled             () { return (GetCurrentConfig().monitoring_options.monitoring_mode & MonitoringMode::shm_monitoring) != 0; }
      ECAL_API bool              IsNetworkMonitoringDisabled        () { return !GetCurrentConfig().monitoring_options.network_monitoring; }
      ECAL_API size_t            GetShmMonitoringQueueSize          () { return GetCurrentConfig().monitoring_options.shm_options.shm_monitoring_queue_size; }
      ECAL_API std::string       GetShmMonitoringDomain             () { return GetCurrentConfig().monitoring_options.shm_options.shm_monitoring_domain;}
      ECAL_API bool              GetDropOutOfOrderMessages          () { return GetCurrentConfig().transport_layer_options.drop_out_of_order_messages; }
    }
  }
}
