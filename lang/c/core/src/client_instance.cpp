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
#include <ecal_c/service/client_instance.h>

#include "common.h"

#if ECAL_CORE_SERVICE
namespace
{
  void Assign_SServiceResponse(struct eCAL_SServiceResponse* service_response_c_, const eCAL::SServiceResponse& service_response_)
  {
    static const std::map<eCAL::eCallState, eCAL_eCallState> call_state_map
    {
      {eCAL::eCallState::none, eCAL_eCallState_none},
      {eCAL::eCallState::executed, eCAL_eCallState_executed},
      {eCAL::eCallState::timeouted, eCAL_eCallState_timeouted},
      {eCAL::eCallState::failed, eCAL_eCallState_failed}
    };

    service_response_c_->call_state = call_state_map.at(service_response_.call_state);
    Assign_SServiceId(&service_response_c_->server_id, service_response_.server_id);
    Assign_SServiceMethodInformation(&service_response_c_->service_method_information, service_response_.service_method_information);
    service_response_c_->ret_state = service_response_.ret_state;
    service_response_c_->response =service_response_.response.data();
    service_response_c_->response_length = service_response_.response.size();
    service_response_c_->error_msg =service_response_.error_msg.c_str();
  }

  void Convert_SServiceResponse(struct eCAL_SServiceResponse* service_response_c_, const eCAL::SServiceResponse& service_response_)
  {
    static const std::map<eCAL::eCallState, eCAL_eCallState> call_state_map
    {
      {eCAL::eCallState::none, eCAL_eCallState_none},
      {eCAL::eCallState::executed, eCAL_eCallState_executed},
      {eCAL::eCallState::timeouted, eCAL_eCallState_timeouted},
      {eCAL::eCallState::failed, eCAL_eCallState_failed}
    };

    service_response_c_->call_state = call_state_map.at(service_response_.call_state);
    Convert_SServiceId(&service_response_c_->server_id, service_response_.server_id);
    Convert_SServiceMethodInformation(&service_response_c_->service_method_information, service_response_.service_method_information);
    service_response_c_->ret_state = service_response_.ret_state;
    service_response_c_->response = Clone_CArray(service_response_.response.data(), service_response_.response.size());
    service_response_c_->error_msg = Clone_CString(service_response_.error_msg.c_str());
  }

  void Convert_SServiceResponseResult(struct eCAL_SServiceResponseResult* service_response_result_c_, const std::pair<bool, eCAL::SServiceResponse>& service_response_result_)
  {
    service_response_result_c_->result = service_response_result_.first;
    Convert_SServiceResponse(&service_response_result_c_->service_response, service_response_result_.second);
  }
}

extern "C"
{
  struct eCAL_ClientInstance
  {
    eCAL::CClientInstance* handle;
  };

  ECALC_API void eCAL_ClientInstance_Delete(eCAL_ClientInstance* client_instance_)
  {
    delete client_instance_->handle;
    delete client_instance_;
  }

  ECALC_API void eCAL_ClientInstances_Delete(eCAL_ClientInstance** client_instances_)
  {
    if (client_instances_ != NULL)
    {
      for (size_t i = 0; client_instances_[i] != NULL; ++i)
      {
        eCAL_ClientInstance_Delete(client_instances_[i]);
      }
      std::free(client_instances_);
    }
  }

  ECALC_API struct eCAL_SServiceResponseResult* eCAL_ClientInstance_CallWithResponse(eCAL_ClientInstance* client_instance_, const char* method_name_, const void* request_, size_t request_length_, int timeout_ms_)
  {
    std::string request_str(static_cast<const char*>(request_), request_length_);
    auto service_response_result = client_instance_->handle->CallWithResponse(method_name_, request_str, timeout_ms_);

    auto* service_response_result_c = reinterpret_cast<struct eCAL_SServiceResponseResult*>(std::malloc(sizeof(struct eCAL_SServiceResponseResult)));
    if (service_response_result_c != NULL)
    {
      Convert_SServiceResponseResult(service_response_result_c, service_response_result);
    }

    return service_response_result_c;
  }

  ECALC_API int eCAL_ClientInstance_CallWithCallback(eCAL_ClientInstance* client_instance_, const char* method_name_, const void* request_, size_t request_length_, eCAL_ResponseCallbackT response_callback_, int timeout_ms_)
  {
    std::string request(static_cast<const char*>(request_), request_length_);
    const auto response_callback = [response_callback_](const eCAL::SServiceResponse& service_response_)
    {
      struct eCAL_SServiceResponse service_response_c;
      Assign_SServiceResponse(&service_response_c, service_response_);
      response_callback_(&service_response_c);
    };

    return !static_cast<int>(client_instance_->handle->CallWithCallback(method_name_, request, response_callback, timeout_ms_));
  }

  ECALC_API int eCAL_ClientInstance_CallWithCallbackAsync(eCAL_ClientInstance* client_instance_, const char* method_name_, const void* request_, size_t request_length_, eCAL_ResponseCallbackT response_callback_)
  {
    std::string request(static_cast<const char*>(request_), request_length_);
    const auto response_callback = [response_callback_](const eCAL::SServiceResponse& service_response_)
    {
      struct eCAL_SServiceResponse service_response_c;
      Assign_SServiceResponse(&service_response_c, service_response_);
      response_callback_(&service_response_c);
    };

    return !static_cast<int>(client_instance_->handle->CallWithCallbackAsync(method_name_, request, response_callback));
  }

  ECALC_API int eCAL_ClientInstance_IsConnected(eCAL_ClientInstance* client_instance_)
  {
    return static_cast<int>(client_instance_->handle->IsConnected());
  }

  ECALC_API struct eCAL_SEntityId* eCAL_ClientInstance_GetClientID(eCAL_ClientInstance* client_instance_)
  {
    auto* client_id = reinterpret_cast<eCAL_SEntityId*>(std::malloc(sizeof(eCAL_SEntityId)));
    if (client_id != NULL)
    {
      Convert_SEntityId(client_id, client_instance_->handle->GetClientID());
    }
    return client_id;
  }
}
#endif // ECAL_CORE_SERVICE
