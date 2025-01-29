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

#include <ecal_c/service/service_info.h>
#include <ecal_c/callback.h>

#ifdef __cplusplus
extern "C"
{
#endif /*__cplusplus*/
  /**
   * @brief Create a server. 
   *
   * @param service_name_  Service name.
   *
   * @return  Handle to created server or NULL if failed.
  **/
  ECALC_API ECAL_HANDLE eCAL_Server_Create(const char* service_name_);

  /**
   * @brief Destroy a server. 
   *
   * @param handle_  Server handle. 
   *
   * @return  None zero if succeeded.
  **/
  ECALC_API int eCAL_Server_Destroy(ECAL_HANDLE handle_);

  /**
   * @brief Add a server method callback.
   * @since eCAL 5.10.0   
   *
   * @param handle_     Server handle.
   * @param method_     Service method name.
   * @param req_type_   Method request type (default = "").
   * @param resp_type_  Method response type (default = "").
   * @param callback_   Callback function for server request.
   * @param par_        User defined context that will be forwarded to the request function.
   *
   * @return  None zero if succeeded.
  **/
  ECALC_API int eCAL_Server_AddMethodCallback(ECAL_HANDLE handle_, const char* method_, const char* req_type_, const char* resp_type_, MethodCallbackCT callback_, void * par_);

  /**
   * @brief Remove a server method callback.
   * @since eCAL 5.10.0   
   *
   * @param handle_  Server handle.
   * @param method_  Service method name.
   *
   * @return  None zero if succeeded.
  **/
  ECALC_API int eCAL_Server_RemMethodCallback(ECAL_HANDLE handle_, const char* method_);

  /**
   * @brief Add server event callback function.
   *
   * @param handle_    Server handle.
   * @param type_      The event type to react on.
   * @param callback_  The callback function to add.
   * @param par_       User defined context that will be forwarded to the callback function.
   *
   * @return  True if succeeded, false if not.
  **/
  ECALC_API int eCAL_Server_AddEventCallback(ECAL_HANDLE handle_, enum eCAL_Server_Event type_, ServerEventCallbackCT callback_, void* par_);

  /**
   * @brief Remove server event callback function.
   *
   * @param handle_  Server handle.
   * @param type_    The event type to remove.
   *
   * @return  True if succeeded, false if not.
  **/
  ECALC_API int eCAL_Server_RemEventCallback(ECAL_HANDLE handle_, enum eCAL_Server_Event type_);
    
  /**
   * @brief Retrieve the service name.
   *
   * @param       handle_   Server handle.
   * @param [out] buf_      Pointer to store the server service string.
   * @param       buf_len_  Length of allocated buffer or ECAL_ALLOCATE_4ME if
   *                        eCAL should allocate the buffer for you (see eCAL_FreeMem).
   *
   * @return  Description buffer length or zero if failed.
  **/
  ECALC_API int eCAL_Server_GetServiceName(ECAL_HANDLE handle_, void* buf_, int buf_len_);

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*ecal_c_service_server_h_included*/
