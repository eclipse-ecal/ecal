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
 * @file   ecal_util.h
 * @brief  eCAL utility interface
**/

#pragma once

#include <ecal/ecal_os.h>

#include <map>
#include <unordered_map>
#include <string>
#include <vector>

namespace eCAL
{
  namespace Util
  {
    /**
     * @brief Retrieve eCAL home path (for starting eCAL applications).
     *          Windows: $ECAL_HOME/eCAL
     *          Linux:   $HOME/.ecal
     *
     * @return  eCAL home path.
    **/
    [[deprecated]]
    ECAL_API std::string GeteCALHomePath();

    /**
     * @brief Retrieve eCAL configuration path.
     *          This is path is for the global eCAL configuration files
     *          like ecal.ini.
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
     * @brief Retrieve full eCAL default ini file name.
     *
     * @deprecated  Use GeteCALActiveIniFile() instead
     *
     * @return  eCAL default ini file name.
    **/
    [[deprecated]]
    ECAL_API std::string GeteCALDefaultIniFile();

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

    struct STopicInfo
    {
      std::string type_name;        //!< Type name of the current topic
      std::string type_description; //!< Descriptor string of the current topic.
    };
    /**
     * @brief Get complete topic map (including types and descriptions).
     *
     * @param topic_info_map_  Map to store the topic informations.
     *                         Map containing { TopicName -> (Type, Description) } mapping of all topics that are currently known.
    **/
    ECAL_API void GetTopics(std::unordered_map<std::string, STopicInfo>& topic_info_map_);

    /**
     * @brief Get all topic names.
     *
     * @param topic_names_ Vector to store the topic names.
    **/
    ECAL_API void GetTopicNames(std::vector<std::string>& topic_names_);

    /**
     * @brief Gets type name of the specified topic.
     *
     * @param topic_name_   Topic name.
     * @param topic_type_   String to store type name.
     *
     * @return  True if succeeded.
    **/
    ECAL_API bool GetTopicTypeName(const std::string& topic_name_, std::string& topic_type_);

    /**
     * @brief Gets type name of the specified topic.
     *
     * @param topic_name_   Topic name.
     *
     * @return  Topic type name.
    **/
    ECAL_API std::string GetTopicTypeName(const std::string& topic_name_);

    /**
     * @brief Gets description of the specified topic.
     *
     * @param topic_name_   Topic name.
     * @param topic_desc_   String to store description.
     *
     * @return  True if succeeded.
    **/
    ECAL_API bool GetTopicDescription(const std::string& topic_name_, std::string& topic_desc_);

    /**
     * @brief Gets description of the specified topic.
     *
     * @param topic_name_   Topic name.
     *
     * @return  Topic description.
    **/
    ECAL_API std::string GetTopicDescription(const std::string& topic_name_);

    struct SServiceMethodInfo
    {
      std::string request_type_name;         //!< Type name of the request message
      std::string request_type_description;  //!< Descriptor string of the request description
      std::string response_type_name;        //!< Type name of the response message
      std::string response_type_description; //!< Descriptor string of the response message
    };
    /**
     * @brief Get complete service map (including request and response types and descriptions).
     *
     * @param service_info_map_  Map to store the topic informations.
     *                           Map { (ServiceName, MethodName) -> ( (ReqType, ReqDescription), (RespType, RespDescription) ) } mapping of all currently known services.
    **/
    ECAL_API void GetServices(std::map<std::tuple<std::string, std::string>, Util::SServiceMethodInfo>& service_info_map_);

    /**
     * @brief Get all service/method names.
     *
     * @param service_method_names_ Vector to store the service/method tuples (Vector { (ServiceName, MethodName) }).
    **/
    ECAL_API void GetServiceNames(std::vector<std::tuple<std::string, std::string>>& service_method_names_);

    /**
     * @brief Gets service method request and response type names.
     *
     * @param service_name_  Service name.
     * @param method_name_   Method name.
     * @param req_type_      String to store request type.
     * @param resp_type_     String to store response type.
     *
     * @return  True if succeeded.
    **/
    ECAL_API bool GetServiceTypeNames(const std::string& service_name_, const std::string& method_name_, std::string& req_type_, std::string& resp_type_);

    /**
     * @brief Gets service method request and response descriptions.
     *
     * @param service_name_  Service name.
     * @param method_name_   Method name.
     * @param req_desc_      String to store request description.
     * @param resp_desc_     String to store response description.
     *
     * @return  True if succeeded.
    **/
    ECAL_API bool GetServiceDescription(const std::string& service_name_, const std::string& method_name_, std::string& req_desc_, std::string& resp_desc_);

    /**
     * @brief Gets type name of the specified topic.
     * 
     * This function is deprecated with eCAL 5.10. Please use GetTopicTypeName.
     *
     * @param topic_name_   Topic name.
     * @param topic_type_   String to store type name.
     *
     * @return  True if succeeded.
    **/
    [[deprecated]]
    ECAL_API bool GetTypeName(const std::string& topic_name_, std::string& topic_type_);

    /**
     * @brief Gets type name of the specified topic.
     *
     * This function is deprecated with eCAL 5.10. Please use GetTopicTypeName.
     *
     * @param topic_name_   Topic name.
     *
     * @return  Topic type name.
    **/
    [[deprecated]]
    ECAL_API std::string GetTypeName(const std::string& topic_name_);

    /**
     * @brief Gets description of the specified topic.
     *
     * This function is deprecated with eCAL 5.10. Please use GetTopicDescription.
     *
     * @param topic_name_   Topic name.
     * @param topic_desc_   String to store description.
     *
     * @return  True if succeeded.
    **/
    [[deprecated]]
    ECAL_API bool GetDescription(const std::string& topic_name_, std::string& topic_desc_);

    /**
     * @brief Gets description of the specified topic.
     *
     * This function is deprecated with eCAL 5.10. Please use GetTopicDescription.
     *
     * @param topic_name_   Topic name.
     *
     * @return  Topic description.
    **/
    [[deprecated]]
    ECAL_API std::string GetDescription(const std::string& topic_name_);
  }
}
