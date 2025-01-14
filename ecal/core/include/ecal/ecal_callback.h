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
  enum class Subscriber_Event
  {
    sub_event_none = 0,
    sub_event_connected = 1,
    sub_event_disconnected = 2,
    sub_event_dropped = 3,
    sub_event_corrupted = 5,
    sub_event_update_connection = 6,
  };

  inline std::string to_string(Subscriber_Event event_) {
    switch (event_) {
    case Subscriber_Event::sub_event_none:                   return "NONE";
    case Subscriber_Event::sub_event_connected:              return "CONNECTED";
    case Subscriber_Event::sub_event_disconnected:           return "DISCONNECTED";
    case Subscriber_Event::sub_event_dropped :               return "DROPPED";
    case Subscriber_Event::sub_event_corrupted:              return "CORRUPTED";
    case Subscriber_Event::sub_event_update_connection :     return "UPDATED_CONNECTION";
    default:            return "Unknown";
    }
  }

  /**
   * @brief eCAL publisher event callback type.
  **/
  enum class Publisher_Event
  {
    pub_event_none = 0,
    pub_event_connected = 1,
    pub_event_disconnected = 2,
    pub_event_dropped = 3,
    pub_event_update_connection = 4,
  };

  inline std::string to_string(Publisher_Event event_) {
    switch (event_) {
    case Publisher_Event::pub_event_none:                   return "NONE";
    case Publisher_Event::pub_event_connected:              return "CONNECTED";
    case Publisher_Event::pub_event_disconnected:           return "DISCONNECTED";
    case Publisher_Event::pub_event_dropped:                return "DROPPED";
    case Publisher_Event::pub_event_update_connection:      return "UPDATED_CONNECTION";
    default:            return "Unknown";
    }
  }

  /**
   * @brief eCAL service client event callback type.
  **/
  enum class Client_Event
  {
    client_event_none = 0,
    client_event_connected = 1,
    client_event_disconnected = 2,
    client_event_timeout = 3,
  };

  inline std::string to_string(Client_Event event_) {
    switch (event_) {
    case Client_Event::client_event_none:                   return "NONE";
    case Client_Event::client_event_connected:              return "CONNECTED";
    case Client_Event::client_event_disconnected:           return "DISCONNECTED";
    default:            return "Unknown";
    }
  }

  /**
   * @brief eCAL service server event callback type.
  **/
  enum class Server_Event
  {
    server_event_none = 0,
    server_event_connected = 1,
    server_event_disconnected = 2,
  };

  inline std::string to_string(Server_Event event_) {
    switch (event_) {
    case Server_Event::server_event_none:                   return "NONE";
    case Server_Event::server_event_connected:              return "CONNECTED";
    case Server_Event::server_event_disconnected:           return "DISCONNECTED";
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
      Publisher_Event      type{ Publisher_Event::pub_event_none };  //!< publisher event type
      long long            time{ 0 };               //!< publisher event time in µs
      long long            clock{ 0 };              //!< publisher event clock
      SDataTypeInformation tdatatype;               //!< datatype description of the connected subscriber            (for pub_event_update_connection only)
    };

    /**
     * @brief Publisher event callback function type.
     *
     * @param topic_id_  The topic id struct of the received message.
     * @param data_      Event callback data structure with the event specific information.
    **/
    using PubEventCallbackT = std::function<void(const Registration::STopicId& topic_id_, const SPubEventCallbackData& data_)>;

    /**
     * @brief eCAL subscriber event callback struct.
    **/
    struct SSubEventCallbackData
    {
      Subscriber_Event      type{ Subscriber_Event::sub_event_none }; //!< subscriber event type
      long long             time{ 0 };              //!< subscriber event time in µs
      long long             clock{ 0 };             //!< subscriber event clock
      SDataTypeInformation  tdatatype;              //!< topic information of the connected subscriber           (for sub_event_update_connection only)
    };

    /**
     * @brief Subscriber event callback function type.
     *
     * @param topic_id_  The topic id struct of the received message.
     * @param data_      Event callback data structure with the event specific information.
    **/
    using SubEventCallbackT = std::function<void(const Registration::STopicId& topic_id_, const SSubEventCallbackData& data_)>;

    /**
     * @brief eCAL client event callback struct.
    **/
    struct SClientEventCallbackData
    {
      Client_Event      type{ Client_Event::client_event_none };  //!< event type
      long long         time = 0;                  //!< event time in µs
    };

    /**
     * @brief Client event callback function type.
     *
     * @param service_id_  The service id struct of the connection that triggered the event.
     * @param data_        Event callback data structure with the event specific information.
    **/
    using ClientEventCallbackT = std::function<void(const Registration::SServiceMethodId& service_id_, const SClientEventCallbackData& data_)>;

    /**
     * @brief eCAL server event callback struct.
    **/
    struct SServerEventCallbackData
    {
      Server_Event      type{ Server_Event::server_event_none };  //!< event type
      long long         time = 0;                  //!< event time in µs
    };

    /**
     * @brief Server event callback function type.
     *
     * @param service_id_  The service id struct of the connection that triggered the event.
     * @param data_        Event callback data structure with the event specific information.
    **/
    using ServerEventCallbackT = std::function<void(const Registration::SServiceMethodId& service_id_, const struct SServerEventCallbackData& data_)>;
  }
}
