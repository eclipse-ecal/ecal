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
#include <ecal_c/service/client.h>

#include "common.h"

#include <cassert>

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
}

extern "C"
{
  struct eCAL_ServiceClient
  {
    eCAL::CServiceClient* handle;
    eCAL_SServiceId service_id;
    eCAL_SEntityId entity_id;
  };

  struct eCAL_ClientInstance
  {
    eCAL::CClientInstance* handle;
  };

  ECALC_API eCAL_ServiceClient* eCAL_ServiceClient_New(const char* service_name_, const struct eCAL_SServiceMethodInformation* method_information_set_, size_t method_information_set_length_, eCAL_ClientEventCallbackT event_callback_)
  {
    assert(service_name_ != NULL);

    eCAL::ServiceMethodInformationSetT method_information_set;
    for (size_t i = 0; i < method_information_set_length_ && method_information_set_ != NULL; ++i)
    {
      eCAL::SServiceMethodInformation method_info;
      method_info.method_name = method_information_set_[i].method_name;
      Assign_SDataTypeInformation(method_info.request_type, &method_information_set_[i].request_type);
      Assign_SDataTypeInformation(method_info.response_type, &method_information_set_[i].response_type);
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

    return new eCAL_ServiceClient{ new eCAL::CServiceClient(service_name_, method_information_set, event_callback_ != NULL ? event_callback : eCAL::ClientEventCallbackT()) };
  }

  ECALC_API void eCAL_ServiceClient_Delete(eCAL_ServiceClient* service_client_)
  {
    assert(service_client_ != NULL);
    delete service_client_->handle;
    delete service_client_;
  }

  ECALC_API eCAL_ClientInstance** eCAL_ServiceClient_GetClientInstances(eCAL_ServiceClient* service_client_)
  {
    assert(service_client_ != NULL);
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

  ECALC_API int eCAL_ServiceClient_CallWithResponse(eCAL_ServiceClient* service_client_, const char* method_name_, const void* request_, size_t request_length_, struct eCAL_SServiceResponse** service_response_vec_, size_t* service_response_vec_length_, const int* timeout_ms_)
  {
    assert(service_client_ != NULL && method_name_ != NULL && service_response_vec_ != NULL && service_response_vec_length_ != NULL);
    assert(*service_response_vec_ == NULL && *service_response_vec_length_ == 0);
    
    std::string request;
    if (request_ != NULL && request_length_ != 0)
      request.assign(reinterpret_cast<const char*>(request_), request_length_);

    eCAL::ServiceResponseVecT service_response_vec;
    const bool result = service_client_->handle->CallWithResponse(method_name_, request, service_response_vec, timeout_ms_ != NULL ? *timeout_ms_ : eCAL::CServiceClient::DEFAULT_TIME_ARGUMENT);

    const auto base_size = aligned_size(sizeof(struct eCAL_SServiceResponse) * service_response_vec.size());
    std::size_t extended_size{ 0 };
    for (const auto& service_response : service_response_vec)
    {
      extended_size += ExtSize_SServiceResponse(service_response);
    }

    *service_response_vec_ = reinterpret_cast<struct eCAL_SServiceResponse*>(std::malloc(base_size + extended_size));

    if (*service_response_vec_ != NULL)
    {
      auto* offset = reinterpret_cast<char*>(*service_response_vec_) + base_size;
      *service_response_vec_length_ = service_response_vec.size();
      for (std::size_t i = 0; i < service_response_vec.size(); ++i)
      {
        Assign_SServiceResponse(&(*service_response_vec_)[i], service_response_vec.at(i), &offset);
      }
    }
    return !static_cast<int>(result);
  }

  ECALC_API int eCAL_ServiceClient_CallWithCallback(eCAL_ServiceClient* service_client_, const char* method_name_, const void* request_, size_t request_length_, eCAL_ResponseCallbackT response_callback_, void* user_argument_, const int* timeout_ms_)
  {
    assert(service_client_ != NULL && method_name_ != NULL && response_callback_ != NULL);
    std::string request;
    if (request_ != NULL && request_length_ != 0)
      request.assign(reinterpret_cast<const char*>(request_), request_length_);
    const auto response_callback = [response_callback_, user_argument_](const eCAL::SServiceResponse& service_response_)
    {
      struct eCAL_SServiceResponse service_response_c;
      Assign_SServiceResponse(&service_response_c, service_response_);
      response_callback_(&service_response_c, user_argument_);
    };

    return !static_cast<int>(service_client_->handle->CallWithCallback(method_name_, request, response_callback, timeout_ms_ != NULL ? *timeout_ms_ : eCAL::CServiceClient::DEFAULT_TIME_ARGUMENT));
  }

  ECALC_API int eCAL_ServiceClient_CallWithCallbackAsync(eCAL_ServiceClient* service_client_, const char* method_name_, const void* request_, size_t request_length_, eCAL_ResponseCallbackT response_callback_, void* user_argument_)
  {
    assert(service_client_ != NULL && method_name_ != NULL && response_callback_ != NULL);
    std::string request;
    if (request_ != NULL && request_length_ != 0)
      request.assign(reinterpret_cast<const char*>(request_), request_length_);
    const auto response_callback = [response_callback_, user_argument_](const eCAL::SServiceResponse& service_response_)
    {
      struct eCAL_SServiceResponse service_response_c;
      Assign_SServiceResponse(&service_response_c, service_response_);
      response_callback_(&service_response_c, user_argument_);
    };

    return !static_cast<int>(service_client_->handle->CallWithCallbackAsync(method_name_, request, response_callback));
  }

  ECALC_API const char* eCAL_ServiceClient_GetServiceName(eCAL_ServiceClient* service_client_)
  {
    assert(service_client_ != NULL);
    return service_client_->handle->GetServiceName().c_str();
  }

  ECALC_API const struct eCAL_SServiceId* eCAL_ServiceClient_GetServiceId(eCAL_ServiceClient* service_client_)
  {
    assert(service_client_ != NULL);
    Assign_SServiceId(&service_client_->service_id, service_client_->handle->GetServiceId());
    return &service_client_->service_id;
  }

  ECALC_API int eCAL_ServiceClient_IsConnected(eCAL_ServiceClient* service_client_)
  {
    assert(service_client_ != NULL);
    return static_cast<int>(service_client_->handle->IsConnected());
  }
}
#endif // ECAL_CORE_SERVICE
