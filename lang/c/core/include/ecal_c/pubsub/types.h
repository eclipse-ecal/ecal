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

#ifndef ecal_c_pubsub_types_h_included
#define ecal_c_pubsub_types_h_included

#include <ecal_c/types.h>

struct eCAL_STopicId
{
  struct eCAL_SEntityId topic_id;
  const char* topic_name;
};

enum eCAL_ePublisherEvent
{
  eCAL_ePublisherEvent_none,
  eCAL_ePublisherEvent_connected,
  eCAL_ePublisherEvent_disconnected,
  eCAL_ePublisherEvent_dropped
};

struct eCAL_SPubEventCallbackData
{
  enum eCAL_ePublisherEvent event_type;
  long long event_time;
  struct eCAL_SDataTypeInformation subscriber_datatype;
};

typedef void (*eCAL_PubEventCallbackT)(const struct eCAL_STopicId*, const struct eCAL_SPubEventCallbackData*);

enum eCAL_eSubscriberEvent
{
  eCAL_eSubscriberEvent_none,
  eCAL_eSubscriberEvent_connected,
  eCAL_eSubscriberEvent_disconnected,
  eCAL_eSubscriberEvent_dropped
};

struct eCAL_SSubEventCallbackData
{
  enum eCAL_eSubscriberEvent event_type;
  long long event_time;
  struct eCAL_SDataTypeInformation publisher_datatype;
};

typedef void (*eCAL_SubEventCallbackT)(const struct eCAL_STopicId*, const struct eCAL_SSubEventCallbackData*);

struct eCAL_SReceiveCallbackData
{
  const void* buffer;
  size_t buffer_size;
  int64_t send_timestamp;
  int64_t send_clock;
};

typedef void (*eCAL_ReceiveCallbackT)(const struct eCAL_STopicId*, const struct eCAL_SDataTypeInformation*, const struct eCAL_SReceiveCallbackData*);

#endif /* ecal_c_pubsub_publisher_h_included */