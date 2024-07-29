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
 * @brief  eCAL global definition file
**/

#pragma once

#include <ecal/config/configuration.h>
#include <ecal/ecal_tlayer.h>
#include <ecal/ecal_log_level.h>
#include <ecal/ecal_os.h>
#include <array>

/**********************************************************************************************/
/*                                  registration settings                                     */
/**********************************************************************************************/

constexpr unsigned int                  REGISTRATION_REGISTRATION_TIMEOUT           = (60U*1000U);
constexpr unsigned int                  REGISTRATION_REGISTRATION_REFRESH           = 1000U;
constexpr  bool                         REGISTRATION_LOOPBACK                       = true;
constexpr const char*                   REGISTRATION_HOST_GROUP_NAME                = "";
constexpr bool                          REGISTRATION_NETWORK_ENABLED                = false;
constexpr bool                          REGISTRATION_LAYER_SHM_ENABLE               = false;
constexpr const char*                   REGISTRATION_LAYER_SHM_DOMAIN               = "ecal_mon";
constexpr unsigned int                  REGISTRATION_LAYER_SHM_QUEUE_SIZE           = 1024U;
constexpr bool                          REGISTRATION_LAYER_UDP_ENABLE               = true;
constexpr unsigned int                  REGISTRATION_LAYER_UDP_PORT                 = 14000U;

/**********************************************************************************************/
/*                                  monitoring settings                                       */
/**********************************************************************************************/
constexpr unsigned int                  MONITORING_TIMEOUT                          = 5000U;
constexpr const char*                   MONITORING_FILTER_EXCL                      = "^__.*$";
constexpr const char*                   MONITORING_FILTER_INCL                      = "";

/**********************************************************************************************/
/*                                  transport_layer settings                                  */
/**********************************************************************************************/
constexpr eCAL::Types::UdpConfigVersion TRANSPORT_LAYER_UDP_CONFIG_VERSION          = eCAL::Types::UdpConfigVersion::V2;
constexpr eCAL::Types::UDPMode          TRANSPORT_LAYER_UDP_MODE                    = eCAL::Types::UDPMode::LOCAL;
constexpr unsigned int                  TRANSPORT_LAYER_UDP_PORT                    = 14002U;
constexpr const char*                   TRANSPORT_LAYER_UDP_MASK                    = "255.255.255.240";
constexpr unsigned int                  TRANSPORT_LAYER_UDP_SEND_BUFFER             = (5U*1024U*1024U);  /* 5 MByte */
constexpr unsigned int                  TRANSPORT_LAYER_UDP_RECEIVE_BUFFER          = (5U*1024U*1024U);  /* 5 MByte */
constexpr bool                          TRANSPORT_LAYER_UDP_JOIN_ALL_INTERFACES     = false;
constexpr bool                          TRANSPORT_LAYER_UDP_NPCAP_ENABLED           = false;
constexpr const char*                   TRANSPORT_LAYER_UDP_NETWORK_GROUP           = "239.0.0.1";
constexpr unsigned int                  TRANSPORT_LAYER_UDP_NETWORK_TTL             = 3U;


constexpr unsigned int                  TRANSPORT_LAYER_TCP_NUMBER_EXECUTOR_READER  = 4U;
constexpr unsigned int                  TRANSPORT_LAYER_TCP_NUMBER_EXECUTOR_WRITER  = 4U;
constexpr unsigned int                  TRANSPORT_LAYER_TCP_MAX_RECONNECTIONS       = 5U;

constexpr unsigned int                  TRANSPORT_LAYER_SHM_MEMFILE_MIN_SIZE_BYTES  = (4U*1024U);
constexpr unsigned int                  TRANSPORT_LAYER_SHM_MEMFILE_RESERVE_PERCENT = 50U;

/**********************************************************************************************/
/*                                  publisher settings                                        */
/**********************************************************************************************/
constexpr bool                          PUBLISHER_LAYER_SHM_ENABLE                  = true;
constexpr bool                          PUBLISHER_LAYER_SHM_ZERO_COPY_MODE          = false;
constexpr unsigned int                  PUBLISHER_LAYER_SHM_ACKNOWLEDGE_TIMEOUT_MS  = 0U;
constexpr unsigned int                  PUBLISHER_LAYER_SHM_MEMFILE_BUFFER_COUNT    = 1U;

constexpr bool                          PUBLISHER_LAYER_UDP_ENABLE                  = true;

