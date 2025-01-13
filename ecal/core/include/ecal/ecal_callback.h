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
#include <ecal/cimpl/ecal_callback_cimpl.h>
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
    using ReceiveIDCallbackT = std::function<void(const Registration::STopicId& topic_id_, const SDataTypeInformation& data_type_info_, const SReceiveCallbackData& data_)>;

    /**
     * @brief eCAL publisher event callback struct.
    **/
    struct SPubEventIDCallbackData
    {
      eCAL_Publisher_Event type{ pub_event_none };  //!< publisher event type
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
    using PubEventIDCallbackT = std::function<void(const Registration::STopicId& topic_id_, const struct SPubEventIDCallbackData& data_)>;

    /**
     * @brief eCAL subscriber event callback struct.
    **/
    struct SSubEventIDCallbackData
    {
      eCAL_Subscriber_Event type{ sub_event_none }; //!< subscriber event type
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
    using SubEventIDCallbackT = std::function<void(const Registration::STopicId& topic_id_, const struct SSubEventIDCallbackData& data_)>;

    /**
     * @brief eCAL client event callback struct.
    **/
    struct SClientEventIDCallbackData
    {
      eCAL_Client_Event type = client_event_none;  //!< event type
      long long         time = 0;                  //!< event time in µs
    };

    /**
     * @brief Client event callback function type.
     *
     * @param service_id_  The service id struct of the connection that triggered the event.
     * @param data_        Event callback data structure with the event specific information.
    **/
    using ClientEventIDCallbackT = std::function<void(const Registration::SServiceMethodId& service_id_, const struct SClientEventIDCallbackData& data_)>;

    /**
     * @brief eCAL server event callback struct.
    **/
    struct SServerEventIDCallbackData
    {
      eCAL_Server_Event type = server_event_none;  //!< event type
      long long         time = 0;                  //!< event time in µs
    };

    /**
     * @brief Server event callback function type.
     *
     * @param service_id_  The service id struct of the connection that triggered the event.
     * @param data_        Event callback data structure with the event specific information.
    **/
    using ServerEventIDCallbackT = std::function<void(const Registration::SServiceMethodId& service_id_, const struct SServerEventIDCallbackData& data_)>;
  }
}
