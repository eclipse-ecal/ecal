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
 * @file   ecal_registration.h
 * @brief  eCAL registration interface
**/

#pragma once

#include <ecal/ecal_os.h>
#include <ecal/ecal_types.h>

#include <cstdint>
#include <functional>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace eCAL
{
  namespace Registration
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
      SDataTypeInformation info;
      DescQualityFlags     quality = DescQualityFlags::NO_QUALITY;

      bool operator<(const SQualityTopicInfo& other) const
      {
        return std::tie(quality, info) < std::tie(other.quality, info);
      }
    };
    using QualityTopicInfoMultiMap = std::multimap<std::string, SQualityTopicInfo>;
    using QualityTopicInfoSet      = std::set<SQualityTopicInfo>;

    using ServiceId = std::uint64_t;
    struct SQualityServiceInfo
    {
      SServiceMethodInformation info;
      DescQualityFlags          request_quality  = DescQualityFlags::NO_QUALITY;
      DescQualityFlags          response_quality = DescQualityFlags::NO_QUALITY;

      bool operator<(const SQualityServiceInfo& other) const
      {
        return std::tie(request_quality, response_quality) < std::tie(other.request_quality, other.response_quality);
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

    using CallbackToken = std::size_t;

    enum class RegistrationEventType
    {
      new_entity,     //!< Represents a new entity registration
      deleted_entity  //!< Represents a deletion of an entity
    };

    using TopicIDCallbackT   = std::function<void(const STopicId&,   RegistrationEventType)>;
    using ServiceIDCallbackT = std::function<void(const SServiceId&, RegistrationEventType)>;

    /**
     * @brief Get complete snapshot of all known publisher.
     *
     * @return Set of topic id's.
    **/
    ECAL_API std::set<STopicId> GetPublisherIDs();

    /**
     * @brief Get data type information with quality for specific publisher.
     *
     * @return True if information could be queried.
    **/
    ECAL_API bool GetPublisherInfo(const STopicId& id_, SQualityTopicInfo& topic_info_);

    /**
     * @brief Register a callback function to be notified when a new publisher becomes available.
     *
     * @param callback_       The callback function to be called with the STopicId of the new publisher.
     *                        The callback function must not be blocked for a longer period of time, 
     *                        otherwise timeout mechanisms of the eCAL registration would be triggered.
     *
     * @return CallbackToken  Token that can be used to unregister the callback.
     */
    ECAL_API CallbackToken AddPublisherEventCallback(const TopicIDCallbackT& callback_);

    /**
     * @brief Unregister the publisher callback using the provided token.
     *
     * @param token  The token returned by AddPublisherCallback.
    */
    ECAL_API void RemPublisherEventCallback(CallbackToken token_);

    /**
     * @brief Get complete snapshot of all known subscriber.
     *
     * @return Set of topic id's.
    **/
    ECAL_API std::set<STopicId> GetSubscriberIDs();

    /**
     * @brief Get data type information with quality for specific subscriber.
     *
     * @return True if information could be queried.
    **/
    ECAL_API bool GetSubscriberInfo(const STopicId& id_, SQualityTopicInfo& topic_info_);

    /**
     * @brief Register a callback function to be notified when a new subscriber becomes available.
     *
     * @param callback_       The callback function to be called with the STopicId of the new subscriber.
     *                        The callback function must not be blocked for a longer period of time, 
     *                        otherwise timeout mechanisms of the eCAL registration would be triggered.
     *
     * @return CallbackToken  Token that can be used to unregister the callback.
     */
    ECAL_API CallbackToken AddSubscriberEventCallback(const TopicIDCallbackT& callback_);

    /**
     * @brief Unregister the subscriber callback using the provided token.
     *
     * @param token  The token returned by AddSubscriberCallback.
    */
    ECAL_API void RemSubscriberEventCallback(CallbackToken token_);

    /**
     * @brief Get complete snapshot of all known services.
     *
     * @return Set of service id's.
    **/
    ECAL_API std::set<SServiceId> GetServiceIDs();

    /**
     * @brief Get service method information with quality for specific service.
     *
     * @return True if information could be queried.
    **/
    ECAL_API bool GetServiceInfo(const SServiceId& id_, SQualityServiceInfo& service_info_);

    /**
     * @brief Get complete snapshot of all known clients.
     *
     * @return Set of service id's.
    **/
    ECAL_API std::set<SServiceId> GetClientIDs();

    /**
     * @brief Get service method information with quality for specific client.
     *
     * @return True if information could be queried.
    **/
    ECAL_API bool GetClientInfo(const SServiceId& id_, SQualityServiceInfo& service_info_);

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
  }
}
