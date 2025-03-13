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

#ifdef __cplusplus
extern "C"
{
#endif /*__cplusplus*/
  typedef struct eCAL_ServiceClient eCAL_ServiceClient;

  ECALC_API eCAL_ServiceClient* eCAL_ServiceClient_New(const char* service_name_, const struct eCAL_SServiceMethodInformation* method_information_set_, size_t method_information_set_length_, eCAL_ClientEventCallbackT event_callback_);
  ECALC_API void eCAL_ServiceClient_Delete(eCAL_ServiceClient* service_client_);

  ECALC_API eCAL_ClientInstance** eCAL_ServiceClient_GetClientInstances(eCAL_ServiceClient* service_client_);

  ECALC_API int eCAL_ServiceClient_CallWithResponse(eCAL_ServiceClient* service_client_, const char* method_name_, const void* request_, size_t request_length_, struct eCAL_SServiceResponse** service_response_vec_, size_t* service_response_vec_length_, const int* timeout_ms_);
  ECALC_API int eCAL_ServiceClient_CallWithCallback(eCAL_ServiceClient* service_client_, const char* method_name_, const void* request_, size_t request_length_, eCAL_ResponseCallbackT response_callback_, const int* timeout_ms_);
  ECALC_API int eCAL_ServiceClient_CallWithCallbackAsync(eCAL_ServiceClient* service_client_, const char* method_name_, const void* request_, size_t request_length_, eCAL_ResponseCallbackT response_callback_);

  ECALC_API const char* eCAL_ServiceClient_GetServiceName(eCAL_ServiceClient* service_client_);

  ECALC_API const struct eCAL_SServiceId* eCAL_ServiceClient_GetServiceId(eCAL_ServiceClient* service_client_);

  ECALC_API int eCAL_ServiceClient_IsConnected(eCAL_ServiceClient* service_client_);
#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*ecal_c_service_client_h_included*/
