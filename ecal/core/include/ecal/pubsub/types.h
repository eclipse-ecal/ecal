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
 * @file   pubsub/types.h
 * @brief  eCAL subscriber callback interface
**/

#pragma once

#include <ecal/deprecate.h>
#include <ecal/namespace.h>
#include <ecal/types.h>

#include <functional>
#include <string>

namespace eCAL
{
  struct STopicId
    {
      SEntityId    topic_id;
      std::string  topic_name;

      bool operator==(const STopicId& other) const
      {
        return topic_id == other.topic_id && topic_name == other.topic_name;
      }

      bool operator<(const STopicId& other) const
      {
        return std::tie(topic_id, topic_name) < std::tie(other.topic_id, other.topic_name);
      }
    };

  inline std::ostream& operator<<(std::ostream& os, const STopicId& id)
    {
      os << "STopicId(topic_id: " << id.topic_id
        << ", topic_name: " << id.topic_name << ")";
      return os;
    }
  
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
    dropped = 3
  };

  inline std::string to_string(eSubscriberEvent event_) {
    switch (event_) {
    case eSubscriberEvent::none:         return "NONE";
    case eSubscriberEvent::connected:    return "CONNECTED";
    case eSubscriberEvent::disconnected: return "DISCONNECTED";
    case eSubscriberEvent::dropped :     return "DROPPED";
    default:                             return "Unknown";
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
    dropped = 3
  };

  inline std::string to_string(ePublisherEvent event_) {
    switch (event_) {
    case ePublisherEvent::none:         return "NONE";
    case ePublisherEvent::connected:    return "CONNECTED";
    case ePublisherEvent::disconnected: return "DISCONNECTED";
    case ePublisherEvent::dropped:      return "DROPPED";
    default:                            return "Unknown";
    }
  }

    /**
     * @brief Receive callback function type with topic id and data struct. The topic id contains the topic name, the process
     *          name, the host name and a uniques topic identifier.
     *
     * @param topic_id_        The topic id struct of the received message.
     * @param data_type_info_  Topic data type information (encoding, type, descriptor).
     * @param data_            Data struct containing payload, timestamp and publication clock.
    **/
    using ReceiveCallbackT = std::function<void(const STopicId& topic_id_, const SDataTypeInformation& data_type_info_, const SReceiveCallbackData& data_)>;

    /**
     * @brief eCAL publisher event callback struct.
    **/
    struct SPubEventCallbackData
    {
      ePublisherEvent      event_type{ ePublisherEvent::none };  //!< publisher event type
      long long            event_time{ 0 };                      //!< publisher event time in µs (eCAL time)
      SDataTypeInformation subscriber_datatype;                  //!< datatype description of the connected subscriber
    };

    /**
     * @brief Publisher event callback function type.
     *
     * @param topic_id_  The topic id struct of the received message.
     * @param data_      Event callback data structure with the event specific information.
    **/
    using PubEventCallbackT = std::function<void(const STopicId& topic_id_, const SPubEventCallbackData& data_)>;

    /**
     * @brief eCAL subscriber event callback struct.
    **/
    struct SSubEventCallbackData
    {
      eSubscriberEvent      event_type{ eSubscriberEvent::none }; //!< subscriber event type
      long long             event_time{ 0 };                      //!< subscriber event time in µs (eCAL time)
      SDataTypeInformation  publisher_datatype;                   //!< topic information of the connected publisher
    };

    /**
     * @brief Subscriber event callback function type.
     *
     * @param topic_id_  The topic id struct of the received message.
     * @param data_      Event callback data structure with the event specific information.
    **/
    using SubEventCallbackT = std::function<void(const STopicId& topic_id_, const SSubEventCallbackData& data_)>;
}

