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
 * @file   ecal_c/callback.h
 * @brief  eCAL callback interface events, structs and functions
**/

#ifndef ecal_c_callback_h_included
#define ecal_c_callback_h_included

/**
 * @brief eCAL subscriber event callback type.
**/
enum eCAL_Subscriber_Event
{
  sub_event_none              = 0,
  sub_event_connected         = 1,
  sub_event_disconnected      = 2,
  sub_event_dropped           = 3,
  sub_event_corrupted         = 5,
  sub_event_update_connection = 6,
};

/**
 * @brief eCAL publisher event callback type.
**/
enum eCAL_Publisher_Event
{
  pub_event_none              = 0,
  pub_event_connected         = 1,
  pub_event_disconnected      = 2,
  pub_event_dropped           = 3,
  pub_event_update_connection = 4,
};

/**
 * @brief eCAL service client event callback type.
**/
enum eCAL_Client_Event
{
  client_event_none         = 0,
  client_event_connected    = 1,
  client_event_disconnected = 2,
  client_event_timeout      = 3,
};

/**
 * @brief eCAL service server event callback type.
**/
enum eCAL_Server_Event
{
  server_event_none         = 0,
  server_event_connected    = 1,
  server_event_disconnected = 2,
};

#ifdef _MSC_VER
#pragma pack(push, 8)
#endif

/**
 * @brief eCAL subscriber receive callback struct (C variant).
**/
struct SReceiveCallbackDataC
{
  void*     buf;    //!< payload buffer
  long      size;   //!< payload buffer size
  long long id;     //!< source id
  long long time;   //!< source time stamp
  long long clock;  //!< source write clock
};

/**
 * @brief eCAL publisher event callback struct (C variant).
**/
struct SPubEventCallbackDataC
{
  enum eCAL_Publisher_Event  type;         //!< event type
  long long                  time;         //!< event time stamp
  long long                  clock;        //!< event clock
  const char*                topic_id;     //!< topic id of the connected subscriber                          (for pub_event_update_connection only)
  const char*                topic_name;   //!< topic type encoding of the connected subscriber               (for pub_event_update_connection only)
  const char*                tencoding;    //!< topic type name of the connected subscriber                   (for pub_event_update_connection only)
  const char*                tdesc;        //!< topic type descriptor information of the connected subscriber (for pub_event_update_connection only)
};

/**
 * @brief eCAL subscriber event callback struct (C variant).
**/
struct SSubEventCallbackDataC
{
  enum eCAL_Subscriber_Event type;         //!< event type
  long long                  time;         //!< event time stamp
  long long                  clock;        //!< event clock
  const char*                topic_id;     //!< topic id of the connected publisher                          (for sub_event_update_connection only)
  const char*                topic_name;   //!< topic type encoding of the connected publisher               (for sub_event_update_connection only)
  const char*                tencoding;    //!< topic type name of the connected publisher                   (for sub_event_update_connection only)
  const char*                tdesc;        //!< topic type descriptor information of the connected publisher (for sub_event_update_connection only)
};

/**
 * @brief eCAL client event callback struct (C variant).
**/
struct SClientEventCallbackDataC
{
  enum eCAL_Client_Event type;  //!< event type
  long long              time;  //!< event time stamp
};

/**
 * @brief eCAL server event callback struct (C variant).
**/
struct SServerEventCallbackDataC
{
  enum eCAL_Server_Event type;  //!< event type
  long long              time;  //!< event time stamp
};

#ifdef _MSC_VER
#pragma pack(pop)
#endif

/**
 * @brief eCAL receive callback function
 *
 * @param topic_name_  Topic name of the data source (publisher).
 * @param data_        Data payload struct.
 * @param par_         Forwarded user defined parameter.
**/
typedef void (*ReceiveCallbackCT)(const char* topic_name_, const struct SReceiveCallbackDataC* data_, void* par_);

/**
 * @brief eCAL timer callback function
 *
 * @param par_         Forwarded user defined parameter.
**/
typedef void (*TimerCallbackCT)(void* par_);

/**
 * @brief eCAL publisher event callback function
 *
 * @param topic_name_  The topic name of the data connection (subscriber).
 * @param data_        Event type struct.
 * @param par_         Forwarded user defined parameter.
**/
typedef void(*SubEventCallbackCT)(const char* topic_name_, const struct SSubEventCallbackDataC* data_, void* par_);

/**
 * @brief eCAL subscriber event callback function
 *
 * @param topic_name_  The topic name of the data connection (publisher).
 * @param data_        Event type struct.
 * @param par_         Forwarded user defined parameter.
**/
typedef void(*PubEventCallbackCT)(const char* topic_name_, const struct SPubEventCallbackDataC* data_, void* par_);

/**
 * @brief eCAL client event callback function
 *
 * @param name_  The service name.
 * @param data_  Event type struct.
 * @param par_   Forwarded user defined parameter.
**/
typedef void(*ClientEventCallbackCT)(const char* name_, const struct SClientEventCallbackDataC* data_, void* par_);

/**
 * @brief eCAL server event callback function
 *
 * @param name_  The service name.
 * @param data_  Event type struct.
 * @param par_   Forwarded user defined parameter.
**/
typedef void(*ServerEventCallbackCT)(const char* name_, const struct SServerEventCallbackDataC* data_, void* par_);

#endif /* ecal_c_callback_h_included */
