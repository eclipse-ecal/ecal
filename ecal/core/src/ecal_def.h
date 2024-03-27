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
 * @brief  eCAL global definition file
**/

#pragma once

#include <ecal/types/ecal_config_types.h>
#include <ecal/ecal_os.h>

/**********************************************************************************************/
/*                                     config settings                                        */
/**********************************************************************************************/
/* base data path name */
constexpr char ECAL_HOME_PATH_WINDOWS[] = "";
constexpr char ECAL_HOME_PATH_LINUX[]   = ".ecal";
constexpr char ECAL_LOG_PATH[]          = "logs";
constexpr char ECAL_SETTINGS_PATH[]     = "cfg";

/* ini file name */
constexpr char ECAL_DEFAULT_CFG[]       = "ecal.ini";

/**********************************************************************************************/
/*                                     monitor settings                                       */
/**********************************************************************************************/
/* timeout for automatic removing monitoring topics in ms */
constexpr unsigned int MON_TIMEOUT      = 5000U;
/* topics blacklist as regular expression (will not be monitored) */
constexpr char MON_FILTER_EXCL[]        =    "^__.*$";
/* topics whitelist as regular expression (will be monitored only) */
constexpr char MON_FILTER_INCL[]        =    "";

/* logging filter settings */
constexpr char MON_LOG_FILTER_CON[]     = "info,warning,error,fatal";
constexpr char MON_LOG_FILTER_FILE[]    = "";
constexpr char MON_LOG_FILTER_UDP[]     = "info,warning,error,fatal";


/**********************************************************************************************/
/*                                     sys settings                                       */
/**********************************************************************************************/
/* sys app witch will not be imported from cloud */
constexpr char SYS_FILTER_EXCL[]        = "^eCALSysClient$|^eCALSysGUI$|^eCALSys$*";

/**********************************************************************************************/
/*                                     network settings                                       */
/**********************************************************************************************/
/* network switch */
constexpr bool NET_ENABLED              = false;

/* eCAL udp multicast defines */
constexpr char NET_UDP_MULTICAST_CONFIG_VERSION[]         = "v1";
constexpr char NET_UDP_MULTICAST_GROUP[]                  = "239.0.0.1";
constexpr char NET_UDP_MULTICAST_MASK[]                   = "0.0.0.15";
constexpr unsigned int NET_UDP_MULTICAST_PORT             = 14000;
constexpr unsigned int NET_UDP_MULTICAST_TTL              = 3;
constexpr unsigned int NET_UDP_MULTICAST_PORT_REG_OFF     = 0;
constexpr unsigned int NET_UDP_MULTICAST_PORT_LOG_OFF     = 1;
constexpr unsigned int NET_UDP_MULTICAST_PORT_SAMPLE_OFF  = 2;
constexpr unsigned int NET_UDP_MULTICAST_SNDBUF           = (5*1024*1024);  /* 5 MByte */
constexpr unsigned int NET_UDP_MULTICAST_RCVBUF           = (5*1024*1024);  /* 5 MByte */
constexpr bool NET_UDP_MULTICAST_JOIN_ALL_IF_ENABLED      = false;

constexpr unsigned int NET_UDP_RECBUFFER_TIMEOUT          = 1000;  /* ms */
constexpr unsigned int NET_UDP_RECBUFFER_CLEANUP          = 10;    /* ms */

/* overall udp multicast bandwidth limitation in bytes/s, -1 == no limitation*/
constexpr int NET_BANDWIDTH_MAX_UDP                       = (-1);

constexpr bool NET_TCP_REC_ENABLED                        = true;
constexpr bool NET_SHM_REC_ENABLED                        = true;

constexpr bool NET_UDP_MC_REC_ENABLED                     = true;

constexpr bool NET_NPCAP_ENABLED                          = false;

constexpr unsigned int NET_TCP_PUBSUB_NUM_EXECUTOR_READER = 4;
constexpr unsigned int NET_TCP_PUBSUB_NUM_EXECUTOR_WRITER = 4;
constexpr unsigned int NET_TCP_PUBSUB_MAX_RECONNECTIONS   = 5;

/* common host group name that enables interprocess mechanisms across (virtual) host borders (e.g, Docker); by default equivalent to local host name */
constexpr char NET_HOST_GROUP_NAME[]                      = "";

/**********************************************************************************************/
/*                                     publisher settings                                     */
/**********************************************************************************************/
/* use shared memory transport layer [auto = 2, on = 1, off = 0] */
constexpr unsigned int PUB_USE_SHM                        = 2;
/* use tcp transport layer           [auto = 2, on = 1, off = 0] */
constexpr unsigned int PUB_USE_TCP                        = 0;
/* use udp multicast transport layer [auto = 2, on = 1, off = 0] */
constexpr unsigned int PUB_USE_UDP_MC                     = 2;

