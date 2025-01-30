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

#include <ecal_c/callback.h>
#include <ecal_c/service/service_info.h>

#ifdef __cplusplus
extern "C"
{
#endif /*__cplusplus*/
  /**
   * @brief Create a client. 
   *
   * @param service_name_  Service name. 
   *
   * @return  Handle to created client or NULL if failed.
  **/
  ECALC_API ECAL_HANDLE eCAL_Client_Create(const char* service_name_);

  /**
   * @brief Destroy a client. 
   *
   * @param handle_  Client handle. 
   *
   * @return  None zero if succeeded.
  **/
  ECALC_API int eCAL_Client_Destroy(ECAL_HANDLE handle_);

  /**
   * @brief Change the host name filter for that client instance
   *
   * @param handle_     Client handle.
   * @param host_name_  Host name filter (empty == all hosts)
   *
   * @return  None zero if succeeded.
  **/
  ECALC_API int eCAL_Client_SetHostName(ECAL_HANDLE handle_, const char* host_name_);

  /**
   * @brief Call a method of this service (none blocking variant with callback). 
   *
   * @param handle_       Client handle. 
   * @param method_name_  Method name.
   * @param request_      Request message buffer. 
   * @param request_len_  Request message length. 
   * @param timeout_      Maximum time before operation returns (in milliseconds, -1 means infinite).
   *
   * @return  None zero if succeeded.
  **/
  ECALC_API int eCAL_Client_Call(ECAL_HANDLE handle_, const char* method_name_, const char* request_, int request_len_, int timeout_);

  /**
   * @brief Call a method of this service (blocking variant). 
   *
   * @param       handle_            Client handle. 
   * @param       method_name_       Method name.
   * @param       request_           Request message buffer. 
   * @param       request_len_       Request message length. 
   * @param       timeout_           Maximum time before operation returns (in milliseconds, -1 means infinite).
   * @param [out] service_response_  Service response struct with additional infos like call state and
   *                                 error message.
   * @param [out] response_          Pointer to the allocated buffer for the response message.
   *                                 In case of more then one connected server, only the reponse of the first
   *                                 answering server is returned (use callback variant to get all responses).
   * @param       response_len_      Response message buffer length or ECAL_ALLOCATE_4ME if
   *                                 eCAL should allocate the buffer for you (see eCAL_FreeMem). 
   *
   * @return  Size of response buffer if succeeded, otherwise zero.
  **/
  ECALC_API int eCAL_Client_Call_Wait(ECAL_HANDLE handle_, const char* method_name_, const char* request_, int request_len_, int timeout_, struct SServiceResponseC* service_response_, void* response_, int response_len_);

  /**
   * @brief Call a method of this service (asynchronously with callback).
   *
   * @param handle_       Client handle.
   * @param method_name_  Method name.
   * @param request_      Request message buffer.
   * @param request_len_  Request message length.
   * @param timeout_      Maximum time before operation returns (in milliseconds, -1 means infinite).
   *
   * @return  None zero if succeeded.
  **/
  ECALC_API int eCAL_Client_Call_Async(ECAL_HANDLE handle_, const char* method_name_, const char* request_, int request_len_, int timeout_);

 /**
   * @brief Add server response callback. 
   * @since eCAL 5.10.0
   *
   * @param handle_    Client handle. 
   * @param callback_  Callback function for server response.  
   * @param par_       User defined context that will be forwarded to the callback function.  
   *
   * @return  None zero if succeeded.
  **/
  ECALC_API int eCAL_Client_AddResponseCallback(ECAL_HANDLE handle_, ResponseCallbackCT callback_, void* par_);

  /**
   * @brief Remove server response callback. 
   *
   * @param handle_  Client handle. 
   *
   * @return  None zero if succeeded.
  **/
  ECALC_API int eCAL_Client_RemResponseCallback(ECAL_HANDLE handle_);

  /**
   * @brief Add client event callback function.
   *
   * @param handle_    Client handle.
   * @param type_      The event type to react on.
   * @param callback_  The callback function to add.
   * @param par_       User defined context that will be forwarded to the callback function.
   *
   * @return  True if succeeded, false if not.
  **/
  ECALC_API int eCAL_Client_AddEventCallback(ECAL_HANDLE handle_, enum eCAL_Client_Event type_, ClientEventCallbackCT callback_, void* par_);

  /**
   * @brief Remove client event callback function.
   *
   * @param handle_  Client handle.
   * @param type_    The event type to remove.
   *
   * @return  True if succeeded, false if not.
  **/
  ECALC_API int eCAL_Client_RemEventCallback(ECAL_HANDLE handle_, enum eCAL_Client_Event type_);

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
  ECALC_API int eCAL_Client_GetServiceName(ECAL_HANDLE handle_, void* buf_, int buf_len_);

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*ecal_c_service_client_h_included*/
