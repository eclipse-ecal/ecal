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

#include <string>
#include <utility>

namespace eCAL
{
  namespace Util
  {
    /**
     * @brief Retrieve eCAL configuration path.
     *          This is path is for the global eCAL configuration files
     *          like ecal.yaml.
     *          This path is read only for standard users.
     *
     * @return  eCAL configuration path.
    **/
    ECAL_API std::string GeteCALConfigPath();

    /**
     * @brief Retrieve eCAL user configuration path.
     *          This is path is for the eCAL application settings files.
     *          This path has read/write permissions for standard users.
     *
     * @return  eCAL data path.
    **/
    ECAL_API std::string GeteCALUserSettingsPath();

    /**
     * @brief Retrieve eCAL standard logging path.
     *          This is path is for the eCAL logging files.
     *          This path has read/write permissions for standard users.
     *
     * @return  eCAL data path.
    **/
    ECAL_API std::string GeteCALLogPath();

    /**
     * @brief Retrieve full path to active eCAL ini file.
     *
     * @return  eCAL active ini file name.
    **/
    ECAL_API std::string GeteCALActiveIniFile();

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
     * @brief Send shutdown event to all local core components.
    **/
    ECAL_API void ShutdownCore();

    /**
     * @brief Enable eCAL message loop back,
     *          that means subscriber will receive messages from
     *          publishers of the same process (default == false).
     *
     * @param state_  Switch on message loop back. 
    **/
    ECAL_API void EnableLoopback(bool state_);

    /**
     * @brief Enable process wide eCAL publisher topic type sharing
     *          that is needed for reflection on subscriber side.
     *
     * @param state_  Switch on type sharing
    **/
    ECAL_API void PubShareType(bool state_);

    /**
     * @brief Enable process wide eCAL publisher topic description sharing
     *          that is needed for reflection on subscriber side.
     *
     * @param state_  Switch on description sharing
    **/
    ECAL_API void PubShareDescription(bool state_);

    /**
     * @brief Splits the topic type (eCAL < 5.12) into encoding and types (>= eCAL 5.12)
     *
     * @param combined_topic_type_  "Old" typename.
     *
     * @return  std::pair(encoding, typename).
    **/
    ECAL_API std::pair<std::string, std::string> SplitCombinedTopicType(const std::string& combined_topic_type_);

    /**
     * @brief Combine separate encoding and type iinformation (>= eCAL 5.12) into a combined typename (eCAL < 5.12) 
     *
     * @param topic_encoding_ Topic Encoding
     * @param topic_type_     Topic Type
     *
     * @return "Old" typename. ( encoding:typename ).
    **/
    ECAL_API std::string CombinedTopicEncodingAndType(const std::string& topic_encoding_, const std::string& topic_type_);
  }
}
