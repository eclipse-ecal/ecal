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

#ifndef ecal_c_service_types_h_included
#define ecal_c_service_types_h_included

#include <ecal_c/types.h>

#include <stddef.h>

/**
 * @brief Unique ID with which to identify a service (client or server)
 *        It can be queried from the client or the server, and it can be obtained from the registration layer.
**/
struct eCAL_SServiceId
{
  struct eCAL_SEntityId service_id;
  const char* service_name;
};

/**
 * @brief eCAL service client event callback type.
**/
enum eCAL_eClientEvent
{
  eCAL_eClientEvent_none,         //!< undefined
  eCAL_eClientEvent_connected,    //!< a new server has been connected to this client
  eCAL_eClientEvent_disconnected, //!< a server has been disconnected from this client
  eCAL_eClientEvent_timeout       //!< a service call has timeouted 
};

/**
 * @brief  Service call state. This enum class is being used when a client is calling a servers.
**/
enum eCAL_eCallState
{
  eCAL_eCallState_none,      //!< undefined
  eCAL_eCallState_executed,  //!< the service call was executed successfully
  eCAL_eCallState_timeouted, //!< the service call has timeouted
  eCAL_eCallState_failed     //!< failed
};

/**
 * @brief eCAL client event callback struct.
**/
struct eCAL_SClientEventCallbackData
{
  enum eCAL_eClientEvent type; //!< event type
  long long time;              //!< event time in µs
};

/** 
 * @brief Client event callback function type.
 *
 * @param service_id_  The service id struct of the connection that triggered the event.
 * @param data_        Event callback data structure with the event specific information.
**/
typedef void (*eCAL_ClientEventCallbackT)(const struct eCAL_SServiceId* /*service_id_*/, const struct eCAL_SClientEventCallbackData* /*data_*/);

/**
 * @brief eCAL service server event callback type.
**/
enum eCAL_eServerEvent
{
  eCAL_eServerEvent_none,        //!< undefined
  eCAL_eServerEvent_connected,   //!< a new client has been connected to this server
  eCAL_eServerEvent_disconnected //!< a client has been disconnected from this server
};

/**
 * @brief eCAL client event callback struct.
**/
struct eCAL_SServerEventCallbackData
{
  enum eCAL_eServerEvent type; //!< event type
  long long time;              //!< event time in µs
};

/**
 * @brief Server event callback function type.
 *
 * @param service_id_  The service id struct of the connection that triggered the event.
 * @param data_        Event callback data structure with the event specific information.
**/
typedef void (*eCAL_ServerEventCallbackT)(const struct eCAL_SServiceId* /*service_id_*/, const struct eCAL_SServerEventCallbackData* /*data_*/);

/**
 * @brief Service method information struct containing the method name, the request and response type information.
 *        This type is used when creating services (servers or clients), or when querying information about them from the registration layer.
**/
struct eCAL_SServiceMethodInformation
{
  const char* method_name;                         //!< The name of the method.
  struct eCAL_SDataTypeInformation request_type;   //!< The type of the method request.
  struct eCAL_SDataTypeInformation response_type;  //!< The type of the method response.
};

/**
   * @brief Service response struct containing the (responding) server information and the response itself.
  **/
struct eCAL_SServiceResponse
{
  enum eCAL_eCallState call_state;                                   //!< call state, to indicate if the call was successful or not
  struct eCAL_SServiceId server_id;                                  //!< Id of the server that executed the service (server id (entity id, process id, host name), name)
  struct eCAL_SServiceMethodInformation service_method_information;  //!< Additional Information about the method that has been called (name & DatatypeInformation of request and reponse)
  int ret_state;                                                     //!< return state of the called service method 
  const void* response;                                              //!< the actual response data of the service call
  size_t response_length;                                            //!< length of response data
  const char* error_msg;                                             //!< human readable error message, in case that the service call could not be executed.
};

/**
 * @brief Service response callback function type (low level client interface).
 *
 * @param service_response_  Service response struct containing the (responding) server informations and the response itself.
 * @param user_argument_     User argument that was forwarded by a SetCallback() function.
 **/
typedef void (*eCAL_ResponseCallbackT)(const struct eCAL_SServiceResponse* /*service_response_*/, void* /*user_argument_*/);

/**
 * @brief Service method callback function type (low level server interface).
 *        This is the type definition of a function that can be registered for a CServiceServer.
 *        It callback is then called, a client
 *
 * @param      method_info       The method information struct containing the request and response type information.
 * @param      request_          The request.
 * @param      request_length    Length of request
 * @param[out] response_         Response data which needs to be set and allocated by eCAL_Malloc() from user space.
 * @param[out] response_length_  Length of response data set from the user space.
 * @param      user_argument_    User argument that was forwarded by a SetCallback() function.
**/
typedef int (*eCAL_ServiceMethodCallbackT)(const struct eCAL_SServiceMethodInformation* /*method_info*/, const void* /*request_*/, size_t /*request_length*/, void** /*response_*/, size_t* /*response_length_*/, void* /*user_argument_*/);
#endif /*ecal_c_service_types_h_included*/