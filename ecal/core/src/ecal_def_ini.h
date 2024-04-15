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

/**
 * @brief  eCAL ini file keys
**/

#pragma once

/////////////////////////////////////
// common
/////////////////////////////////////
constexpr const char*   CMN_SECTION_S                             = "common";
constexpr const char*   CMN_REGISTRATION_TO_S                     = "registration_timeout";
constexpr const char*   CMN_REGISTRATION_REFRESH_S                = "registration_refresh";

/////////////////////////////////////
// network
/////////////////////////////////////
constexpr const char*   NET_SECTION_S                             = "network";

constexpr const char*   NET_ENABLED_S                             = "network_enabled";

constexpr const char*   NET_UDP_MULTICAST_CONFIG_VERSION_S        = "multicast_config_version";
constexpr const char*   NET_UDP_MULTICAST_GROUP_S                 = "multicast_group";
constexpr const char*   NET_UDP_MULTICAST_MASK_S                  = "multicast_mask";
constexpr const char*   NET_UDP_MULTICAST_PORT_S                  = "multicast_port";
constexpr const char*   NET_UDP_MULTICAST_TTL_S                   = "multicast_ttl";

constexpr const char*   NET_UDP_MULTICAST_SNDBUF_S                = "multicast_sndbuf";
constexpr const char*   NET_UDP_MULTICAST_RCVBUF_S                = "multicast_rcvbuf";

constexpr const char*   NET_UDP_MULTICAST_JOIN_ALL_IF_ENABLED_S   = "multicast_join_all_if";

constexpr const char*   NET_UDP_MC_REC_ENABLED_S                  = "udp_mc_rec_enabled";
constexpr const char*   NET_SHM_REC_ENABLED_S                     = "shm_rec_enabled";
constexpr const char*   NET_TCP_REC_ENABLED_S                     = "tcp_rec_enabled";

constexpr const char*   NET_NPCAP_ENABLED_S                       = "npcap_enabled";

constexpr const char*   NET_TCP_PUBSUB_NUM_EXECUTOR_READER_S      = "tcp_pubsub_num_executor_reader";
constexpr const char*   NET_TCP_PUBSUB_NUM_EXECUTOR_WRITER_S      = "tcp_pubsub_num_executor_writer";
constexpr const char*   NET_TCP_PUBSUB_MAX_RECONNECTIONS_S        = "tcp_pubsub_max_reconnections";

constexpr const char*   NET_HOST_GROUP_NAME_S                     = "host_group_name";

/////////////////////////////////////
// time
/////////////////////////////////////
constexpr const char*   TIME_SECTION_S                            = "time";
constexpr const char*   TIME_SYNC_MOD_RT_S                        = "timesync_module_rt";
constexpr const char*   TIME_SYNC_MOD_REPLAY_S                    = "timesync_module_replay";

/////////////////////////////////////
// process
/////////////////////////////////////
constexpr const char*   PROCESS_SECTION_S                         = "process";
constexpr const char*   PROCESS_TERMINAL_EMULATOR_S               = "terminal_emulator";

/////////////////////////////////////
// monitoring
/////////////////////////////////////
constexpr const char*   MON_SECTION_S                             = "monitoring";

constexpr const char*   MON_TIMEOUT_S                             = "timeout";
constexpr const char*   MON_FILTER_EXCL_S                         = "filter_excl";
constexpr const char*   MON_FILTER_INCL_S                         = "filter_incl";

constexpr const char*   MON_LOG_FILTER_CON_S                      = "filter_log_con";
constexpr const char*   MON_LOG_FILTER_FILE_S                     = "filter_log_file";
constexpr const char*   MON_LOG_FILTER_UDP_S                      = "filter_log_udp";

/////////////////////////////////////
// sys
/////////////////////////////////////
constexpr const char*   SYS_SECTION_S                             = "sys";
constexpr const char*   SYS_FILTER_EXCL_S                         = "filter_excl";

/////////////////////////////////////
// publisher
/////////////////////////////////////
constexpr const char*   PUB_SECTION_S                             = "publisher";

constexpr const char*   PUB_USE_UDP_MC_S                          = "use_udp_mc";
constexpr const char*   PUB_USE_SHM_S                             = "use_shm";
constexpr const char*   PUB_USE_TCP_S                             = "use_tcp";

constexpr const char*   PUB_MEMFILE_MINSIZE_S                     = "memfile_minsize";
constexpr const char*   PUB_MEMFILE_RESERVE_S                     = "memfile_reserve";
constexpr const char*   PUB_MEMFILE_ACK_TO_S                      = "memfile_ack_timeout";
constexpr const char*   PUB_MEMFILE_ZERO_COPY_S                   = "memfile_zero_copy";
constexpr const char*   PUB_MEMFILE_BUF_COUNT_S                   = "memfile_buffer_count";

constexpr const char*   PUB_SHARE_TTYPE_S                         = "share_ttype";
constexpr const char*   PUB_SHARE_TDESC_S                         = "share_tdesc";

/////////////////////////////////////
// service
/////////////////////////////////////
constexpr const char*   SERVICE_SECTION_S                         = "service";

constexpr const char*   SERVICE_PROTOCOL_V0_S                     = "protocol_v0";
constexpr const char*   SERVICE_PROTOCOL_V1_S                     = "protocol_v1";

/////////////////////////////////////
// experimental
/////////////////////////////////////
constexpr const char*   EXP_SECTION_S                             = "experimental";

constexpr const char*   EXP_SHM_MONITORING_ENABLED_S              = "shm_monitoring_enabled";
constexpr const char*   EXP_NETWORK_MONITORING_DISABLED_S         = "network_monitoring_disabled";
constexpr const char*   EXP_SHM_MONITORING_QUEUE_SIZE_S           = "shm_monitoring_queue_size";
constexpr const char*   EXP_SHM_MONITORING_DOMAIN_S               = "shm_monitoring_domain";
constexpr const char*   EXP_DROP_OUT_OF_ORDER_MESSAGES_S          = "drop_out_of_order_messages";
