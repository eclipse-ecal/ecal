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
 * @file   ecal_callback.h
 * @brief  eCAL subscriber callback interface
**/

#pragma once

#include <ecal/cimpl/ecal_callback_cimpl.h>

#include <functional>
#include <string>

namespace eCAL
{
  /**
   * @brief eCAL subscriber receive callback struct.
  **/
  struct SReceiveCallbackData
  {
    SReceiveCallbackData()
    {
      buf   = nullptr;
      size  = 0;
      id    = 0;
      time  = 0;
      clock = 0;
    };
    void*     buf;    //!< payload buffer
    long      size;   //!< payload buffer size
    long long id;     //!< publisher id (SetId())
    long long time;   //!< publisher send time in µs
    long long clock;  //!< publisher send clock
  };

  /**
   * @brief eCAL publisher event callback struct.
  **/
  struct SPubEventCallbackData
  {
    SPubEventCallbackData()
    {
      type  = pub_event_none;
      time  = 0;
      clock = 0;
    };
    eCAL_Publisher_Event type;    //!< publisher event type
    long long            time;    //!< publisher event time in µs
    long long            clock;   //!< publisher event clock
  };

  /**
   * @brief eCAL subscriber event callback struct.
  **/
  struct SSubEventCallbackData
  {
    SSubEventCallbackData()
    {
      type  = sub_event_none;
      time  = 0;
      clock = 0;
    };
    eCAL_Subscriber_Event type;   //!< subscriber event type
    long long             time;   //!< subscriber event time in µs
    long long             clock;  //!< subscriber event clock
  };

  /**
   * @brief eCAL service attributes.
  **/
  struct SServiceAttr
  {
    std::string    key;           //!< unique service key (internal)
    std::string    hname;         //!< host name
    std::string    pname;         //!< process name
    std::string    uname;         //!< process unit name
    std::string    sname;         //!< service name
    std::string    sid;           //!< service id
    int            pid = 0;       //!< process id
    unsigned short tcp_port = 0;  //!< service tcp port
  };

  /**
   * @brief eCAL client event callback struct.
  **/
  struct SClientEventCallbackData
  {
    SClientEventCallbackData()
    {
      type = client_event_none;
      time = 0;
    };
    eCAL_Client_Event type;  //!< event type
    long long         time;  //!< event time in µs
    SServiceAttr      attr;  //!< event related service attributes
  };

  /**
   * @brief eCAL client attributes.
  **/
  struct SClientAttr
  {
    std::string    key;           //!< unique service key (internal)
    std::string    hname;         //!< host name
    std::string    pname;         //!< process name
    std::string    uname;         //!< process unit name
    std::string    sname;         //!< service name
    std::string    sid;           //!< service id
    int            pid = 0;       //!< process id
  };

  /**
   * @brief eCAL server event callback struct.
  **/
  struct SServerEventCallbackData
  {
    SServerEventCallbackData()
    {
      type = server_event_none;
      time = 0;
    };
    eCAL_Server_Event type;  //!< event type
    long long         time;  //!< event time in µs
  };

  /**
   * @brief Raw data receive callback function type.
   *
   * @param topic_name_  The topic name of the received message.
   * @param data_        Data atruct containing payload, timestamp and publication clock.
  **/
  typedef std::function<void(const char* topic_name_, const struct SReceiveCallbackData* data_)> ReceiveCallbackT;

  /**
   * @brief Timer callback function type.
  **/
  typedef std::function<void(void)> TimerCallbackT;

  /**
   * @brief Registration callback type.
   *
   * @param sample_       The sample protocol buffer regsitration payload buffer.
   * @param sample_size_  The payload buffer size.
  **/
  typedef std::function<void(const char* sample_, int sample_size_)> RegistrationCallbackT;

  /**
   * @brief Publisher event callback function type.
   *
   * @param topic_name_  The topic name of the publisher that triggered the event.
   * @param data_        Event callback data structure with the event specific informations.
  **/
  typedef std::function<void(const char* topic_name_, const struct SPubEventCallbackData* data_)> PubEventCallbackT;

  /**
   * @brief Subscriber event callback function type.
   *
   * @param topic_name_  The topic name of the subscriber that triggered the event.
   * @param data_        Event callback data structure with the event specific informations.
  **/
  typedef std::function<void(const char* topic_name_, const struct SSubEventCallbackData* data_)> SubEventCallbackT;

  /**
   * @brief Client event callback function type.
   *
   * @param name_  The name of the connection that triggered the event.
   * @param data_  Event callback data structure with the event specific informations.
  **/
  typedef std::function<void(const char* name_, const struct SClientEventCallbackData* data_)> ClientEventCallbackT;

  /**
   * @brief Server event callback function type.
   *
   * @param name_  The name of the connection that triggered the event.
   * @param data_  Event callback data structure with the event specific informations.
  **/
  typedef std::function<void(const char* name_, const struct SServerEventCallbackData* data_)> ServerEventCallbackT;
};
