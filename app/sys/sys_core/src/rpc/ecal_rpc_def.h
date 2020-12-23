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
 * @brief  eCAL rpc global definition file
**/

#pragma once

/**********************************************************************************************/
/*                                     config settings                                        */
/**********************************************************************************************/
/* base data path name */
#define ECAL_BIN_PATH_WINDOWS           "bin"

/**********************************************************************************************/
/*                                     events                                                 */
/**********************************************************************************************/
/* recorder start event for starting local recording (not the actual application) */
#define EVENT_RECORDER_START            "ecal_recorder_start"

/* recorder close event for closing local recording application */
#define EVENT_RECORDER_CLOSE            "ecal_recorder_close"

/* recorder start event for starting local CAN recording (not the actual application) */
#define EVENT_RECORDER_CAN_START        "ecal_recorder_can_start"

/* recorder close event for closing local CAN recorder application */
#define EVENT_RECORDER_CAN_CLOSE        "ecal_recorder_can_close"
