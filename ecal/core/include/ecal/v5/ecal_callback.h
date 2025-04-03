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
 * @file   v5/ecal_callback.h
 * @brief  eCAL subscriber callback interface
**/

#pragma once

#include <ecal/namespace.h>

#include <ecal/deprecate.h>
#include <ecal/pubsub/types.h>   // Should we instead bring back our old event types???
#include <ecal/service/types.h>  // Should we instead bring back our old event types???

#include <functional>
#include <string>

namespace eCAL
{
  ECAL_CORE_NAMESPACE_V5
  {
    /**
     * @brief eCAL subscriber receive callback struct.
     **/
    struct SReceiveCallbackData
    {
      void* buf = nullptr;  //!< payload buffer
      long      size = 0;        //!< payload buffer size
      long long id = 0;        //!< publisher id (SetId())
      long long time = 0;        //!< publisher send time in µs
      long long clock = 0;        //!< publisher send clock
    };


    /**
   * @brief eCAL publisher event callback struct.
  **/
    struct SPubEventCallbackData
    {
      ePublisherEvent      type = ePublisherEvent::none;  //!< publisher event type
      long long            time{ 0 };               //!< publisher event time in µs
      long long            clock{ 0 };              //!< publisher event clock
      std::string          tid;                     //!< topic id of the of the connected subscriber                 (for pub_event_update_connection only)
      SDataTypeInformation tdatatype;               //!< datatype description of the connected subscriber            (for pub_event_update_connection only)
    };

    /**
     * @brief eCAL subscriber event callback struct.
    **/
    struct SSubEventCallbackData
    {
      eSubscriberEvent      type = eSubscriberEvent::none; //!< subscriber event type
      long long             time{ 0 };              //!< subscriber event time in µs
      long long             clock{ 0 };             //!< subscriber event clock
      std::string           tid;                  //!< topic id of the of the connected publisher              (for sub_event_update_connection only)
      SDataTypeInformation  tdatatype;            //!< topic information of the connected subscriber           (for sub_event_update_connection only)
    };

    /**
     * @brief eCAL service attributes.
    **/
    struct SServiceAttr
    {
      std::string             key;              //!< unique service key (internal)
      std::string             hname;            //!< host name
      std::string             pname;            //!< process name
      std::string             uname;            //!< process unit name
      std::string             sname;            //!< service name
      EntityIdT sid = 0;                        //!< service id
      int                     pid = 0;          //!< process id

      // internal protocol specifics
      unsigned int   version = 0;  //!< service protocol version
      unsigned short tcp_port_v0 = 0;  //!< service tcp port protocol version 0
      unsigned short tcp_port_v1 = 0;  //!< service tcp port protocol version 1
    };

    /**
     * @brief eCAL client event callback struct.
    **/
    struct SClientEventCallbackData
    {
      eClientEvent      type = eClientEvent::none;  //!< event type
      long long         time = 0;                  //!< event time in µs
      SServiceAttr      attr;                      //!< event related service attributes
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

      unsigned int   version = 0;   //!< client version
    };


    /**
     * @brief Service response struct containing the (responding) server informations and the response itself. (deprecated)
    **/
    struct SServiceResponse
    {
      SServiceResponse()
      {
        ret_state = 0;
        call_state = eCallState::none;
      };
      std::string  host_name;      //!< service host name
      std::string  service_name;   //!< name of the service
      std::string  service_id;     //!< id of the service
      std::string  method_name;    //!< name of the service method
      std::string  error_msg;      //!< human readable error message
      int          ret_state;      //!< return state of the called service method
      eCallState   call_state;     //!< call state (see eCallState)
      std::string  response;       //!< service response
    };
    using ServiceResponseVecT = std::vector<SServiceResponse>; //!< vector of multiple service responses (deprecated)

    /**
     * @brief Service response callback function type (low level client interface). (deprecated)
     *
     * @param service_response_  Service response struct containing the (responding) server informations and the response itself.
    **/
    using ResponseCallbackT = std::function<void(const struct v5::SServiceResponse& service_response_)>;

    /**
     * @brief Service method callback function type (low level server interface). (deprecated)
     *
     * @param method_     The method name.
     * @param req_type_   The type of the method request.
     * @param resp_type_  The type of the method response.
     * @param request_    The request.
     * @param response_   The response returned from the method call.
    **/
    using MethodCallbackT = std::function<int(const std::string& method_, const std::string& req_type_, const std::string& resp_type_, const std::string& request_, std::string& response_)>;


    /**
     * @brief eCAL server event callback struct.
    **/
    struct SServerEventCallbackData
    {
      eServerEvent      type = eServerEvent::none;  //!< event type
      long long         time = 0;                  //!< event time in µs
    };

    /**
     * @brief Receive callback function type with topic name and data struct. (deprecated)
     *
     * @param topic_name_  The topic name of the received message.
     * @param data_        Data struct containing payload, timestamp and publication clock.
    **/
    using ReceiveCallbackT = std::function<void(const char* topic_name_, const struct v5::SReceiveCallbackData* data_)>;

    /**
     * @brief Publisher event callback function type. (deprecated)
     *
     * @param topic_name_  The topic name of the publisher that triggered the event.
     * @param data_        Event callback data structure with the event specific information.
    **/
    using PubEventCallbackT = std::function<void(const char* topic_name_, const struct SPubEventCallbackData* data_)>;

    /**
     * @brief Subscriber event callback function type. (deprecated)
     *
     * @param topic_name_  The topic name of the subscriber that triggered the event.
     * @param data_        Event callback data structure with the event specific information.
    **/
    using SubEventCallbackT = std::function<void(const char* topic_name_, const struct SSubEventCallbackData* data_)>;

    /**
     * @brief Client event callback function type. (deprecated)
     *
     * @param service_name_  The service name of the connection that triggered the event.
     * @param data_          Event callback data structure with the event specific information.
    **/
    using ClientEventCallbackT = std::function<void(const char* service_name_, const struct SClientEventCallbackData* data_)>;

    /**
     * @brief Server event callback function type. (deprecated)
     *
     * @param service_name_  The service name of the connection that triggered the event.
     * @param data_          Event callback data structure with the event specific information.
    **/
    using ServerEventCallbackT = std::function<void(const char* service_name_, const struct SServerEventCallbackData* data_)>;
  }
}
