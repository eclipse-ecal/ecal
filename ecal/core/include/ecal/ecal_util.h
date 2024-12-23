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
 * @file   ecal_util.h
 * @brief  eCAL utility interface
**/

#pragma once

#include <ecal/ecal_os.h>
#include <ecal/ecal_deprecate.h>

#include <string>
#include <utility>

namespace eCAL
{
  namespace Util
  {
    /**
     * @brief Retrieve eCAL configuration path.
     *
     *        Checks for a valid default configuration file (ecal.yaml) in the following paths:
     *        1. ECAL_CONFIG_DIR environment variable path
     *        2. Local user path (win: Appdata/Local, unix: ~/.ecal)
     *        3. System paths like /etc/ecal, ProgramData/eCAL
     *
     * @return  First path that contains a valid config file.
     *          Returns empty string if no valid config file is found.
    **/
    ECAL_API std::string GeteCALConfigDir(); // ECAL_DATA PATH?

    /**
     * @brief Retrieve eCAL standard logging path.
     *          This is path is for the eCAL logging files.
     *          This path has read/write permissions for standard users.
     * 
     *        1. ECAL_LOG_DIR environment variable path
     *        2. eCAL configuration path
     *        3. eCAL default configuration path
     *        4. System temporary directory
     *        5. Fallback path /ecal_tmp
     *
     * @return  eCAL logging path.
    **/
    ECAL_API std::string GeteCALLogDir();

    /**
     * @brief Send shutdown event to specified local user process using it's unit name.
     *
     * @param unit_name_   Process unit name.
    **/
    ECAL_API void ShutdownProcess(const std::string& unit_name_);

    /**
     * @brief Send shutdown event to specified local user process using it's process id.
     *
     * @param process_id_   Process id.
    **/
    ECAL_API void ShutdownProcess(int process_id_);

    /**
     * @brief Send shutdown event to all local user processes.
    **/
    ECAL_API void ShutdownProcesses();

    /**
     * @brief Splits the topic type (eCAL < 5.12) into encoding and types (>= eCAL 5.12)
     *
     * @param combined_topic_type_  "Old" typename.
     *
     * @return  std::pair(encoding, typename).
    **/
    ECAL_DEPRECATE_SINCE_6_0("Please refactor your code to use SDatatypeInformation. This function will be removed with eCAL 7")
    ECAL_API std::pair<std::string, std::string> SplitCombinedTopicType(const std::string& combined_topic_type_);

    /**
     * @brief Combine separate encoding and type iinformation (>= eCAL 5.12) into a combined typename (eCAL < 5.12) 
     *
     * @param topic_encoding_ Topic Encoding
     * @param topic_type_     Topic Type
     *
     * @return "Old" typename. ( encoding:typename ).
    **/
    ECAL_DEPRECATE_SINCE_6_0("Please refactor your code to use SDatatypeInformation. This function will be removed with eCAL 7")
    ECAL_API std::string CombinedTopicEncodingAndType(const std::string& topic_encoding_, const std::string& topic_type_);
  }
}
