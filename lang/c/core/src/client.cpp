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

#if ECAL_CORE_SERVICE
namespace
{
  void Assign_SClientEventCallbackData(struct eCAL_SClientEventCallbackData* client_event_callback_data_c_, const eCAL::SClientEventCallbackData& client_event_callback_data_)
  {
    static const std::map<eCAL::eClientEvent, eCAL_eClientEvent> client_event_map
    {
      {eCAL::eClientEvent::none, eCAL_eClientEvent_none},
      {eCAL::eClientEvent::connected, eCAL_eClientEvent_connected},
      {eCAL::eClientEvent::disconnected, eCAL_eClientEvent_disconnected},
      {eCAL::eClientEvent::timeout, eCAL_eClientEvent_timeout}
    };

    client_event_callback_data_c_->type = client_event_map.at(client_event_callback_data_.type);
    client_event_callback_data_c_->time = client_event_callback_data_.time;
  }

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
  struct eCAL_ServiceClient
  {
    eCAL::CServiceClient* handle;
  };

  struct eCAL_ClientInstance
  {
    eCAL::CClientInstance* handle;
  };

  ECALC_API eCAL_ServiceClient* eCAL_ServiceClient_New(const char* service_name_, const struct eCAL_SServiceMethodInformation* method_information_set_, size_t method_information_set_length_, eCAL_ClientEventCallbackT event_callback_)
  {
    eCAL::ServiceMethodInformationSetT method_information_set;
    for (size_t i = 0; i < method_information_set_length_ && method_information_set_ != NULL; ++i)
    {
      eCAL::SServiceMethodInformation method_info;
      method_info.method_name = method_information_set_[i].method_name;
      Convert_SDataTypeInformation(method_info.request_type, &method_information_set_[i].request_type);
      Convert_SDataTypeInformation(method_info.response_type, &method_information_set_[i].response_type);
      method_information_set.insert(method_info);
    }

    const auto event_callback = [event_callback_](const eCAL::SServiceId& service_id_, const eCAL::SClientEventCallbackData& client_event_callback_data_)
    {
      struct eCAL_SServiceId service_id_c;
      struct eCAL_SClientEventCallbackData client_event_callback_data_c;

      Assign_SServiceId(&service_id_c, service_id_);
      Assign_SClientEventCallbackData(&client_event_callback_data_c, client_event_callback_data_);
      event_callback_(&service_id_c, &client_event_callback_data_c);
    };

    return new eCAL_ServiceClient{ new eCAL::CServiceClient(service_name_, method_information_set, event_callback) };
  }

  ECALC_API void eCAL_ServiceClient_Delete(eCAL_ServiceClient* service_client_)
  {
    delete service_client_->handle;
    delete service_client_;
  }

  ECALC_API eCAL_ClientInstance** eCAL_ServiceClient_GetClientInstances(eCAL_ServiceClient* service_client_)
  {
    auto client_instances = service_client_->handle->GetClientInstances();

    eCAL_ClientInstance**  client_instances_c = reinterpret_cast<eCAL_ClientInstance**>(std::malloc(sizeof(eCAL_ClientInstance*) * (client_instances.size() + 1)));
    if (client_instances_c != NULL)
    {
      for (std::size_t i = 0; i < client_instances.size(); ++i)
      {
        client_instances_c[i] = new eCAL_ClientInstance{new eCAL::CClientInstance(std::move(client_instances[i]))};
      }
      client_instances_c[client_instances.size()] = NULL;
    }
      
    return client_instances_c;
  }

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

  ECALC_API int eCAL_ServiceClient_CallWithResponse(eCAL_ServiceClient* service_client_, const char* method_name_, const void* request_, size_t request_length_, struct eCAL_SServiceResponse** service_response_vec_, size_t* service_response_vec_length_, int timeout_ms_)
  {
    std::string request(reinterpret_cast<const char*>(request_), request_length_);
    eCAL::ServiceResponseVecT service_response_vec;
    bool result = service_client_->handle->CallWithResponse(method_name_, request, service_response_vec, timeout_ms_);

    *service_response_vec_length_ = service_response_vec.size();
    *service_response_vec_ = reinterpret_cast<eCAL_SServiceResponse*>(std::malloc(sizeof(eCAL_SServiceResponse) * (*service_response_vec_length_)));
    for (size_t i = 0; i < *service_response_vec_length_; ++i)
    {
      Assign_SServiceResponse(&(*service_response_vec_)[i], service_response_vec[i]);
    }

    return !static_cast<int>(result);
  }

  ECALC_API int eCAL_ServiceClient_CallWithCallback(eCAL_ServiceClient* service_client_, const char* method_name_, const void* request_, size_t request_length_, eCAL_ResponseCallbackT response_callback_, int timeout_ms_)
  {
    std::string request(reinterpret_cast<const char*>(request_), request_length_);
    const auto response_callback = [response_callback_](const eCAL::SServiceResponse& service_response_)
    {
      struct eCAL_SServiceResponse service_response_c;
      Assign_SServiceResponse(&service_response_c, service_response_);
      response_callback_(&service_response_c);
    };

    return !static_cast<int>(service_client_->handle->CallWithCallback(method_name_, request, response_callback, timeout_ms_));
  }

  ECALC_API int eCAL_ServiceClient_CallWithCallbackAsync(eCAL_ServiceClient* service_client_, const char* method_name_, const void* request_, size_t request_length_, eCAL_ResponseCallbackT response_callback_)
  {
    std::string request(reinterpret_cast<const char*>(request_), request_length_);
    const auto response_callback = [response_callback_](const eCAL::SServiceResponse& service_response_)
    {
      struct eCAL_SServiceResponse service_response_c;
      Assign_SServiceResponse(&service_response_c, service_response_);
      response_callback_(&service_response_c);
    };

    return !static_cast<int>(service_client_->handle->CallWithCallbackAsync(method_name_, request, response_callback));
  }

  ECALC_API const char* eCAL_ServiceClient_GetServiceName(eCAL_ServiceClient* service_client_)
  {
    return service_client_->handle->GetServiceName().c_str();
  }

  ECALC_API struct eCAL_SServiceId* eCAL_ServiceClient_GetServiceId(eCAL_ServiceClient* service_client_)
  {
    auto* service_id = reinterpret_cast<eCAL_SServiceId*>(std::malloc(sizeof(eCAL_SServiceId)));
    if (service_id != NULL)
    {
      Convert_SServiceId(service_id, service_client_->handle->GetServiceId());
    }
    return service_id;
  }

  ECALC_API int eCAL_ServiceClient_IsConnected(eCAL_ServiceClient* service_client_)
  {
    return static_cast<int>(service_client_->handle->IsConnected());
  }

  ECALC_API void eCAL_SServiceId_Free(struct eCAL_SServiceId* service_id_)
  {
    if (service_id_ != NULL)
    {
      Free_SServiceId(service_id_);
      std::free(service_id_);
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

  ECALC_API void eCAL_SEntityId_Free(struct eCAL_SEntityId* entity_id_)
  {
    if (entity_id_ != NULL)
    {
      Free_SEntityId(entity_id_);
      std::free(reinterpret_cast<void*>(entity_id_));
    }
  }
}
#endif // ECAL_CORE_SERVICE
