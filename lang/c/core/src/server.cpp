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
 * @file   server.cpp
 * @brief  eCAL service c interface
**/

#include <ecal/ecal.h>
#include <ecal/v5/ecal_server.h>
#include <ecal_c/service/server.h>

#include "common.h"

#include <mutex>

#if ECAL_CORE_SERVICE
namespace
{
  eCAL_Server_Event enum_class_to_enum(eCAL::eServerEvent cpp_event_)
  {
    switch (cpp_event_)
    {
    case eCAL::eServerEvent::none:         return server_event_none;
    case eCAL::eServerEvent::connected:    return server_event_connected;
    case eCAL::eServerEvent::disconnected: return server_event_disconnected;
    default:                                            return server_event_none;
    }
  }

  eCAL::eServerEvent enum_to_enum_class(eCAL_Server_Event c_event)
  {
    switch (c_event)
    {
    case server_event_none:         return eCAL::eServerEvent::none;
    case server_event_connected:    return eCAL::eServerEvent::connected;
    case server_event_disconnected: return eCAL::eServerEvent::disconnected;
    default:                        return eCAL::eServerEvent::none;
    }
  }



  std::recursive_mutex g_method_callback_mtx; // NOLINT(*-avoid-non-const-global-variables)
  int g_method_callback(const std::string& method_, const std::string& req_type_, const std::string& resp_type_, const std::string& request_, std::string& response_, MethodCallbackCT callback_, void* par_)
  {
    const std::lock_guard<std::recursive_mutex> lock(g_method_callback_mtx);
    void* response(nullptr);
    int   response_len(ECAL_ALLOCATE_4ME);
    const int ret_state = callback_(method_.c_str(), req_type_.c_str(), resp_type_.c_str(), request_.c_str(), static_cast<int>(request_.size()), &response, &response_len, par_);
    if (response_len > 0)
    {
      response_ = std::string(static_cast<const char*>(response), static_cast<size_t>(response_len));
    }
    return ret_state;
  }

  std::recursive_mutex g_server_event_callback_mtx; // NOLINT(*-avoid-non-const-global-variables)
  void g_server_event_callback(const char* name_, const struct eCAL::v5::SServerEventCallbackData* data_, const ServerEventCallbackCT callback_, void* par_)
  {
    const std::lock_guard<std::recursive_mutex> lock(g_server_event_callback_mtx);
    SServerEventCallbackDataC data{};
    data.time = data_->time;
    data.type = enum_class_to_enum(data_->type);
    callback_(name_, &data, par_);
  }
}

extern "C"
{
  ECALC_API ECAL_HANDLE eCAL_Server_Create(const char* service_name_)
  {
    if (service_name_ == nullptr) return(nullptr);
    auto* server = new eCAL::v5::CServiceServer(service_name_);
    return(server);
  }

  ECALC_API int eCAL_Server_Destroy(ECAL_HANDLE handle_)
  {
    if (handle_ == nullptr) return(0);
    auto* server = static_cast<eCAL::v5::CServiceServer*>(handle_);
    delete server; // NOLINT(*-owning-memory)
    return(1);
  }

  ECALC_API int eCAL_Server_AddMethodCallback(ECAL_HANDLE handle_, const char* method_, const char* req_type_, const char* resp_type_, MethodCallbackCT callback_, void* par_)
  {
    if (handle_ == nullptr) return(0);
    auto* server = static_cast<eCAL::v5::CServiceServer*>(handle_);
    auto callback = std::bind(g_method_callback, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, callback_, par_);
    return static_cast<int>(server->AddMethodCallback(method_, req_type_, resp_type_, callback));
  }

  ECALC_API int eCAL_Server_RemMethodCallback(ECAL_HANDLE handle_, const char* method_)
  {
    if (handle_ == nullptr) return(0);
    auto* server = static_cast<eCAL::v5::CServiceServer*>(handle_);
    return static_cast<int>(server->RemMethodCallback(method_));
  }

  ECALC_API int eCAL_Server_AddEventCallback(ECAL_HANDLE handle_, enum eCAL_Server_Event type_, ServerEventCallbackCT callback_, void* par_)
  {
    if (handle_ == nullptr) return(0);
    auto* server = static_cast<eCAL::v5::CServiceServer*>(handle_);
    auto callback = std::bind(g_server_event_callback, std::placeholders::_1, std::placeholders::_2, callback_, par_);
    if (server->AddEventCallback(enum_to_enum_class(type_), callback)) return(1);
    return(0);
  }

  ECALC_API int eCAL_Server_RemEventCallback(ECAL_HANDLE handle_, enum eCAL_Server_Event type_)
  {
    if (handle_ == nullptr) return(0);
    auto* server = static_cast<eCAL::v5::CServiceServer*>(handle_);
    if (server->RemEventCallback(enum_to_enum_class(type_))) return(1);
    return(0);
  }

  ECALC_API int eCAL_Server_GetServiceName(ECAL_HANDLE handle_, void* buf_, int buf_len_)
  {
    if (handle_ == nullptr) return(0);
    auto* server = static_cast<eCAL::v5::CServiceServer*>(handle_);
    const std::string service_name = server->GetServiceName();
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
}
#endif // ECAL_CORE_SERVICE
