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

/**
 * @brief  Function definitions for setting default config values
**/

#include "ecal/ecal_config.h"

#include "ecal_global_accessors.h"
#include "ecal_def.h"
#include "config/ecal_config_reader.h"

#include "ecal/ecal_process.h"
#include "config/ecal_cmd_parser.h"

#include <algorithm>

constexpr const char* COMMON       = "common";
constexpr const char* MONITORING   = "monitoring";
constexpr const char* NETWORK      = "network";
constexpr const char* EXPERIMENTAL = "experimental";
constexpr const char* PUBLISHER    = "publisher";
constexpr const char* SYS          = "sys";
constexpr const char* TIME         = "time";
constexpr const char* SERVICE      = "service";
constexpr const char* PROCESS      = "process";

namespace {
  void tokenize(const std::string& str, std::vector<std::string>& tokens,
      const std::string& delimiters = " ", bool trimEmpty = false)
  {
    std::string::size_type pos = 0;
    std::string::size_type lastPos = 0;

    for (;;)
    {
      pos = str.find_first_of(delimiters, lastPos);
      if (pos == std::string::npos)
      {
        pos = str.length();
        if (pos != lastPos || !trimEmpty)
        {
          tokens.emplace_back(str.data() + lastPos, pos - lastPos);
        }
        break;
      }
      else
      {
        if (pos != lastPos || !trimEmpty)
        {
          tokens.emplace_back(str.data() + lastPos, pos - lastPos);
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
  };
}

namespace eCAL 
{
    void Configuration::InitConfig(std::string ini_path_ /*= std::string("")*/)
    {
      CConfig iniConfig;
      if (!command_line_arguments.config_keys.empty())
        iniConfig.OverwriteKeys(command_line_arguments.config_keys);

      if (!ini_path_.empty())
      {
        iniConfig.AddFile(ini_path_);
        ecal_ini_file_path = ini_path_;
      }

      // transport layer options
      auto& transportLayerOptions = transport_layer;
      transportLayerOptions.drop_out_of_order_messages = iniConfig.get(EXPERIMENTAL, "drop_out_of_order_messages", EXP_DROP_OUT_OF_ORDER_MESSAGES);

      auto& multicastOptions = transportLayerOptions.mc_options;
      
      const std::string udp_config_version_string = iniConfig.get(NETWORK, "multicast_config_version", "v1");
      if (udp_config_version_string == "v1")
        multicastOptions.config_version = Types::UdpConfigVersion::V1;
      if (udp_config_version_string == "v2")
        multicastOptions.config_version = Types::UdpConfigVersion::V2;
      
      multicastOptions.group               = iniConfig.get(NETWORK, "multicast_group",       NET_UDP_MULTICAST_GROUP);
      multicastOptions.mask                = iniConfig.get(NETWORK, "multicast_mask",        NET_UDP_MULTICAST_MASK);
      multicastOptions.port                = iniConfig.get(NETWORK, "multicast_port",        NET_UDP_MULTICAST_PORT);
      multicastOptions.ttl                 = iniConfig.get(NETWORK, "multicast_ttl",         NET_UDP_MULTICAST_TTL);
      multicastOptions.recbuf              = iniConfig.get(NETWORK, "multicast_rcvbuf",      NET_UDP_MULTICAST_RCVBUF);
      multicastOptions.sndbuf              = iniConfig.get(NETWORK, "multicast_sndbuf",      NET_UDP_MULTICAST_SNDBUF);
      multicastOptions.join_all_interfaces = iniConfig.get(NETWORK, "multicast_join_all_if", NET_UDP_MULTICAST_JOIN_ALL_IF_ENABLED);
      multicastOptions.npcap_enabled       = iniConfig.get(NETWORK, "npcap_enabled",         NET_NPCAP_ENABLED);

      auto& shmOptions = transportLayerOptions.shm_options;
      shmOptions.host_group_name            = iniConfig.get(NETWORK,      "host_group_name",            NET_HOST_GROUP_NAME);

      // registration options
      auto registrationTimeout              = iniConfig.get(COMMON,    "registration_timeout", CMN_REGISTRATION_TO);
      auto registrationRefresh              = iniConfig.get(COMMON,    "registration_refresh", CMN_REGISTRATION_REFRESH);
      registration = Registration::Configuration(registrationTimeout, registrationRefresh);
      auto& registrationOptions = registration;
      registrationOptions.network_enabled          = iniConfig.get(NETWORK,   "network_enabled", NET_ENABLED);
      registrationOptions.shm_registration_enabled = iniConfig.get(NETWORK,   "shm_registration_enabled", SHM_REGISTRATION_ENABLED);
      registrationOptions.share_tdesc              = iniConfig.get(PUBLISHER, "share_tdesc",     PUB_SHARE_TDESC);
      registrationOptions.share_ttype              = iniConfig.get(PUBLISHER, "share_ttype",     PUB_SHARE_TTYPE);

      // monitoring options
      auto& monitoringOptions = monitoring;
      monitoringOptions.monitoring_timeout          = iniConfig.get(MONITORING,   "timeout", MON_TIMEOUT);;
      monitoringOptions.filter_excl                 = iniConfig.get(MONITORING,   "filter_excl",                 MON_FILTER_EXCL);
      monitoringOptions.filter_incl                 = iniConfig.get(MONITORING,   "filter_incl",                 MON_FILTER_INCL);

      // auto& udpMonitoringOptions = monitoringOptions.udp_options;
      // TODO: Nothing here yet

      auto& shmMonitoringOptions = monitoringOptions.shm_options;
      shmMonitoringOptions.shm_monitoring_domain     = iniConfig.get(EXPERIMENTAL, "shm_monitoring_domain",     EXP_SHM_MONITORING_DOMAIN);
      shmMonitoringOptions.shm_monitoring_queue_size = iniConfig.get(EXPERIMENTAL, "shm_monitoring_queue_size", EXP_SHM_MONITORING_QUEUE_SIZE);

      // subscriber options
      auto& subscriberOptions = subscriber;
      subscriberOptions.shm.enable = iniConfig.get(NETWORK, "shm_rec_enabled",    NET_SHM_REC_ENABLED) != 0;
      
      subscriberOptions.tcp.enable              = iniConfig.get(NETWORK, "tcp_rec_enabled",                NET_TCP_REC_ENABLED) != 0;
      subscriberOptions.tcp.max_reconnections   = iniConfig.get(NETWORK, "tcp_pubsup_max_reconnections",   NET_TCP_PUBSUB_MAX_RECONNECTIONS);
      subscriberOptions.tcp.num_executor_reader = iniConfig.get(NETWORK, "tcp_pubsup_num_executor_reader", NET_TCP_PUBSUB_NUM_EXECUTOR_READER);
      subscriberOptions.tcp.num_executor_writer = iniConfig.get(NETWORK, "tcp_pubsup_num_executor_writer", NET_TCP_PUBSUB_NUM_EXECUTOR_WRITER);
      
      subscriberOptions.udp.enable = iniConfig.get(NETWORK, "udp_mc_rec_enabled", NET_UDP_MC_REC_ENABLED) != 0;

      // publisher options
      auto& publisherOptions = publisher;
      publisherOptions.shm.enable                  = iniConfig.get(PUBLISHER, "use_shm",              static_cast<int>(PUB_USE_SHM)) != 0;
      publisherOptions.shm.memfile_min_size_bytes  = iniConfig.get(PUBLISHER, "memfile_minsize",      PUB_MEMFILE_MINSIZE);
      publisherOptions.shm.memfile_reserve_percent = iniConfig.get(PUBLISHER, "memfile_reserve",      PUB_MEMFILE_RESERVE);
      
      publisherOptions.udp.enable            = iniConfig.get(PUBLISHER, "use_udp_mc",     static_cast<int>(PUB_USE_UDP_MC)) != 0;
      // TODO PG: Add here when its available in config file
      publisherOptions.udp.loopback          = false; 
      publisherOptions.udp.sndbuf_size_bytes = iniConfig.get(NETWORK, "multicast_sndbuf", NET_UDP_MULTICAST_SNDBUF);
      
      publisherOptions.share_topic_description = iniConfig.get(PUBLISHER, "share_tdesc", PUB_SHARE_TDESC);
      publisherOptions.share_topic_type        = iniConfig.get(PUBLISHER, "share_ttype", PUB_SHARE_TTYPE);

      publisherOptions.tcp.enable = iniConfig.get(PUBLISHER, "use_tcp",    static_cast<int>(PUB_USE_TCP)) != 0;
      publisherOptions.tcp.num_executor_reader = iniConfig.get(NETWORK, "tcp_pubsup_num_executor_reader", NET_TCP_PUBSUB_NUM_EXECUTOR_READER);
      publisherOptions.tcp.num_executor_writer = iniConfig.get(NETWORK, "tcp_pubsup_num_executor_writer", NET_TCP_PUBSUB_NUM_EXECUTOR_WRITER);

      // timesync options
      auto& timesyncOptions = timesync;
      timesyncOptions.timesync_module_rt     = iniConfig.get(TIME, "timesync_module_rt", TIME_SYNC_MODULE);
      timesyncOptions.timesync_module_replay = iniConfig.get(TIME, "timesync_module_replay", TIME_SYNC_MOD_REPLAY);

      // service options
      auto& serviceOptions = service;
      serviceOptions.protocol_v0 = iniConfig.get(SERVICE, "protocol_v0", SERVICE_PROTOCOL_V0);
      serviceOptions.protocol_v1 = iniConfig.get(SERVICE, "protocol_v1", SERVICE_PROTOCOL_V1);

      // sys options
      auto& sysConfig = application.sys;
      sysConfig.filter_excl = iniConfig.get(SYS, "filter_excl", SYS_FILTER_EXCL);

      // process options
      auto& startupConfig = application.startup;
      startupConfig.terminal_emulator = iniConfig.get(PROCESS, "terminal_emulator", PROCESS_TERMINAL_EMULATOR);

      auto& loggingOptions = logging;
      // needs to be adapted when switching from simpleini
      loggingOptions.filter_log_con              = ParseLogLevel(iniConfig.get(MONITORING, "filter_log_con",  "info,warning,error,fatal"));
      loggingOptions.filter_log_file             = ParseLogLevel(iniConfig.get(MONITORING, "filter_log_file", ""));
      loggingOptions.filter_log_udp              = ParseLogLevel(iniConfig.get(MONITORING, "filter_log_udp",  "info,warning,error,fatal"));
    };

    Configuration::Configuration(int argc_ , char **argv_)
    {
      std::vector<std::string> arguments;
      if (argc_ > 0 && argv_ != nullptr)
      {
        for (size_t i = 0; i < static_cast<size_t>(argc_); ++i) 
          if (argv_[i] != nullptr) 
            arguments.emplace_back(argv_[i]);
      } 
      Init(arguments);
    }

    Configuration::Configuration(std::vector<std::string>& args_)
    {      
      Init(args_);
    }

    void Configuration::Init(std::vector<std::string>& arguments_)
    {
      Config::CmdParser parser(arguments_);
      
      command_line_arguments.config_keys       = parser.getConfigKeys();
      command_line_arguments.specified_config  = parser.getUserIni();
      command_line_arguments.dump_config       = parser.getDumpConfig();
      command_line_arguments.config_keys_map   = parser.getConfigKeysMap();

      InitConfig(command_line_arguments.specified_config);
    }

    void Configuration::InitConfigWithDefaultIni()
    {
      InitConfig(g_default_ini_file);
    }

    Configuration::Configuration()
    : Configuration(0, nullptr)
    {
      InitConfig();
    }

    std::string Configuration::GetIniFilePath()
    {
      return ecal_ini_file_path;
    }

    Configuration& GetConfiguration()
    {
      return g_ecal_configuration;
    };
}