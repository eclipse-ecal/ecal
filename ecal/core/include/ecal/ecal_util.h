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
#include <ecal/ecal_deprecate.h>
#include <ecal/ecal_types.h>

#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <tuple>
#include <utility>

namespace eCAL
{
    namespace Util
    {
    // enumeration of quality bits used for detecting how good a data information is
    enum class DescQualityFlags : std::uint8_t
    {
      NO_QUALITY                = 0,         //!< Special value for initialization
      DESCRIPTION_AVAILABLE     = 0x1 << 3,  //!< Having a type descriptor available
      ENCODING_AVAILABLE        = 0x1 << 2,  //!< Having a type encoding
      TYPENAME_AVAILABLE        = 0x1 << 1,  //!< Having a type name available
      INFO_COMES_FROM_PRODUCER  = 0x1 << 0   //!< Info is coming from the producer (like a publisher, service)
    };

    constexpr inline DescQualityFlags  operator~  (DescQualityFlags  a)                     { return static_cast<DescQualityFlags>( ~static_cast<std::underlying_type<DescQualityFlags>::type>(a) ); }
    constexpr inline DescQualityFlags  operator|  (DescQualityFlags  a, DescQualityFlags b) { return static_cast<DescQualityFlags>( static_cast<std::underlying_type<DescQualityFlags>::type>(a) | static_cast<std::underlying_type<DescQualityFlags>::type>(b) ); }
    constexpr inline DescQualityFlags  operator&  (DescQualityFlags  a, DescQualityFlags b) { return static_cast<DescQualityFlags>( static_cast<std::underlying_type<DescQualityFlags>::type>(a) & static_cast<std::underlying_type<DescQualityFlags>::type>(b) ); }
    constexpr inline DescQualityFlags  operator^  (DescQualityFlags  a, DescQualityFlags b) { return static_cast<DescQualityFlags>( static_cast<std::underlying_type<DescQualityFlags>::type>(a) ^ static_cast<std::underlying_type<DescQualityFlags>::type>(b) ); }
    inline           DescQualityFlags& operator|= (DescQualityFlags& a, DescQualityFlags b) { return reinterpret_cast<DescQualityFlags&>( reinterpret_cast<std::underlying_type<DescQualityFlags>::type&>(a) |= static_cast<std::underlying_type<DescQualityFlags>::type>(b) ); }
    inline           DescQualityFlags& operator&= (DescQualityFlags& a, DescQualityFlags b) { return reinterpret_cast<DescQualityFlags&>( reinterpret_cast<std::underlying_type<DescQualityFlags>::type&>(a) &= static_cast<std::underlying_type<DescQualityFlags>::type>(b) ); }
    inline           DescQualityFlags& operator^= (DescQualityFlags& a, DescQualityFlags b) { return reinterpret_cast<DescQualityFlags&>( reinterpret_cast<std::underlying_type<DescQualityFlags>::type&>(a) ^= static_cast<std::underlying_type<DescQualityFlags>::type>(b) ); }

    using TopicId = std::uint64_t;
    struct SQualityTopicInfo
    {
      TopicId              id = 0;
      SDataTypeInformation info;
      DescQualityFlags     quality = DescQualityFlags::NO_QUALITY;

      bool operator<(const SQualityTopicInfo& other) const
      {
        return std::tie(quality, id) < std::tie(other.quality, other.id);
      }
    };
    using QualityTopicInfoMultiMap = std::multimap<std::string, SQualityTopicInfo>;
    using QualityTopicInfoSet      = std::set<SQualityTopicInfo>;

    using ServiceId = std::uint64_t;
    struct SQualityServiceInfo
    {
      ServiceId                 id = 0;
      SServiceMethodInformation info;
      DescQualityFlags          request_quality  = DescQualityFlags::NO_QUALITY;
      DescQualityFlags          response_quality = DescQualityFlags::NO_QUALITY;

      bool operator<(const SQualityServiceInfo& other) const
      {
        return std::tie(request_quality, response_quality, id) < std::tie(other.request_quality, other.response_quality, other.id);
      }
    };
    struct SServiceMethod
    {
      std::string service_name;
      std::string method_name;

      bool operator<(const SServiceMethod& other) const
      {
        return std::tie(service_name, method_name) < std::tie(other.service_name, other.method_name);
      }
    };
    using QualityServiceInfoMultimap = std::multimap<SServiceMethod, SQualityServiceInfo>;
    using SQualityServiceInfoSet     = std::set<SQualityServiceInfo>;
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
     * @brief Get complete snapshot of data type information with quality and topic id for all known publisher.
     *
     * @return MultiMap containing the quality datatype information and the topic id's.
    **/
    ECAL_API QualityTopicInfoMultiMap GetPublishers();

    /**
     * @brief Get data type information with quality and topic id for this publishers.
     *
     * @param topic_name_  Topic name.
     *
     * @return Set containing the quality datatype information for this publisher.
    **/
    ECAL_API QualityTopicInfoSet GetPublishers(const std::string& topic_name_);

    /**
     * @brief Get complete snapshot of data type information with quality and topic id for all known subscribers.
     *
     * @return MultiMap containing the quality datatype information and the topic id's.
    **/
    ECAL_API QualityTopicInfoMultiMap GetSubscribers();

    /**
     * @brief Get data type information with quality and topic id for this subscriber.
     *
     * @param topic_name_  Topic name.
     *
     * @return Set containing the quality datatype information for this subscriber.
    **/
    ECAL_API QualityTopicInfoSet GetSubscribers(const std::string& topic_name_);

