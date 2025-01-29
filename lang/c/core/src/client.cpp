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
 * @file   client.cpp
 * @brief  eCAL client c interface
**/

#include <ecal/ecal.h>
#include <ecal/v5/ecal_client.h>
#include <ecal_c/service/client.h>
#include <ecal/service/types.h>

#include "common.h"

#include <mutex>

#if ECAL_CORE_SERVICE
namespace
{
  eCallState enum_class_to_enum(eCAL::eCallState cpp_call_state_)
  {
    switch (cpp_call_state_)
    {
    case eCAL::eCallState::none:      return call_state_none;
    case eCAL::eCallState::executed:  return call_state_executed;
    case eCAL::eCallState::timeouted: return call_state_timeouted;
    case eCAL::eCallState::failed:    return call_state_failed;
    default: return call_state_none;
    }
  }

  eCAL_Client_Event enum_class_to_enum(eCAL::eClientEvent cpp_event_)
  {
    switch (cpp_event_)
    {
    case eCAL::eClientEvent::none:         return client_event_none;
    case eCAL::eClientEvent::connected:    return client_event_connected;
    case eCAL::eClientEvent::disconnected: return client_event_disconnected;
    case eCAL::eClientEvent::timeout:      return client_event_timeout;
    default:                                            return client_event_none;
    }
  }

  eCAL::eClientEvent enum_to_enum_class(eCAL_Client_Event c_event)
  {
    switch (c_event)
    {
    case client_event_none:         return eCAL::eClientEvent::none;
    case client_event_connected:    return eCAL::eClientEvent::connected;
    case client_event_disconnected: return eCAL::eClientEvent::disconnected;
    case client_event_timeout:      return eCAL::eClientEvent::timeout;
    default:                        return eCAL::eClientEvent::none;
    }
  }


  std::recursive_mutex g_response_callback_mtx; // NOLINT(*-avoid-non-const-global-variables)
  void g_response_callback(const struct eCAL::v5::SServiceResponse& service_response_, const ResponseCallbackCT callback_, void* par_)
  {
    const std::lock_guard<std::recursive_mutex> lock(g_response_callback_mtx);
    struct SServiceResponseC service_response {};
    service_response.host_name    = service_response_.host_name.c_str();
    service_response.service_name = service_response_.service_name.c_str();
    service_response.service_id   = service_response_.service_id.c_str();
    service_response.method_name  = service_response_.method_name.c_str();
    service_response.error_msg    = service_response_.error_msg.c_str();
    service_response.ret_state    = service_response_.ret_state;
    service_response.call_state   = enum_class_to_enum(service_response_.call_state);
    service_response.response     = service_response_.response.c_str();
    service_response.response_len = static_cast<int>(service_response_.response.size());
    callback_(&service_response, par_);
  }

  std::recursive_mutex g_client_event_callback_mtx; // NOLINT(*-avoid-non-const-global-variables)
  void g_client_event_callback(const char* name_, const struct eCAL::v5::SClientEventCallbackData* data_, const ClientEventCallbackCT callback_, void* par_)
  {
    const std::lock_guard<std::recursive_mutex> lock(g_client_event_callback_mtx);
    SClientEventCallbackDataC data{};
    data.time = data_->time;
    data.type = enum_class_to_enum(data_->type);
    callback_(name_, &data, par_);
  }
}