/* share topic type                  [          on = 1, off = 0] */
constexpr unsigned int PUB_SHARE_TTYPE                    = 1;
/* share topic description           [          on = 1, off = 0] */
constexpr unsigned int PUB_SHARE_TDESC                    = 1;

/* minimum size for created shared memory files */
constexpr unsigned int PUB_MEMFILE_MINSIZE                = (4*1024);
/* reserve buffer size before reallocation in % */
constexpr unsigned int PUB_MEMFILE_RESERVE                = 50;

/* timeout for create / open a memory file using mutex lock in ms */
constexpr unsigned int PUB_MEMFILE_CREATE_TO              = 200;
constexpr unsigned int PUB_MEMFILE_OPEN_TO                = 200;

/* timeout for memory read acknowledge signal from data reader in ms */
constexpr unsigned int PUB_MEMFILE_ACK_TO                 = 0;  /* ms */

/* defines number of memory files handle by the publisher for a 1:n connection
   a higher number will increase data throughput, but will also increase the size of used memory, number of semaphores
   and number of memory file observer threads on subscription side, default = 1, double buffering = 2
   higher values than 3 are not recommended
   values > 1 will break local IPC compatibility to eCAL 5.9 and older
*/
constexpr unsigned int PUB_MEMFILE_BUF_COUNT              = 1;

/* allow subscriber to access memory file without copying content in advance (zero copy)
   this memory file is blocked for other readers wihle processed by the user callback function
   this option is fully IPC compatible to all eCAL 5.x versions
*/
constexpr unsigned int PUB_MEMFILE_ZERO_COPY              = 0;

/**********************************************************************************************/
/*                                     service settings                                       */
/**********************************************************************************************/
/* support service protocol v0, eCAL 5.11 and older (0 = off, 1 = on) */
constexpr unsigned int SERVICE_PROTOCOL_V0                = 1;

/* support service protocol v1, eCAL 5.12 and newer (0 = off, 1 = on) */
constexpr unsigned int SERVICE_PROTOCOL_V1                = 1;

/**********************************************************************************************/
/*                                     time settings                                          */
/**********************************************************************************************/
constexpr char TIME_SYNC_MOD_RT[]                         = "";
constexpr char TIME_SYNC_MOD_REPLAY[]                     = "";

/**********************************************************************************************/
/*                                     process settings                                       */
/**********************************************************************************************/
constexpr char PROCESS_TERMINAL_EMULATOR[]                = "";

/**********************************************************************************************/
/*                                     ecal internal timings                                  */
/**********************************************************************************************/
/* timeout for automatic removing registered topics and memory files in global database in ms */
constexpr unsigned int CMN_REGISTRATION_TO                  = (60*1000);

/* time for resend registration info from publisher/subscriber in ms */
constexpr unsigned int CMN_REGISTRATION_REFRESH             = 1000;

/* delta time to check timeout for data readers in ms */
constexpr unsigned int CMN_DATAREADER_TIMEOUT_RESOLUTION_MS = 100;

/* cylce time udp receive threads in ms */
constexpr unsigned int CMN_UDP_RECEIVE_THREAD_CYCLE_TIME_MS = 1000;

/**********************************************************************************************/
/*                                     events                                                 */
/**********************************************************************************************/
/* common stop event prefix to shut down a local user process */
constexpr char EVENT_SHUTDOWN_PROC[]                        = "ecal_shutdown_process";

/**********************************************************************************************/
/*                                     experimental                                           */
/**********************************************************************************************/
/* enable distribution of monitoring/registration information via shared memory */
constexpr bool EXP_SHM_MONITORING_ENABLED                   = false;
/* disable distribution of monitoring/registration information via network (default) */
constexpr bool EXP_NETWORK_MONITORING_DISABLED              = false;
/* queue size of monitoring/registration events  */
constexpr unsigned int EXP_SHM_MONITORING_QUEUE_SIZE        = 1024;
/* domain name for shared memory based monitoring/registration */
constexpr char EXP_SHM_MONITORING_DOMAIN[]                  = "ecal_monitoring";
/* memory file access timeout */
constexpr unsigned int EXP_MEMFILE_ACCESS_TIMEOUT           = 100;

/* enable dropping of payload messages that arrive out of order */
constexpr bool EXP_DROP_OUT_OF_ORDER_MESSAGES               = false;
