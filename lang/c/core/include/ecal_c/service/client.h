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
 * @file   ecal_c/service/client.h
 * @brief  eCAL client c interface
**/

#ifndef ecal_c_service_client_h_included
#define ecal_c_service_client_h_included

#include <ecal_c/export.h>
#include <ecal_c/types.h>
#include <ecal_c/service/types.h>
#include <ecal_c/service/client_instance.h>

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif /*__cplusplus*/
  typedef struct eCAL_ServiceClient eCAL_ServiceClient;

  /**
   * @brief Creates a new client instance
   *
   * @param service_name_                  Unique service name.
   * @param method_information_set_        Set of method names and corresponding datatype information as array. Optional, can be NULL.
   * @param method_information_set_length_ Length of the method information set. Optional, can be 0.
   * @param event_callback_                The client event callback funtion. Optional, can be NULL.
   * 
   * @return Client handle if succeeded, otherwise NULL. The handle needs to be released by eCAL_ServiceClient_Delete().
  **/
  ECALC_API eCAL_ServiceClient* eCAL_ServiceClient_New(const char* service_name_, const struct eCAL_SServiceMethodInformation* method_information_set_, size_t method_information_set_length_, eCAL_ClientEventCallbackT event_callback_);
  
  /**
   * @brief Deletes a client instance
   *
   * @param service_client_  Client handle.
  **/
  ECALC_API void eCAL_ServiceClient_Delete(eCAL_ServiceClient* service_client_);

  /**
   * @brief Get the client instances for all matching services
   * 
   * @param service_client_  Client handle.
   *
   * @return Pointer array of client instance handles if succeeded, NULL otherwise. The handles are stored in a null pointer terminated array and have to be deleted by eCAL_ClientInstances_Delete().
  **/
  ECALC_API eCAL_ClientInstance** eCAL_ServiceClient_GetClientInstances(eCAL_ServiceClient* service_client_);

  /**
   * @brief Blocking call of a service method for all existing service instances, response will be returned as an array of struct eCAL_SServiceResponse
   *
   * @param       service_client_               Client handle.
   * @param       method_name_                  Method name.
   * @param       request_                      Request data.
   * @param       request_length_               Length of requested data.
   * @param [out] service_response_vec_         Returned array of service responses from every called service. Must point to NULL and needs to be released by eCAL_Free().
   * @param [out] service_response_vec_length_  Returned length of response array. Must point to zero.
   * @param       timeout_ms_                   Maximum time before operation returns. Optional, can be NULL.
   *
   * @return Zero if all calls were successful and minimum one instance was connected, non-zero otherwise.
  **/
  ECALC_API int eCAL_ServiceClient_CallWithResponse(eCAL_ServiceClient* service_client_, const char* method_name_, const void* request_, size_t request_length_, struct eCAL_SServiceResponse** service_response_vec_, size_t* service_response_vec_length_, const int* timeout_ms_);
  
  /**
   * @brief Blocking call (with timeout) of a service method for all existing service instances, using callback
   *
   * @param service_client_          Client handle.
   * @param method_name_             Method name.
   * @param request_                 Request data.
   * @param request_length_          Length of requested data.
   * @param callback_                Callback function for the service method response.
   * @param callback_user_argument_  User argument that is forwarded to the callback. Optional, can be NULL.
   * @param timeout_ms_              Maximum time before operation returns. Optional, can be NULL.
   *
   * @return Zero if all calls were successful and minimum one instance was connected, non-zero otherwise.
  **/
  ECALC_API int eCAL_ServiceClient_CallWithCallback(eCAL_ServiceClient* service_client_, const char* method_name_, const void* request_, size_t request_length_, eCAL_ResponseCallbackT callback_, void* callback_user_argument_, const int* timeout_ms_);
  
  /**
   * @brief Asynchronous call of a service method for all existing service instances, using callback
   *
   * @param service_client_          Client handle.
   * @param method_name_             Method name.
   * @param request_                 Request data.
   * @param request_length_          Length of requested data.
   * @param callback_                Callback function for the service method response.
   * @param callback_user_argument_  User argument that is forwarded to the callback. Optional, can be NULL.
   *
   * @return Zero if all calls were successful and minimum one instance was connected, non-zero otherwise.
  **/
  ECALC_API int eCAL_ServiceClient_CallWithCallbackAsync(eCAL_ServiceClient* service_client_, const char* method_name_, const void* request_, size_t request_length_, eCAL_ResponseCallbackT callback_, void* callback_user_argument_);

  /**
   * @brief Retrieve service name.
   * 
   * @param service_client_  Client handle.
   *
   * @return The service name.
  **/
  ECALC_API const char* eCAL_ServiceClient_GetServiceName(eCAL_ServiceClient* service_client_);

  /**
   * @brief Retrieve the service id.
   *
   * @param service_client_  Client handle.
   * 
   * @return The service id.
  **/
  ECALC_API const struct eCAL_SServiceId* eCAL_ServiceClient_GetServiceId(eCAL_ServiceClient* service_client_);

  /**
   * @brief Check connection to at least one service.
   *
   * @param service_client_  Client handle.
   * 
   * @return Non-zero if at least one service client instance is connected, zero otherwise.
  **/
  ECALC_API int eCAL_ServiceClient_IsConnected(eCAL_ServiceClient* service_client_);
#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*ecal_c_service_client_h_included*/
