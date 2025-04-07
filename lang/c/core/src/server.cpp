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
#include <ecal_c/service/server.h>

#include "common.h"
#include <cassert>

#if ECAL_CORE_SERVICE
namespace
{
  void Assign_SServerEventCallbackData(struct eCAL_SServerEventCallbackData* server_event_callback_data_c_, const eCAL::SServerEventCallbackData& server_event_callback_data_)
  {
    static const std::map<eCAL::eServerEvent, eCAL_eServerEvent> server_event_map
    {
      {eCAL::eServerEvent::none, eCAL_eServerEvent_none},
      {eCAL::eServerEvent::connected, eCAL_eServerEvent_connected},
      {eCAL::eServerEvent::disconnected, eCAL_eServerEvent_disconnected}
    };

    server_event_callback_data_c_->type = server_event_map.at(server_event_callback_data_.type);
    server_event_callback_data_c_->time = server_event_callback_data_.time;
  }
}

extern "C"
{
  struct eCAL_ServiceServer
  {
    eCAL::CServiceServer* handle;
    eCAL_SServiceId service_id;
  };

  ECALC_API eCAL_ServiceServer* eCAL_ServiceServer_New(const char* service_name_, eCAL_ServerEventCallbackT event_callback_)
  {
    assert(service_name_ != NULL);
    const auto event_callback = [event_callback_](const eCAL::SServiceId& service_id_, const struct eCAL::SServerEventCallbackData& server_event_callback_data_)
    {
      struct eCAL_SServiceId service_id_c;
      struct eCAL_SServerEventCallbackData server_event_callback_data_c;

      Assign_SServiceId(&service_id_c, service_id_);
      Assign_SServerEventCallbackData(&server_event_callback_data_c, server_event_callback_data_);
      event_callback_(&service_id_c, &server_event_callback_data_c);
    };

    return new eCAL_ServiceServer{ new eCAL::CServiceServer(service_name_, event_callback_ != NULL ? event_callback : eCAL::ServerEventCallbackT()) };
  }

  ECALC_API void eCAL_ServiceServer_Delete(eCAL_ServiceServer* service_server_)
  {
    assert(service_server_ != NULL);
    delete service_server_->handle;
    delete service_server_;
  }

  ECALC_API int eCAL_ServiceServer_SetMethodCallback(eCAL_ServiceServer* service_server_, const struct eCAL_SServiceMethodInformation* method_info_, eCAL_ServiceMethodCallbackT callback_, void* callback_user_argument_)
  {
    assert(service_server_ != NULL && method_info_ != NULL && callback_ != NULL);
    eCAL::SServiceMethodInformation method_info;
    Assign_SServiceMethodInformation(method_info, method_info_);

    const auto callback = [callback_, callback_user_argument_](const eCAL::SServiceMethodInformation& method_info_, const std::string& request_, std::string& response_) -> int
    {
      struct eCAL_SServiceMethodInformation method_info_c;
      Assign_SServiceMethodInformation(&method_info_c, method_info_);

      void* response_c = NULL;
      size_t response_length_c = 0;
      int ret_state = callback_(&method_info_c, request_.data(), request_.size(), &response_c, &response_length_c, callback_user_argument_);

      if (response_c != NULL && response_length_c != 0)
        response_.assign(static_cast<const char*>(response_c), response_length_c);
      std::free(response_c);

      return ret_state;
    };

    return !static_cast<int>(service_server_->handle->SetMethodCallback(method_info, callback));
  }

  ECALC_API int eCAL_ServiceServer_RemoveMethodCallback(eCAL_ServiceServer* service_server_, const char* method_name_)
  {
    assert(service_server_ != NULL && method_name_ != NULL);
    return !static_cast<int>(service_server_->handle->RemoveMethodCallback(method_name_));
  }

  ECALC_API const char* eCAL_ServiceServer_GetServiceName(eCAL_ServiceServer* service_server_)
  {
    assert(service_server_ != NULL);
    return service_server_->handle->GetServiceName().c_str();
  }

  ECALC_API const struct eCAL_SServiceId* eCAL_ServiceServer_GetServiceId(eCAL_ServiceServer* service_server_)
  {
    assert(service_server_ != NULL);
    Assign_SServiceId(&service_server_->service_id, service_server_->handle->GetServiceId());
    return &service_server_->service_id;
  }

  ECALC_API int eCAL_ServiceServer_IsConnected(eCAL_ServiceServer* service_server_)
  {
    assert(service_server_ != NULL);
    return static_cast<int>(service_server_->handle->IsConnected());
  }
}
#endif // ECAL_CORE_SERVICE