constexpr bool                          PUBLISHER_LAYER_TCP_ENABLE                  = true;

constexpr bool                          PUBLISHER_SHARE_TOPIC_TYPE                  = true;
constexpr bool                          PUBLISHER_SHARE_TOPIC_DESCRIPTION           = true;

constexpr std::array<const char*, 3>    PUBLISHER_PRIORITY_LOCAL                    = { "shm", "udp", "tcp"};
constexpr std::array<const char*, 2>    PUBLISHER_PRIORITY_NETWORK                  = { "udp", "tcp"};


/**********************************************************************************************/
/*                                  subscriber settings                                       */
/**********************************************************************************************/
constexpr bool                          SUBSCRIBER_LAYER_SHM_ENABLE                 = true;
constexpr bool                          SUBSCRIBER_LAYER_UDP_ENABLE                 = true;
constexpr bool                          SUBSCRIBER_LAYER_TCP_ENABLE                 = false;
constexpr bool                          SUBSCRIBER_DROP_OUT_OF_ORDER_MESSAGES       = true;

/**********************************************************************************************/
/*                                  time settings                                             */
/**********************************************************************************************/
constexpr const char*                   TIME_RT                                     = "ecaltime-localtime";
constexpr const char*                   TIME_REPLAY                                 = "";

/**********************************************************************************************/
/*                                  service settings                                          */
/**********************************************************************************************/
constexpr bool                          SERVICE_PROTOCOL_V0                         = true;
constexpr bool                          SERVICE_PROTOCOL_V1                         = false;

/**********************************************************************************************/
/*                                  application settings                                      */
/**********************************************************************************************/
constexpr const char*                   APPLICATION_SYS_FILTER_EXCL                 =  "^eCALSysClient$|^eCALSysGUI$|^eCALSys$*";
constexpr const char*                   APPLICATION_TERMINAL_EMULATOR               = "";
/**********************************************************************************************/
/*                                  logging settings                                          */
/**********************************************************************************************/
constexpr bool                          LOGGING_SINKS_CONSOLE_ENABLE                = false;
constexpr std::array<const char*, 4>    LOGGING_SINKS_CONSOLE_LEVEL                 = {"info", "warning", "error", "fatal"};
constexpr bool                          LOGGING_SINKS_FILE_ENABLE                   = false;
constexpr std::array<const char*, 0>    LOGGING_SINKS_FILE_LEVEL                    = {};
constexpr const char*                   LOGGING_SINKS_FILE_PATH                     = "";
constexpr bool                          LOGGING_SINKS_UDP_ENABLE                    = true;
constexpr std::array<const char*, 4>    LOGGING_SINKS_UDP_LEVEL                     = {"info", "warning", "error", "fatal"};
constexpr unsigned int                  LOGGING_SINKS_UDP_PORT                      = 14001U;

/**********************************************************************************************/
/*                                     config settings                                        */
/**********************************************************************************************/
/* base data path name */
constexpr const char* ECAL_HOME_PATH_WINDOWS = "";
constexpr const char* ECAL_HOME_PATH_LINUX   = ".ecal";
constexpr const char* ECAL_LOG_PATH          = "logs";
constexpr const char* ECAL_SETTINGS_PATH     = "cfg";

/* ini file name */
constexpr const char* ECAL_DEFAULT_CFG       = "ecal.yaml";

/* eCAL udp multicast defines */
constexpr unsigned int NET_UDP_MULTICAST_PORT_REG_OFF                       = 0U;
constexpr unsigned int NET_UDP_MULTICAST_PORT_LOG_OFF                       = 1U;
constexpr unsigned int NET_UDP_MULTICAST_PORT_SAMPLE_OFF                    = 2U;


/* timeout for create / open a memory file using mutex lock in ms */
constexpr unsigned int PUB_MEMFILE_CREATE_TO              = 200U;
constexpr unsigned int PUB_MEMFILE_OPEN_TO                = 200U;


/**********************************************************************************************/
/*                                     events                                                 */
/**********************************************************************************************/
/* common stop event prefix to shut down a local user process */
constexpr const char* EVENT_SHUTDOWN_PROC                   = "ecal_shutdown_process";

/**********************************************************************************************/
/*                                     experimental                                           */
/**********************************************************************************************/
/* memory file access timeout */
constexpr unsigned int EXP_MEMFILE_ACCESS_TIMEOUT           = 100U;
