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

#ifdef __cplusplus
extern "C"
{
#endif /*__cplusplus*/
  typedef struct eCAL_ClientInstance eCAL_ClientInstance;

  struct eCAL_SServiceResponseResult
  {
    int result;
    struct eCAL_SServiceResponse service_response;
  };

  ECALC_API void eCAL_ClientInstance_Delete(eCAL_ClientInstance* client_instance_);
  ECALC_API void eCAL_ClientInstances_Delete(eCAL_ClientInstance** client_instances_);

  ECALC_API struct eCAL_SServiceResponseResult* eCAL_ClientInstance_CallWithResponse(eCAL_ClientInstance* client_instance_, const char* method_name_, const void* request_, size_t request_length_, int timeout_ms_);
  ECALC_API int eCAL_ClientInstance_CallWithCallback(eCAL_ClientInstance* client_instance_, const char* method_name_, const void* request_, size_t request_length_, eCAL_ResponseCallbackT response_callback_, int timeout_ms_);
  ECALC_API int eCAL_ClientInstance_CallWithCallbackAsync(eCAL_ClientInstance* client_instance_, const char* method_name_, const void* request_, size_t request_length_, eCAL_ResponseCallbackT response_callback_);

  ECALC_API int eCAL_ClientInstance_IsConnected(eCAL_ClientInstance* client_instance_);

  ECALC_API struct eCAL_SEntityId* eCAL_ClientInstance_GetClientID(eCAL_ClientInstance* client_instance_);
#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*ecal_c_service_client_instance_h_included*/
