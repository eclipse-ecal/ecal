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
 * @file   ecal_callback.h
 * @brief  eCAL subscriber callback interface
**/

#pragma once

#include <ecal/ecal_deprecate.h>
#include <ecal/ecal_types.h>

#include <functional>
#include <string>

namespace eCAL
{
  /**
   * @brief eCAL subscriber receive callback struct.
  **/
  struct SReceiveCallbackData
  {
    void*     buf   = nullptr;  //!< payload buffer
    long      size  = 0;        //!< payload buffer size
    long long id    = 0;        //!< publisher id (SetId())
    long long time  = 0;        //!< publisher send time in µs
    long long clock = 0;        //!< publisher send clock
  };

  /**
 * @brief eCAL subscriber event callback type.
**/
  enum class eSubscriberEvent
  {
    none = 0,
    connected = 1,
    disconnected = 2,
    dropped = 3,
    corrupted = 5,
    update_connection = 6,
  };

  inline std::string to_string(eSubscriberEvent event_) {
    switch (event_) {
    case eSubscriberEvent::none:                   return "NONE";
    case eSubscriberEvent::connected:              return "CONNECTED";
    case eSubscriberEvent::disconnected:           return "DISCONNECTED";
    case eSubscriberEvent::dropped :               return "DROPPED";
    case eSubscriberEvent::corrupted:              return "CORRUPTED";
    case eSubscriberEvent::update_connection :     return "UPDATED_CONNECTION";
    default:            return "Unknown";
    }
  }

  /**
   * @brief eCAL publisher event callback type.
  **/
  enum class ePublisherEvent
  {
    none = 0,
    connected = 1,
    disconnected = 2,
    dropped = 3,
    update_connection = 4,
  };

  inline std::string to_string(ePublisherEvent event_) {
    switch (event_) {
    case ePublisherEvent::none:                   return "NONE";
    case ePublisherEvent::connected:              return "CONNECTED";
    case ePublisherEvent::disconnected:           return "DISCONNECTED";
    case ePublisherEvent::dropped:                return "DROPPED";
    case ePublisherEvent::update_connection:      return "UPDATED_CONNECTION";
    default:            return "Unknown";
    }
  }

  /**
   * @brief eCAL service client event callback type.
  **/
  enum class eClientEvent
  {
    none = 0,
    connected = 1,
    disconnected = 2,
    timeout = 3,
  };

  inline std::string to_string(eClientEvent event_) {
    switch (event_) {
    case eClientEvent::none:                   return "NONE";
    case eClientEvent::connected:              return "CONNECTED";
    case eClientEvent::disconnected:           return "DISCONNECTED";
    default:            return "Unknown";
    }
  }

  /**
   * @brief eCAL service server event callback type.
  **/
  enum class eServerEvent
  {
    none = 0,
    connected = 1,
    disconnected = 2,
  };

  inline std::string to_string(eServerEvent event_) {
    switch (event_) {
    case eServerEvent::none:                   return "NONE";
    case eServerEvent::connected:              return "CONNECTED";
    case eServerEvent::disconnected:           return "DISCONNECTED";
    default:            return "Unknown";
    }
  }

  /**
   * @brief Timer callback function type.
  **/
  using TimerCallbackT = std::function<void()>;

  inline namespace v6
  {
    /**
     * @brief Receive callback function type with topic id and data struct. The topic id contains the topic name, the process
     *          name, the host name and a uniques topic identifier.
     *
     * @param topic_id_        The topic id struct of the received message.
     * @param data_type_info_  Topic data type information (encoding, type, descriptor).
     * @param data_            Data struct containing payload, timestamp and publication clock.
    **/
    using ReceiveCallbackT = std::function<void(const Registration::STopicId& topic_id_, const SDataTypeInformation& data_type_info_, const SReceiveCallbackData& data_)>;

    /**
     * @brief eCAL publisher event callback struct.
    **/
    struct SPubEventCallbackData
    {
      Registration::STopicId publisher_id;                   //!< ID of the publisher triggering the event
      ePublisherEvent        type{ ePublisherEvent::none };  //!< publisher event type
      long long              time{ 0 };                      //!< publisher event time in µs
      long long              clock{ 0 };                     //!< publisher event clock
      SDataTypeInformation   tdatatype;                      //!< datatype description of the connected subscriber         (for pub_event_update_connection only)
    };

    /**
     * @brief Publisher event callback function type.
     *
     * @param topic_id_  The topic id struct of the received message.
     * @param data_      Event callback data structure with the event specific information.
    **/
    using PubEventCallbackT = std::function<void(const SPubEventCallbackData& data_)>;

    /**
     * @brief eCAL subscriber event callback struct.
    **/
    struct SSubEventCallbackData
    {
      Registration::STopicId subscriber_id;                  //!< ID of the publisher triggering the event
      eSubscriberEvent       type{ eSubscriberEvent::none }; //!< subscriber event type
      long long              time{ 0 };                      //!< subscriber event time in µs
      long long              clock{ 0 };                     //!< subscriber event clock
      SDataTypeInformation   tdatatype;                      //!< topic information of the connected subscriber           (for sub_event_update_connection only)
    };

    /**
     * @brief Subscriber event callback function type.
     *
     * @param topic_id_  The topic id struct of the received message.
     * @param data_      Event callback data structure with the event specific information.
    **/
    using SubEventCallbackT = std::function<void(const SSubEventCallbackData& data_)>;

    /**
     * @brief eCAL client event callback struct.
    **/
    struct SClientEventCallbackData
    {
      // TODO: shouldn't this be a SServiceId? No methods involved?
      Registration::SServiceMethodId  client_id;                   //!< ID of the client triggering the event
      eClientEvent                    type{ eClientEvent::none };  //!< event type
      long long                       time = 0;                    //!< event time in µs
    };

    /**
     * @brief Client event callback function type.
     *
     * @param service_id_  The service id struct of the connection that triggered the event.
     * @param data_        Event callback data structure with the event specific information.
    **/
    using ClientEventCallbackT = std::function<void(const SClientEventCallbackData& data_)>;

    /**
     * @brief eCAL server event callback struct.
    **/
    struct SServerEventCallbackData
    {
      // TODO: shouldn't this be a SServiceId? No methods involved?
      Registration::SServiceMethodId  server_id;                   //!< ID of the server triggering the event
      eServerEvent                    type{ eServerEvent::none };  //!< event type
      long long                       time = 0;                    //!< event time in µs
    };

    /**
     * @brief Server event callback function type.
     *
     * @param service_id_  The service id struct of the connection that triggered the event.
     * @param data_        Event callback data structure with the event specific information.
    **/
    using ServerEventCallbackT = std::function<void(const struct SServerEventCallbackData& data_)>;
  }
}
