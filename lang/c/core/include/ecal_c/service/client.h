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
  typedef struct eCAL_ServiceClient eCAL_ServiceClient;
  typedef struct eCAL_ClientInstance eCAL_ClientInstance;



  enum eCAL_eClientEvent
  {
    eCAL_eClientEvent_none,
    eCAL_eClientEvent_connected,
    eCAL_eClientEvent_disconnected,
    eCAL_eClientEvent_timeout
  };

  enum eCAL_eCallState
  {
    eCAL_eCallState_none,
    eCAL_eCallState_executed,
    eCAL_eCallState_timeouted,
    eCAL_eCallState_failed
  };

  struct eCAL_SClientEventCallbackData
  {
    enum eCAL_eClientEvent type;
    long long time;
  };

  struct eCAL_SServiceResponse
  {
    enum eCAL_eCallState call_state;
    struct eCAL_SServiceId server_id;
    struct eCAL_SServiceMethodInformation service_method_information;
    int ret_state;
    const void* response;
    size_t response_length;
    const char* error_msg;
  };

  struct eCAL_SServiceResponseResult
  {
    int result;
    struct eCAL_SServiceResponse service_response;
  };

  typedef void (*eCAL_ClientEventCallbackT)(const struct eCAL_SServiceId*, const struct eCAL_SClientEventCallbackData*);
  typedef void (*eCAL_ResponseCallbackT)(const struct eCAL_SServiceResponse*);
  

  ECALC_API eCAL_ServiceClient* eCAL_ServiceClient_New(const char* service_name_, const struct eCAL_SServiceMethodInformation* method_information_set_, size_t method_information_set_length_, eCAL_ClientEventCallbackT event_callback_);
  ECALC_API void eCAL_ServiceClient_Delete(eCAL_ServiceClient* service_client_);

  ECALC_API eCAL_ClientInstance** eCAL_ServiceClient_GetClientInstances(eCAL_ServiceClient* service_client_);
  ECALC_API int eCAL_ServiceClient_CallWithResponse(eCAL_ServiceClient* service_client_, const char* method_name_, const void* request_, size_t request_length_, struct eCAL_SServiceResponse** service_response_vec_, size_t* service_response_vec_length_, int timeout_ms_);
  ECALC_API int eCAL_ServiceClient_CallWithCallback(eCAL_ServiceClient* service_client_, const char* method_name_, const void* request_, size_t request_length_, eCAL_ResponseCallbackT response_callback_, int timeout_ms_);
  ECALC_API int eCAL_ServiceClient_CallWithCallbackAsync(eCAL_ServiceClient* service_client_, const char* method_name_, const void* request_, size_t request_length_, eCAL_ResponseCallbackT response_callback_);

  ECALC_API const char* eCAL_ServiceClient_GetServiceName(eCAL_ServiceClient* service_client_);

  ECALC_API struct eCAL_SServiceId* eCAL_ServiceClient_GetServiceId(eCAL_ServiceClient* service_client_);
  ECALC_API void eCAL_SServiceId_Free(struct eCAL_SServiceId* service_id_);
  
  ECALC_API int eCAL_ServiceClient_IsConnected(eCAL_ServiceClient* service_client_);

  ECALC_API void eCAL_ClientInstance_Delete(eCAL_ClientInstance* client_instance_);
  ECALC_API void eCAL_ClientInstances_Delete(eCAL_ClientInstance** client_instances_);
  
  ECALC_API struct eCAL_SServiceResponseResult* eCAL_ClientInstance_CallWithResponse(eCAL_ClientInstance* client_instance_, const char* method_name_, const void* request_, size_t request_length_, int timeout_ms_);
  ECALC_API int eCAL_ClientInstance_CallWithCallback(eCAL_ClientInstance* client_instance_, const char* method_name_, const void* request_, size_t request_length_, eCAL_ResponseCallbackT response_callback_, int timeout_ms_);
  ECALC_API int eCAL_ClientInstance_CallWithCallbackAsync(eCAL_ClientInstance* client_instance_, const char* method_name_, const void* request_, size_t request_length_, eCAL_ResponseCallbackT response_callback_);

  ECALC_API int eCAL_ClientInstance_IsConnected(eCAL_ClientInstance* client_instance_);

  ECALC_API struct eCAL_SEntityId* eCAL_ClientInstance_GetClientID(eCAL_ClientInstance* client_instance_);
  ECALC_API void eCAL_SEntityId_Free(struct eCAL_SEntityId* entity_id_);

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*ecal_c_service_client_h_included*/
