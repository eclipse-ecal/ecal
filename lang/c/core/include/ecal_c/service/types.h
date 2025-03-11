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

struct eCAL_SServiceId
{
  struct eCAL_SEntityId service_id;
  const char* service_name;
};

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

typedef void (*eCAL_ClientEventCallbackT)(const struct eCAL_SServiceId*, const struct eCAL_SClientEventCallbackData*);

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

struct eCAL_SServiceMethodInformation
{
  const char* method_name;
  struct eCAL_SDataTypeInformation request_type;
  struct eCAL_SDataTypeInformation response_type;
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

typedef void (*eCAL_ResponseCallbackT)(const struct eCAL_SServiceResponse*);

typedef int (*eCAL_ServiceMethodCallbackT)(const struct eCAL_SServiceMethodInformation*, const void*, size_t, void**, size_t*);
#endif /*ecal_c_service_types_h_included*/