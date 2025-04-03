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
  /**
   * @brief A struct which uniquely identifies anybody producing or consuming topics, e.g. a CPublisher or a CSubscriber.
  **/
  struct STopicId
  {
    SEntityId    topic_id;    //!< The unique id of the topic
    std::string  topic_name;  //!< The topics name (on which matching is performed in the pub/sub case)

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
    const void* buffer = nullptr;       //!< payload buffer, containing the sent data
    size_t      buffer_size = 0;        //!< payload buffer size
    int64_t     send_timestamp = 0;     //!< publisher send timestamp in µs
    int64_t     send_clock = 0;         //!< publisher send clock. Each publisher increases the counter by one, every time a message is sent. It can be used to detect message drops.
  };

  /**
  * @brief eCAL publisher event callback type.
  **/
  enum class ePublisherEvent
  {
    none = 0,
    connected = 1,     //!< a new subscriber has been connected to the publisher
    disconnected = 2,  //!< a previously connected subscriber has been disconnected from this publisher
    dropped = 3        //!< some subscriber has missed a message that was sent by this publisher
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
 * @brief eCAL subscriber event callback type.
**/
  enum class eSubscriberEvent
  {
    none = 0,
    connected = 1,      //!< a new publisher has been connected to the subscriber
    disconnected = 2,   //!< a previously connected publisher has been disconnected from this subscriber
    dropped = 3         //!< a message coming from a publisher has been dropped, e.g. the subscriber has missed it
  };

  inline std::string to_string(eSubscriberEvent event_) {
    switch (event_) {
    case eSubscriberEvent::none:         return "NONE";
    case eSubscriberEvent::connected:    return "CONNECTED";
    case eSubscriberEvent::disconnected: return "DISCONNECTED";
    case eSubscriberEvent::dropped:      return "DROPPED";
    default:                             return "Unknown";
    }
  }

  /**
   * @brief Receive callback function type. A user can register this callback type with a subscriber, and this callback will be triggered when the user receives any data.
   *
   * @param publisher_id_    The topic id of the publisher that has sent the data which is now being received.
   * @param data_type_info_  Topic metadata, as set by the publisher (encoding, type, descriptor). 
   *                         This can be used to validate that the received data can be properly interpreted by the subscriber.
   * @param data_            Data struct containing payload, timestamp and publication clock.
  **/
  using ReceiveCallbackT = std::function<void(const STopicId& publisher_id_, const SDataTypeInformation& data_type_info_, const SReceiveCallbackData& data_)>;

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

