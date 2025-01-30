/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
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
#include <ecal/log_level.h>
#include <ecal/os.h>

/**********************************************************************************************/
/*                                     config settings                                        */
/**********************************************************************************************/
/* base folder names */
constexpr const char* ECAL_FOLDER_NAME_WINDOWS      = "eCAL";
constexpr const char* ECAL_FOLDER_NAME_LINUX        = "ecal";
constexpr const char* ECAL_FOLDER_NAME_HOME_LINUX   = ".ecal";
constexpr const char* ECAL_FOLDER_NAME_LOG          = "logs";
constexpr const char* ECAL_FOLDER_NAME_TMP_WINDOWS  = "Temp";

/* ini file name */
constexpr const char* ECAL_DEFAULT_CFG              = "ecal.yaml";

/* environment variables */
constexpr const char* ECAL_DATA_VAR                 = "ECAL_DATA";
constexpr const char* ECAL_LOG_VAR                  = "ECAL_LOG_DIR";
constexpr const char* ECAL_LINUX_HOME_VAR           = "HOME";
constexpr const char* ECAL_LINUX_TMP_VAR            = "TMPDIR";

/* system paths */
constexpr const char* ECAL_LINUX_SYSTEM_PATH        = "/etc";
constexpr const char* ECAL_LINUX_TMP_DIR            = "/var/tmp";
constexpr const char* ECAL_FALLBACK_TMP_DIR         = "/ecal_tmp";

/* back*/

/* eCAL udp multicast defines */
constexpr unsigned int NET_UDP_MULTICAST_PORT_REG_OFF     = 0U; // to delete
constexpr unsigned int NET_UDP_MULTICAST_PORT_LOG_OFF     = 1U; // to delete
constexpr unsigned int NET_UDP_MULTICAST_PORT_SAMPLE_OFF  = 2U; // to delete

/* timeout for create / open a memory file using mutex lock in ms */
constexpr unsigned int PUB_MEMFILE_CREATE_TO              = 200U;
constexpr unsigned int PUB_MEMFILE_OPEN_TO                = 200U;
/* memory file access timeout */
constexpr unsigned int EXP_MEMFILE_ACCESS_TIMEOUT         = 100U;


/**********************************************************************************************/
/*                                     events                                                 */
/**********************************************************************************************/
/* common stop event prefix to shut down a local user process */
constexpr const char* EVENT_SHUTDOWN_PROC                 = "ecal_shutdown_process";