    /**
     * @brief Get highest quality data type information out of a set of quality data type information.
     * 
     * @param quality_topic_info_set_  Set of quality data type information
     *
     * @return Highest quality data type information.
    **/
    ECAL_API SDataTypeInformation GetHighestQualityDataTypeInformation(const QualityTopicInfoSet& quality_topic_info_set_);

    /**
     * @brief Get complete snapshot of service method information with quality and service id for all known services.
     *
     * @return MultiMap<ServiceName, MethodName> containing the quality datatype information and the service id's.
    **/
    ECAL_API QualityServiceInfoMultimap GetServices();

    /**
     * @brief Get complete snapshot of service method information with quality and client id for all known clients.
     *
     * @return MultiMap<ClientName, MethodName> containing the quality datatype information and the client id's.
    **/
    ECAL_API QualityServiceInfoMultimap GetClients();

    /**
     * @brief Get highest quality service method type information out of a set of quality service method information.
     *
     * @param quality_service_info_set_  Set of quality service method information
     *
     * @return Highest quality service method information.
    **/
    ECAL_API SServiceMethodInformation GetHighestQualityServiceMethodInformation(const SQualityServiceInfoSet& quality_service_info_set_);

    /**
     * @brief Get complete topic map.
     *
     * @param data_type_info_map_  Map to store the datatype information.
     *                             Map { TopicName -> SDataTypeInformation } mapping of all currently known publisher/subscriber.
    **/
    ECAL_API void GetTopics(std::map<std::string, SDataTypeInformation>& data_type_info_map_);

    /**
     * @brief Get complete quality topic map.
     *
     * @param quality_topic_info_map_  Map to store the quality datatype information.
     *                                 Map { TopicName -> SQualityDataTypeInformation } mapping of all currently known publisher/subscriber.
    **/
    ECAL_API void GetTopics(std::map<std::string, SQualityTopicInfo>& quality_topic_info_map_);

    /**
     * @brief Get all topic names.
     *
     * @param topic_names_ Set to store the topic names.
    **/
    ECAL_API void GetTopicNames(std::set<std::string>& topic_names_);

    /**
     * @brief Gets description of the specified topic.
     *
     * @param topic_name_       Topic name.
     * @param data_type_info_   SDataTypeInformation to be filled by this function.
     *
     * @return True if TopicInformation for specified topic could be retrieved, false otherwise.
    **/
    ECAL_API bool GetTopicDataTypeInformation(const std::string& topic_name_, SDataTypeInformation& data_type_info_);

    /**
     * @brief Get complete service map.
     *
     * @param service_method_info_map_  Map to store the service/method descriptions.
     *                                  Map { (ServiceName, MethodName) -> SServiceMethodInformation } mapping of all currently known services.
    **/
    ECAL_API void GetServices(std::map<SServiceMethod, SServiceMethodInformation>& service_method_info_map_);

    /**
     * @brief Get complete quality service map.
     *
     * @param quality_service_info_map_  Map to store the quality service/method descriptions.
     *                                     Map { (ServiceName, MethodName) -> SQualityServiceMethodInformation } mapping of all currently known services.
    **/
    ECAL_API void GetServices(std::map<SServiceMethod, SQualityServiceInfo>& quality_service_info_map_);

    /**
     * @brief Get all service/method names.
     *
     * @param service_method_names_ Set to store the service/method names (Set { (ServiceName, MethodName) }).
    **/
    ECAL_API void GetServiceMethodNames(std::set<SServiceMethod>& service_method_names_);

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
     * @brief Get complete client map.
     *
     * @param client_method_info_map_  Map to store the client/method descriptions.
     *                                 Map { (ClientName, MethodName) ->  SServiceMethodInformation } mapping of all currently known clients.
    **/
    ECAL_API void GetClients(std::map<SServiceMethod, SServiceMethodInformation>& client_method_info_map_);

    /**
     * @brief Get complete quality client map.
     *
     * @param quality_client_info_map_  Map to store the quality client/method descriptions.
     *                                  Map { (ClientName, MethodName) ->  SQualityServiceMethodInformation } mapping of all currently known clients.
    **/
    ECAL_API void GetClients(std::map<SServiceMethod, SQualityServiceInfo>& quality_client_info_map_);

    /**
     * @brief Get all client/method names.
     *
     * @param client_method_names_ Set to store the client/method names (Set { (ClientName, MethodName) }).
    **/
    ECAL_API void GetClientMethodNames(std::set<SServiceMethod>& client_method_names_);

    /**
     * @brief Gets client method request and response type names.
     *
     * @param client_name_  Client name.
     * @param method_name_  Method name.
     * @param req_type_     String to store request type.
     * @param resp_type_    String to store response type.
     *
     * @return  True if succeeded.
    **/
    ECAL_API bool GetClientTypeNames(const std::string& client_name_, const std::string& method_name_, std::string& req_type_, std::string& resp_type_);

    /**
     * @brief Gets client method request and response descriptions.
     *
     * @param client_name_  Client name.
     * @param method_name_  Method name.
     * @param req_desc_     String to store request description.
     * @param resp_desc_    String to store response description.
     *
     * @return  True if succeeded.
    **/
    ECAL_API bool GetClientDescription(const std::string& client_name_, const std::string& method_name_, std::string& req_desc_, std::string& resp_desc_);

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
