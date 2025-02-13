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
 * @file   util.h
 * @brief  eCAL utility interface
**/

#pragma once

#include <ecal/os.h>
#include <ecal/deprecate.h>

#include <string>
#include <utility>

namespace eCAL
{
  namespace Util
  {
    /**
     * @brief Retrieve eCAL data path.
     *
     *        Checks if a valid eCAL data path is available in the following order:
     *        1. ECAL_DATA environment variable path
     *        2. Local user path (win: Appdata/Local, unix: ~/.ecal)
     *        3. System paths like /etc/ecal, ProgramData/eCAL
     *
     * @return  First directory that is not empty.
     *          Returns empty string if no valid directory is found.
    **/
    ECAL_API std::string GeteCALDataDir();

    /**
     * @brief Returns the path to the eCAL log directory. Searches in following order:
     *
     *        1. Environment variable ECAL_LOG_DIR
     *        2. Environment variable ECAL_DATA (also checking for logs subdirectory)
     *        3. The path provided from the configuration
     *        4. The path where ecal.yaml was loaded from (also checking for logs subdirectory)
     *        5. The temporary directory (e.g. /tmp [unix], Appdata/local/Temp [win])
     *        6. Fallback path /ecal_tmp
     * 
     *        In case of 5/6, a unique temporary folder will be created.
     *        
     * @returns The path to the eCAL log directory. The subdirectory logs might not exist yet.
     *          Returns empty string if no root path could be found.
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

