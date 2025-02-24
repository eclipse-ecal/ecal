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
  typedef struct eCAL_ServiceServer eCAL_ServiceServer;

  enum eCAL_eServerEvent
  {
    eCAL_eServerEvent_none,
    eCAL_eServerEvent_connected,
    eCAL_eServerEvent_disconnected
  };

  struct eCAL_SServerEventCallbackData
  {
    enum eCAL_eServerEvent type;
    long long time;
  };

  typedef void (*eCAL_ServerEventCallbackT)(const struct eCAL_SServiceId*, const struct eCAL_SServerEventCallbackData*);
  typedef int (*eCAL_ServiceMethodCallbackT)(const struct eCAL_SServiceMethodInformation*, const void*, size_t, void**, size_t*);

  ECALC_API eCAL_ServiceServer* eCAL_ServiceServer_New(const char* service_name_, eCAL_ServerEventCallbackT event_callback_);
  ECALC_API void eCAL_ServiceServer_Delete(eCAL_ServiceServer* service_server_);

  ECALC_API int eCAL_ServiceServer_SetMethodCallback(eCAL_ServiceServer* service_server_, const struct eCAL_SServiceMethodInformation* method_info_, eCAL_ServiceMethodCallbackT callback_);
  ECALC_API int eCAL_ServiceServer_RemoveMethodCallback(eCAL_ServiceServer* service_server_, const char* method_name_);

  ECALC_API const char* eCAL_ServiceServer_GetServiceName(eCAL_ServiceServer* service_server_);
  ECALC_API struct eCAL_SServiceId* eCAL_ServiceServer_GetServiceId(eCAL_ServiceServer* service_server_);
  ECALC_API int eCAL_ServiceServer_IsConnected(eCAL_ServiceServer* service_server_);

  //ECALC_API void eCAL_SServiceId_Free(struct eCAL_SServiceId* service_id_);

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*ecal_c_service_server_h_included*/
