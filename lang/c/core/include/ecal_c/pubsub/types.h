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
 * @brief  eCAL subscriber callback c interface
**/

#ifndef ecal_c_pubsub_types_h_included
#define ecal_c_pubsub_types_h_included

#include <ecal_c/types.h>

#include <stddef.h>
#include <stdint.h>

/**
 * @brief A struct which uniquely identifies anybody producing or consuming topics, e.g. a CPublisher or a CSubscriber.
**/
struct eCAL_STopicId
{
  struct eCAL_SEntityId topic_id; //!< The unique id of the topic
  const char* topic_name;         //!< The topics name (on which matching is performed in the pub/sub case)
};

/**
* @brief eCAL publisher event callback type.
**/
enum eCAL_ePublisherEvent
{
  eCAL_ePublisherEvent_none,
  eCAL_ePublisherEvent_connected,    //!< a new subscriber has been connected to the publisher
  eCAL_ePublisherEvent_disconnected, //!< a previously connected subscriber has been disconnected from this publisher
  eCAL_ePublisherEvent_dropped        //!< some subscriber has missed a message that was sent by this publisher
};

/**
 * @brief eCAL publisher event callback struct.
**/
struct eCAL_SPubEventCallbackData
{
  enum eCAL_ePublisherEvent event_type;                 //!< publisher event type
  long long event_time;                                 //!< publisher event time in µs (eCAL time)
  struct eCAL_SDataTypeInformation subscriber_datatype; //!< datatype description of the connected subscriber
};

/**
 * @brief Publisher event callback function type.
 *
 * @param topic_id_  The topic id struct of the received message.
 * @param data_      Event callback data structure with the event specific information.
**/
typedef void (*eCAL_PubEventCallbackT)(const struct eCAL_STopicId*, const struct eCAL_SPubEventCallbackData*);

enum eCAL_eSubscriberEvent
{
  eCAL_eSubscriberEvent_none,
  eCAL_eSubscriberEvent_connected,
  eCAL_eSubscriberEvent_disconnected,
  eCAL_eSubscriberEvent_dropped
};

/**
 * @brief eCAL publisher event callback struct.
**/
struct eCAL_SSubEventCallbackData
{
  enum eCAL_eSubscriberEvent event_type;               //!< publisher event type
  long long event_time;                                //!< publisher event time in µs (eCAL time)
  struct eCAL_SDataTypeInformation publisher_datatype; //!< datatype description of the connected subscriber
};

/**
 * @brief Subscriber event callback function type.
 *
 * @param topic_id_  The topic id struct of the received message.
 * @param data_      Event callback data structure with the event specific information.
**/
typedef void (*eCAL_SubEventCallbackT)(const struct eCAL_STopicId*, const struct eCAL_SSubEventCallbackData*);

/**
  * @brief eCAL subscriber receive callback struct.
**/
struct eCAL_SReceiveCallbackData
{
  const void* buffer;     //!< payload buffer, containing the sent data
  size_t buffer_size;     //!< payload buffer size
  int64_t send_timestamp; //!< publisher send timestamp in µs
  int64_t send_clock;     //!< publisher send clock. Each publisher increases the counter by one, every time a message is sent. It can be used to detect message drops.
};


/**
 * @brief Receive callback function type. A user can register this callback type with a subscriber, and this callback will be triggered when the user receives any data.
 *
 * @param publisher_id_    The topic id of the publisher that has sent the data which is now being received.
 * @param data_type_info_  Topic metadata, as set by the publisher (encoding, type, descriptor).
 *                         This can be used to validate that the received data can be properly interpreted by the subscriber.
 * @param data_            Data struct containing payload, timestamp and publication clock.
**/
typedef void (*eCAL_ReceiveCallbackT)(const struct eCAL_STopicId*, const struct eCAL_SDataTypeInformation*, const struct eCAL_SReceiveCallbackData*);

#endif /* ecal_c_pubsub_publisher_h_included */