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
 * @file   service/types.h
 * @brief  Type definitions for eCAL services
**/

#pragma once

#include <ecal/namespace.h>
#include <ecal/types.h>

#include <functional>
#include <string>
#include <vector>
#include <set>

namespace eCAL
{
  /**
   * @brief  Service call state. This enum class is being used when a client is calling a servers.
  **/
  enum class eCallState
  {
    none = 0,    //!< undefined
    executed,    //!< the service call was executed successfully
    timeouted,   //!< the service call has timeouted
    failed       //!< failed
  };

  /**
   * @brief eCAL service client event callback type.
  **/
  // TODO: add documentation!!!
  enum class eClientEvent
  {
    none = 0,          //!< undefined
    connected = 1,     //!< a new server has been connected to this client
    disconnected = 2,  //!< a server has been disconnected from this client
    // TODO: does it make sense here? the user is already being notified about the timeout via the eCallState
    timeout = 3,       //!< a service call has timeouted 
  };

  inline std::string to_string(eClientEvent event_) {
    switch (event_) {
    case eClientEvent::none:                   return "NONE";
    case eClientEvent::connected:              return "CONNECTED";
    case eClientEvent::disconnected:           return "DISCONNECTED";
    default:            return "Unknown";
    }
  }

  /**
   * @brief eCAL service server event callback type.
  **/
  enum class eServerEvent
  {
    none = 0,         //!< undefined
    connected = 1,    //!< a new client has been connected to this server
    disconnected = 2, //!< a client has been disconnected from this server
  };

  inline std::string to_string(eServerEvent event_) {
    switch (event_) {
    case eServerEvent::none:                   return "NONE";
    case eServerEvent::connected:              return "CONNECTED";
    case eServerEvent::disconnected:           return "DISCONNECTED";
    default:            return "Unknown";
    }
  }

  /**
   * @brief Unique ID with which to identify a service (client or server)
   *        It can be queried from the client or the server, and it can be obtained from the registration layer.
  **/
  struct SServiceId
  {
    SEntityId    service_id;    //!< the actual ID
    std::string  service_name;  //!< the name of the service

    bool operator==(const SServiceId& other) const
    {
      return service_id == other.service_id && service_name == other.service_name;
    }

    bool operator<(const SServiceId& other) const
    {
      return std::tie(service_id, service_name) < std::tie(other.service_id, other.service_name);
    }
  };

  /**
   * @brief Service method information struct containing the method name, the request and response type information.
   *        This type is used when creating services (servers or clients), or when querying information about them from the registration layer.
  **/
  struct SServiceMethodInformation
  {
    std::string              method_name;     //!< The name of the method.
    SDataTypeInformation     request_type;    //!< The type of the method request.
    SDataTypeInformation     response_type;   //!< The type of the method response.

    bool operator==(const SServiceMethodInformation& other) const
    {
      return method_name == other.method_name && request_type == other.request_type && response_type == other.response_type;
    }

    bool operator<(const SServiceMethodInformation& other) const
    {
      return std::tie(method_name, method_name, response_type) < std::tie(other.method_name, other.method_name, other.response_type);
    }
  };

  /**
   * @brief Set SServiceMethodInformation (name, request type, reponse type).
   *            Used to Create a Client Instance
   *            Can be queried from eCAL::Registration.
  **/
  using ServiceMethodInformationSetT = std::set<SServiceMethodInformation>;

  /**
   * @brief Service response struct containing the (responding) server information and the response itself.
  **/
  struct SServiceResponse
  {
    eCallState                     call_state = eCallState::none; //!< call state, to indicate if the call was successful or not

    SServiceId                     server_id;                    //!< Id of the server that executed the service (server id (entity id, process id, host name), name)
    // TODO: the datatypeinformation of the service_method_information are not yet being filled!!!
    SServiceMethodInformation      service_method_information;   //!< Additional Information about the method that has been called (name & DatatypeInformation of request and reponse)
    // TODO: does this value even make sense? or should service methods always return void? It's not actually used anywhere
    int                            ret_state  = 0;               //!< return state of the called service method 
    std::string                    response;                     //!< the actual response data of the service call
  
    std::string                    error_msg;                    //!< human readable error message, in case that the service call could not be executed.
  };
  using ServiceResponseVecT = std::vector<SServiceResponse>; //!< vector of multiple service responses

  /**
   * @brief Service response callback function type (low level client interface).
   * 
   * @param service_response_  Service response struct containing the (responding) server informations and the response itself.
  **/
  using ResponseCallbackT = std::function<void (const SServiceResponse& service_response_)>;

  /**
   * @brief Service method callback function type (low level server interface).
   *        This is the type definition of a function that can be registered for a CServiceServer.
   *        It callback is then called, a client
   *
   * @param method_info The method information struct containing the request and response type information.
   * @param request_    The request.
   * @param response_   The response returned from the method call.
  **/
  using ServiceMethodCallbackT = std::function<int(const SServiceMethodInformation& method_info_, const std::string& request_, std::string& response_)>;
 
  /**
   * @brief eCAL client event callback struct.
  **/
  struct SClientEventCallbackData
  {
    eClientEvent      type{ eClientEvent::none };  //!< event type
    long long         time = 0;                  //!< event time in µs
  };

  /**
   * @brief Client event callback function type.
   *
   * @param service_id_  The service id struct of the connection that triggered the event.
   * @param data_        Event callback data structure with the event specific information.
  **/
  using ClientEventCallbackT = std::function<void(const SServiceId& service_id_, const SClientEventCallbackData& data_)>;

  /**
   * @brief eCAL server event callback struct.
  **/
  struct SServerEventCallbackData
  {
    eServerEvent      type{ eServerEvent::none };  //!< event type
    long long         time = 0;                  //!< event time in µs
  };

  /**
   * @brief Server event callback function type.
   *
   * @param service_id_  The service id struct of the connection that triggered the event.
   * @param data_        Event callback data structure with the event specific information.
  **/
  using ServerEventCallbackT = std::function<void(const SServiceId& service_id_, const struct SServerEventCallbackData& data_)>;

}
