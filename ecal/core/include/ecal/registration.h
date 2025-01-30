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
 * @file   registration.h
 * @brief  eCAL registration interface
**/

#pragma once

#include <ecal/os.h>
#include <ecal/pubsub/types.h>
#include <ecal/service/types.h>

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
    struct SServiceMethod
    {
      std::string service_name;
      std::string method_name;

      bool operator<(const SServiceMethod& other) const
      {
        return std::tie(service_name, method_name) < std::tie(other.service_name, other.method_name);
      }
    };

    using CallbackToken = std::size_t;

    enum class RegistrationEventType
    {
      new_entity,     //!< Represents a new entity registration
      deleted_entity  //!< Represents a deletion of an entity
    };

    /* @brief Event callback, when a topic related entity (publisher / subscriber) has been created or deleted */
    using TopicEventCallbackT = std::function<void(const STopicId&, RegistrationEventType)>;

    /**
     * @brief Get complete snapshot of all known publisher.
     *
     * @return Set of topic id's.
    **/
    ECAL_API bool GetPublisherIDs(std::set<STopicId>& topic_ids_);

    /**
     * @brief Get data type information with quality for specific publisher.
     *
     * @return True if information could be queried.
    **/
    ECAL_API bool GetPublisherInfo(const STopicId& id_, SDataTypeInformation& topic_info_);

    /**
     * @brief Register a callback function to be notified when a new publisher becomes available.
     *
     * @param callback_       The callback function to be called with the STopicId of the new publisher.
     *                        The callback function must not be blocked for a longer period of time, 
     *                        otherwise timeout mechanisms of the eCAL registration would be triggered.
     *
     * @return CallbackToken  Token that can be used to unregister the callback.
     */
    ECAL_API CallbackToken AddPublisherEventCallback(const TopicEventCallbackT& callback_);

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
    ECAL_API bool GetSubscriberIDs(std::set<STopicId>& topic_ids_);

    /**
     * @brief Get data type information with quality for specific subscriber.
     *
     * @return True if information could be queried.
    **/
    ECAL_API bool GetSubscriberInfo(const STopicId& id_, SDataTypeInformation& topic_info_);

    /**
     * @brief Register a callback function to be notified when a new subscriber becomes available.
     *
     * @param callback_       The callback function to be called with the STopicId of the new subscriber.
     *                        The callback function must not be blocked for a longer period of time, 
     *                        otherwise timeout mechanisms of the eCAL registration would be triggered.
     *
     * @return CallbackToken  Token that can be used to unregister the callback.
     */
    ECAL_API CallbackToken AddSubscriberEventCallback(const TopicEventCallbackT& callback_);

    /**
     * @brief Unregister the subscriber callback using the provided token.
     *
     * @param token  The token returned by AddSubscriberCallback.
    */
    ECAL_API void RemSubscriberEventCallback(CallbackToken token_);

    /**
     * @brief Get complete snapshot of all known servers.
     *
     * @return Set of service id's.
    **/
    ECAL_API bool GetServerIDs(std::set<SServiceId>& service_ids_);

    /**
     * @brief Get service method information for a specific server.
     *
     * @return True if information could be queried.
    **/
    ECAL_API bool GetServerInfo(const SServiceId& id_, ServiceMethodInformationSetT& service_method_info_);

    /**
     * @brief Get complete snapshot of all known clients.
     *
     * @return Set of service id's.
    **/
    ECAL_API bool GetClientIDs(std::set<SServiceId>& service_ids_);

    /**
     * @brief Get service method information for a specific client.
     *
     * @return True if information could be queried.
    **/
    ECAL_API bool GetClientInfo(const SServiceId& id_, ServiceMethodInformationSetT& service_method_info_);

    /**
     * @brief Get all names of topics that are being published.
     *        This is a convenience function. 
     *        It calls GetPublisherIDs() and filters by name
     *
     * @param topic_names_ Set to store the topic names.
    **/
    ECAL_API bool GetPublishedTopicNames(std::set<std::string>& topic_names_);

    /**
     * @brief Get all names of topics that are being subscribed
     *        This is a convenience function. 
     *        It calls GetSubscriberIDs() and filters by name
     *
     * @param topic_names_ Set to store the topic names.
    **/
    ECAL_API bool GetSubscribedTopicNames(std::set<std::string>& topic_names_);

        /**
     * @brief Get the pairs of service name / method name of all eCAL Servers.
     *
     * @param service_method_names_ Set to store the service/method names (Set { (ServiceName, MethodName) }).
    **/
    ECAL_API bool GetServerMethodNames(std::set<SServiceMethod>& server_method_names_);

    /**
     * @brief Get the pairs of service name / method name of all eCAL Clients.
     *
     * @param client_method_names_ Set to store the client/method names (Set { (ClientName, MethodName) }).
    **/
    ECAL_API bool GetClientMethodNames(std::set<SServiceMethod>& client_method_names_);
  }
}

