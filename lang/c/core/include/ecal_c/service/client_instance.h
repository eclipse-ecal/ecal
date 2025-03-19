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
  * @file   ecal_c/service/client_instance.h
  * @brief  eCAL client instance c interface
 **/

#ifndef ecal_c_service_client_instance_h_included
#define ecal_c_service_client_instance_h_included

#include <ecal_c/export.h>
#include <ecal_c/types.h>
#include <ecal_c/service/types.h>

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif /*__cplusplus*/
  typedef struct eCAL_ClientInstance eCAL_ClientInstance;

  /**
   * @brief Deletes an array of client instances
   *
   * @param client_instances_  Client instance handles.
  **/
  ECALC_API void eCAL_ClientInstances_Delete(eCAL_ClientInstance** client_instances_);

  /**
   * @brief Blocking call of a service method for one service instances. Response will be returned as struct eCAL_SServiceResponse.
   *
   * @param       client_instance_  Client instance handle.
   * @param       method_name_      Method name.
   * @param       request_          Request data.
   * @param       request_length_   Length of requested data.
   * @param       timeout_ms_       Maximum time before operation returns. Optional, can be NULL.
   *
   * @return Pointer to service response if succeeded, NULL otherwise. Needs to be released by eCAL_Free().
  **/
  ECALC_API struct eCAL_SServiceResponse* eCAL_ClientInstance_CallWithResponse(eCAL_ClientInstance* client_instance_, const char* method_name_, const void* request_, size_t request_length_, const int* timeout_ms_);
  
  /**
   * @brief Blocking call (with timeout) of a service method for on service instances, using callback
   *
   * @param client_instance_    Client instance handle.
   * @param method_name_        Method name.
   * @param request_            Request data.
   * @param request_length_     Length of requested data.
   * @param response_callback_  Callback function for the service method response.
   * @param user_argument_      User argument that is forwarded to the callback. Optional, can be NULL.
   * @param timeout_ms_         Maximum time before operation returns. Optional, can be NULL.
   *
   * @return Zero if succeeded, non-zero otherwise.
  **/
  ECALC_API int eCAL_ClientInstance_CallWithCallback(eCAL_ClientInstance* client_instance_, const char* method_name_, const void* request_, size_t request_length_, eCAL_ResponseCallbackT response_callback_, void* user_argument_, const int* timeout_ms_);
  
  /**
   * @brief Asynchronous call of a service method for one service instances, using callback
   *
   * @param client_instance_    Client instance handle.
   * @param method_name_        Method name.
   * @param request_            Request data.
   * @param request_length_     Length of requested data.
   * @param response_callback_  Callback function for the service method response.
   * @param user_argument_      User argument that is forwarded to the callback. Optional, can be NULL.
   *
   * @return Zero if succeeded, non-zero otherwise.
  **/
  ECALC_API int eCAL_ClientInstance_CallWithCallbackAsync(eCAL_ClientInstance* client_instance_, const char* method_name_, const void* request_, size_t request_length_, eCAL_ResponseCallbackT response_callback_, void* user_argument);

  /**
   * @brief Check connection state.
   *
   * @param client_instance_  Client instance handle.
   *
   * @return Non-zero if connected, zero otherwise.
  **/
  ECALC_API int eCAL_ClientInstance_IsConnected(eCAL_ClientInstance* client_instance_);

  /**
   * @brief Get unique client entity id.
   *
   * @param client_instance_  Client instance handle.
   * 
   * @return  The client entity id.
  **/
  ECALC_API const struct eCAL_SEntityId* eCAL_ClientInstance_GetClientID(eCAL_ClientInstance* client_instance_);
#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*ecal_c_service_client_instance_h_included*/