extern "C"
{
  ECALC_API ECAL_HANDLE eCAL_Client_Create(const char* service_name_)
  {
    if (service_name_ == nullptr) return(nullptr);
    auto* client = new eCAL::v5::CServiceClient(service_name_);
    return(client);
  }

  ECALC_API int eCAL_Client_Destroy(ECAL_HANDLE handle_)
  {
    if (handle_ == nullptr) return(0);
    auto* client = static_cast<eCAL::v5::CServiceClient*>(handle_);
    delete client; // NOLINT(*-owning-memory)
    return(1);
  }

  ECALC_API int eCAL_Client_SetHostName(ECAL_HANDLE handle_, const char* host_name_)
  {
    if (handle_ == nullptr) return(0);
    auto* client = static_cast<eCAL::v5::CServiceClient*>(handle_);
    if (client->SetHostName(host_name_)) return(1);
    return(0);
  }

  ECALC_API int eCAL_Client_Call(ECAL_HANDLE handle_, const char* method_name_, const char* request_, int request_len_, int timeout_)
  {
    if (handle_ == nullptr) return(0);
    auto* client = static_cast<eCAL::v5::CServiceClient*>(handle_);
    if (client->Call(method_name_, std::string(request_, static_cast<size_t>(request_len_)), timeout_)) return(1);
    return(0);
  }

  // The C API variant is not able to return all service repsonses but only the first one !
  ECALC_API int eCAL_Client_Call_Wait(ECAL_HANDLE handle_, const char* method_name_, const char* request_, int request_len_, int timeout_, struct SServiceResponseC* service_response_, void* response_, int response_len_)
  {
    if (handle_ == nullptr) return(0);
    auto* client = static_cast<eCAL::v5::CServiceClient*>(handle_);
    eCAL::v5::ServiceResponseVecT service_response_vec;
    if (client->Call(method_name_, std::string(request_, static_cast<size_t>(request_len_)), timeout_, &service_response_vec))
    {
      if (!service_response_vec.empty())
      {
        service_response_->host_name    = nullptr;
        service_response_->service_name = nullptr;
        service_response_->service_id   = nullptr;
        service_response_->method_name  = nullptr;
        service_response_->error_msg    = nullptr;
        service_response_->ret_state    = service_response_vec[0].ret_state;
        service_response_->call_state   = enum_class_to_enum(service_response_vec[0].call_state);
        service_response_->response     = nullptr;
        service_response_->response_len = 0;
        return(CopyBuffer(response_, response_len_, service_response_vec[0].response));
      }
    }
    return(0);
  }
}

ECALC_API int eCAL_Client_Call_Async(ECAL_HANDLE handle_, const char* method_name_, const char* request_, int request_len_, int timeout_)
{
  if (handle_ == nullptr) return(0);
  auto* client = static_cast<eCAL::v5::CServiceClient*>(handle_);
  if (client->CallAsync(method_name_, std::string(request_, static_cast<size_t>(request_len_)), timeout_)) return(1);
  return(0);
}

int eCAL_Client_AddResponseCallback(ECAL_HANDLE handle_, ResponseCallbackCT callback_, void* par_)
{
  if (handle_ == nullptr) return(0);
  auto* client = static_cast<eCAL::v5::CServiceClient*>(handle_);
  auto callback = std::bind(g_response_callback, std::placeholders::_1, callback_, par_);
  return static_cast<int>(client->AddResponseCallback(callback));
}

int eCAL_Client_RemResponseCallback(ECAL_HANDLE handle_)
{
  if (handle_ == nullptr) return(0);
  auto* client = static_cast<eCAL::v5::CServiceClient*>(handle_);
  return static_cast<int>(client->RemResponseCallback());
}

ECALC_API int eCAL_Client_AddEventCallback(ECAL_HANDLE handle_, enum eCAL_Client_Event type_, ClientEventCallbackCT callback_, void* par_)
{
  if (handle_ == nullptr) return(0);
  auto* client = static_cast<eCAL::v5::CServiceClient*>(handle_);
  auto callback = std::bind(g_client_event_callback, std::placeholders::_1, std::placeholders::_2, callback_, par_);
  if (client->AddEventCallback(enum_to_enum_class(type_), callback)) return(1);
  return(0);
}

ECALC_API int eCAL_Client_RemEventCallback(ECAL_HANDLE handle_, enum eCAL_Client_Event type_)
{
  if (handle_ == nullptr) return(0);
  auto* client = static_cast<eCAL::v5::CServiceClient*>(handle_);
  if (client->RemEventCallback(enum_to_enum_class(type_))) return(1);
  return(0);
}

ECALC_API int eCAL_Client_GetServiceName(ECAL_HANDLE handle_, void* buf_, int buf_len_)
{
  if (handle_ == nullptr) return(0);
  auto* client = static_cast<eCAL::v5::CServiceClient*>(handle_);
  const std::string service_name = client->GetServiceName();
  const int buffer_len = CopyBuffer(buf_, buf_len_, service_name);
  if (buffer_len != static_cast<int>(service_name.size()))
  {
    return(0);
  }
  else
  {
    return(buffer_len);
  }
}
#endif // ECAL_CORE_SERVICE
