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
constexpr unsigned int NET_UDP_MULTICAST_PORT_REG_OFF                       = 0U; // to delete
constexpr unsigned int NET_UDP_MULTICAST_PORT_LOG_OFF                       = 1U; // to delete
constexpr unsigned int NET_UDP_MULTICAST_PORT_SAMPLE_OFF                    = 2U; // to delete

/* timeout for create / open a memory file using mutex lock in ms */
constexpr unsigned int PUB_MEMFILE_CREATE_TO              = 200U;
constexpr unsigned int PUB_MEMFILE_OPEN_TO                = 200U;
/* memory file access timeout */
constexpr unsigned int EXP_MEMFILE_ACCESS_TIMEOUT           = 100U;


/**********************************************************************************************/
/*                                     events                                                 */
/**********************************************************************************************/
/* common stop event prefix to shut down a local user process */
constexpr const char* EVENT_SHUTDOWN_PROC                   = "ecal_shutdown_process";

