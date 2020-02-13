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
 * @file   ecal_callback_cimpl.h
 * @brief  eCAL subscriber callback interface for C Interface
**/

#ifndef ecal_callback_cimpl_h_included
#define ecal_callback_cimpl_h_included

/**
 * @brief eCAL subscriber event callback struct.
**/
enum eCAL_Subscriber_Event
{
  sub_event_none          =  0,
  sub_event_connected     =  1,
  sub_event_disconnected  =  2,
  sub_event_dropped       =  3,
  sub_event_timeout       =  4,
  sub_event_corrupted     =  5,
};

/**
 * @brief eCAL publisher event callback struct.
**/
enum eCAL_Publisher_Event
{
  pub_event_none          =  0,
  pub_event_connected     =  1,
  pub_event_disconnected  =  2,
  pub_event_dropped       =  3,
};

/**
 * @brief eCAL registration event type.
**/
enum eCAL_Registration_Event
{
  reg_event_none          = 0,
  reg_event_publisher     = 1,
  reg_event_subscriber    = 2,
  reg_event_service       = 3,
  reg_event_process       = 4
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
 * @brief eCAL subscriber event callback struct (C variant).
**/
#ifdef _MSC_VER
#pragma pack(push, 8)
#endif

/**
 * @brief eCAL publisher event callback struct (C variant).
**/
struct SPubEventCallbackDataC
{
  enum eCAL_Publisher_Event  type;  //!< event type
  long long                  time;  //!< event time stamp
  long long                  clock; //!< event clock
};

/**
 * @brief eCAL subscriber event callback struct (C variant).
**/
struct SSubEventCallbackDataC
{
  enum eCAL_Subscriber_Event type;  //!< event type
  long long                  time;  //!< event time stamp
  long long                  clock; //!< event clock
};

#ifdef _MSC_VER
#pragma pack(pop)
#endif

/**
 * @brief eCAL publisher event callback function
 *
 * @param topic_name_  Topic name of the data connection (subscriber).
 * @param data_        Event type struct.
 * @param par_         Forwarded user defined parameter.
**/
typedef void(*SubEventCallbackCT)(const char* topic_name_, const struct SSubEventCallbackDataC* data_, void* par_);

/**
 * @brief eCAL subscriber event callback function
 *
 * @param topic_name_  Topic name of the data connection (publisher).
 * @param data_        Event type struct.
 * @param par_         Forwarded user defined parameter.
**/
typedef void(*PubEventCallbackCT)(const char* topic_name_, const struct SPubEventCallbackDataC* data_, void* par_);

#endif /* ecal_callback_cimpl_h_included */
