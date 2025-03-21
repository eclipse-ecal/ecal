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
  * @file   config/time.h
  * @brief  eCAL time configuration
 **/

#ifndef ecal_c_config_time_h_included
#define ecal_c_config_time_h_included

struct eCAL_Time_Configuration
{
  const char* timesync_module_rt;     /*!< Time synchronisation interface name (dynamic library)
                                     The name will be extended with platform suffix (32|64), debug suffix (d) and platform extension (.dll|.so)
                                     Available modules are:
                                       - ecaltime-localtime    local system time without synchronization
                                       - ecaltime-linuxptp     For PTP / gPTP synchronization over ethernet on Linux
                                                               (device configuration in ecaltime.ini)
                                     (Default: ecaltime-localtime)*/
  const char* timesync_module_replay; //!< (Default: "")
};

#endif /* ecal_c_config_time_h_included */