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

#include <ecal/ecal_os.h>

/**********************************************************************************************/
/*                                     config settings                                        */
/**********************************************************************************************/
/* base data path name */
#define ECAL_HOME_PATH_WINDOWS                     ""
#define ECAL_HOME_PATH_LINUX                       ".ecal"
#define ECAL_LOG_PATH                              "logs"
#define ECAL_SETTINGS_PATH                         "cfg"

/* ini file name */
#define ECAL_DEFAULT_CFG                           "ecal.ini"

/**********************************************************************************************/
/*                                     monitor settings                                       */
/**********************************************************************************************/
/* timeout for automatic removing monitoring topics in ms */
#define MON_TIMEOUT                                 5000
/* topics blacklist as regular expression (will not be monitored) */
#define MON_FILTER_EXCL                            "_.*"
/* topics whitelist as regular expression (will be monitored only) */
#define MON_FILTER_INCL                            ""

/* logging filter settings */
#define MON_LOG_FILTER_CON                         "info,warning,error,fatal"
#define MON_LOG_FILTER_FILE                        ""
#define MON_LOG_FILTER_UDP                         "info,warning,error,fatal"

/**********************************************************************************************/
/*                                     network settings                                       */
/**********************************************************************************************/
/* network switch */
#ifdef CFG_FORCE_DEFAULT_LOCAL
#define NET_ENABLED                                 false
#else /* CFG_FORCE_DEFAULT_LOCAL */
#define NET_ENABLED                                 true
#endif /* CFG_FORCE_DEFAULT_LOCAL */

/* eCAL udp multicast defines */
#define NET_UDP_MULTICAST_GROUP                    "239.0.0.1"
#define NET_UDP_MULTICAST_MASK                     "0.0.0.15"
#define NET_UDP_MULTICAST_PORT                     14000
#ifdef CFG_FORCE_DEFAULT_LOCAL
#define NET_UDP_MULTICAST_TTL                          0
#else /* CFG_FORCE_DEFAULT_LOCAL */
#define NET_UDP_MULTICAST_TTL                          3
#endif /* CFG_FORCE_DEFAULT_LOCAL */
#define NET_UDP_MULTICAST_PORT_REG_OFF                 0
#define NET_UDP_MULTICAST_PORT_LOG_OFF                 1
#define NET_UDP_MULTICAST_PORT_SAMPLE_OFF              2
#define NET_UDP_MULTICAST_SNDBUF            (5*1024*1024)  /* 5 MByte */
#define NET_UDP_MULTICAST_RCVBUF            (5*1024*1024)  /* 5 MByte */

#define NET_UDP_RECBUFFER_TIMEOUT                   1000   /* ms */
#define NET_UDP_RECBUFFER_CLEANUP                     10   /* ms */

/* overall udp multicast bandwidth limitation in bytes/s, -1 == no limitation*/
#define NET_BANDWIDTH_MAX_UDP                         -1

#define NET_INPROC_REC_ENABLED                      true
#define NET_TCP_REC_ENABLED                         true
#define NET_SHM_REC_ENABLED                         true

#ifdef CFG_FORCE_DEFAULT_LOCAL
#define NET_UDP_MC_REC_ENABLED                      false
#else /* CFG_FORCE_DEFAULT_LOCAL */
#define NET_UDP_MC_REC_ENABLED                      true
#endif /* CFG_FORCE_DEFAULT_LOCAL */

#define NET_NPCAP_ENABLED                           false

#define NET_TCP_PUBSUB_NUM_EXECUTOR_READER          4
#define NET_TCP_PUBSUB_NUM_EXECUTOR_WRITER          4
#define NET_TCP_PUBSUB_MAX_RECONNECTIONS            5

/**********************************************************************************************/
/*                                     iceoryx settings                                       */
/**********************************************************************************************/
#define ICEORYX_SERVICE                            "eCAL"
#define ICEORYX_INSTANCE                           ""

/**********************************************************************************************/
/*                                     publisher settings                                     */
/**********************************************************************************************/
/* use inproc transport layer        [auto = 2, on = 1, off = 0] */
#define PUB_USE_INPROC                                 0
/* use shared memory transport layer [auto = 2, on = 1, off = 0] */
#define PUB_USE_SHM                                    2
/* use tcp transport layer           [auto = 2, on = 1, off = 0] */
#define PUB_USE_TCP                                    0
/* use udp multicast transport layer [auto = 2, on = 1, off = 0] */
#ifdef CFG_FORCE_DEFAULT_LOCAL
#define PUB_USE_UDP_MC                                 0
#else /* CFG_FORCE_DEFAULT_LOCAL */
#define PUB_USE_UDP_MC                                 2
#endif /* CFG_FORCE_DEFAULT_LOCAL */

/* share topic type                  [          on = 1, off = 0] */
#define PUB_SHARE_TTYPE                                1
/* share topic description           [          on = 1, off = 0] */
#define PUB_SHARE_TDESC                                1

/* minimum size for created shared memory files */
#define PUB_MEMFILE_MINSIZE                      (4*1024)
/* reserve buffer size before reallocation in % */
#define PUB_MEMFILE_RESERVE                           50

/* timeout for create / open a memory file using mutex lock in ms */
#define PUB_MEMFILE_CREATE_TO                        200
#define PUB_MEMFILE_OPEN_TO                           50

/* timeout for memory read acknowledge signal from data reader in ms */
#define PUB_MEMFILE_ACK_TO                            0   /* ms */

/* defines number of memory files handle by the publisher for a 1:n connection
   a higher number will increase data throughput, but will also increase the size of used memory, number of semaphores
   and number of memory file observer threads on subscription side, default = 1, double buffering = 2
   higher values than 3 are not recommended
   values > 1 will break local IPC compatibility to eCAL 5.9 and older
*/
#define PUB_MEMFILE_BUF_COUNT                         1

/* allow subscriber to access memory file without copying content in advance (zero copy)
   this memory file is blocked for other readers wihle processed by the user callback function
   this option is fully IPC compatible to all eCAL 5.x versions
*/
#define PUB_MEMFILE_ZERO_COPY                         0

/**********************************************************************************************/
/*                                     time settings                                          */
/**********************************************************************************************/
#define TIME_SYNC_MOD_RT                              ""
#define TIME_SYNC_MOD_REPLAY                          ""

/**********************************************************************************************/
/*                                     process settings                                       */
/**********************************************************************************************/
#define PROCESS_TERMINAL_EMULATOR                     ""

/**********************************************************************************************/
/*                                     ecal internal timings                                  */
/**********************************************************************************************/
/* timeout for automatic removing registered topics and memory files in global database in ms */
#define CMN_REGISTRATION_TO                     (60*1000)

/* time for resend registration info from publisher/subscriber in ms */
#define CMN_REGISTRATION_REFRESH                    1000

/* delta time to check timeout for data readers in ms */
#define CMN_DATAREADER_TIMEOUT_DTIME                  10

/**********************************************************************************************/
/*                                     events                                                 */
/**********************************************************************************************/
/* common stop event prefix to shut down a local user process */
#define EVENT_SHUTDOWN_PROC             "ecal_shutdown_process"
