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
 * @file   ecal_c/service/server.h
 * @brief  eCAL service c interface
**/

#ifndef ecal_c_service_server_h_included
#define ecal_c_service_server_h_included

#include <ecal_c/export.h>
#include <ecal_c/types.h>
#include <ecal_c/service/types.h>

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif /*__cplusplus*/
  typedef struct eCAL_ServiceServer eCAL_ServiceServer;

  /**
   * @brief Creates a new server instance
   *
   * @param service_name_   Unique service name.
   * @param event_callback_ Callback function for server events. Optional, can be NULL.
   * 
   * @return Server handle if succeeded, NULL otherwise. The handle needs to be released by eCAL_ServiceServer_Delete().
  **/
  ECALC_API eCAL_ServiceServer* eCAL_ServiceServer_New(const char* service_name_, eCAL_ServerEventCallbackT event_callback_);

  /**
   * @brief Deletes a server instance
   *
   * @param service_server_  Server handle.
  **/
  ECALC_API void eCAL_ServiceServer_Delete(eCAL_ServiceServer* service_server_);

  /**
   * @brief Set/overwrite a method callback, that will be invoked, when a connected client is making a service call.
   *
   * @param service_server_  Server handle.
   * @param method_info_     Service method information (method name, request & response types).
   * @param callback_        Callback function for client request.
   *
   * @return Zero if succeeded, non-zero otherwise.
  **/
  ECALC_API int eCAL_ServiceServer_SetMethodCallback(eCAL_ServiceServer* service_server_, const struct eCAL_SServiceMethodInformation* method_info_, eCAL_ServiceMethodCallbackT callback_);
  
  /**
   * @brief Remove method callback.
   *
   * @param service_server_  Server handle.
   * @param method_          Service method name.
   *
   * @return Zero if succeeded, non-zero otherwise.
  **/
  ECALC_API int eCAL_ServiceServer_RemoveMethodCallback(eCAL_ServiceServer* service_server_, const char* method_name_);

  /**
   * @brief Retrieve service name.
   *
   * @param service_server_  Server handle.
   *
   * @return The service name.
  **/
  ECALC_API const char* eCAL_ServiceServer_GetServiceName(eCAL_ServiceServer* service_server_);

  /**
   * @brief Retrieve the service id.
   *
   * @param service_server_  Server handle.
   *
   * @return The service id.
  **/
  ECALC_API const struct eCAL_SServiceId* eCAL_ServiceServer_GetServiceId(eCAL_ServiceServer* service_server_);

  /**
   * @brief Check connection state.
   *
   * @param service_server_  Server handle.
   *
   * @return Non-zero if connected, zero otherwise.
  **/
  ECALC_API int eCAL_ServiceServer_IsConnected(eCAL_ServiceServer* service_server_);
#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*ecal_c_service_server_h_included*/
