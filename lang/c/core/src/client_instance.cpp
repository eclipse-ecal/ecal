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
 * @file   client_instance.cpp
 * @brief  eCAL client instance c interface
**/

#include <ecal/ecal.h>
#include <ecal_c/service/client_instance.h>

#include "common.h"
#include "client_instance.h"

#include <cassert>

#if ECAL_CORE_SERVICE
namespace
{
}

extern "C"
{
  ECALC_API void eCAL_ClientInstance_Delete(eCAL_ClientInstance* client_instance_)
  {
    assert(client_instance_ != NULL);
    delete client_instance_->handle;
    delete client_instance_;
  }

  ECALC_API void eCAL_ClientInstances_Delete(eCAL_ClientInstance** client_instances_)
  {
    assert(client_instances_ != NULL);
    if (client_instances_ != NULL)
    {
      for (size_t i = 0; client_instances_[i] != NULL; ++i)
      {
        eCAL_ClientInstance_Delete(client_instances_[i]);
      }
      std::free(client_instances_);
    }
  }

  ECALC_API struct eCAL_SServiceResponse* eCAL_ClientInstance_CallWithResponse(eCAL_ClientInstance* client_instance_, const char* method_name_, const void* request_, size_t request_length_, const int* timeout_ms_)
  {
    assert(client_instance_ != NULL && method_name_ != NULL);
    std::string request;
    if (request_ != NULL && request_length_ != 0)
      request.assign(reinterpret_cast<const char*>(request_), request_length_);

    const auto service_response_result = client_instance_->handle->CallWithResponse(method_name_, request, timeout_ms_ != NULL ? *timeout_ms_ : eCAL::CClientInstance::DEFAULT_TIME_ARGUMENT);
    eCAL_SServiceResponse* service_response_c = NULL;
    if (service_response_result.first)
    {
      const auto& service_response = service_response_result.second;

      const auto base_size = aligned_size(sizeof(struct eCAL_SServiceResponse));
      const auto extended_size = ExtSize_SServiceResponse(service_response);

      service_response_c = reinterpret_cast<struct eCAL_SServiceResponse*>(std::malloc(base_size + extended_size));

      if (service_response_c != NULL)
      {
        auto* offset = reinterpret_cast<char*>(service_response_c) + base_size;
        Assign_SServiceResponse(service_response_c, service_response, &offset);
      }
    }
    return service_response_c;
  }

  ECALC_API int eCAL_ClientInstance_CallWithCallback(eCAL_ClientInstance* client_instance_, const char* method_name_, const void* request_, size_t request_length_, eCAL_ResponseCallbackT callback_, void* callback_user_argument_, const int* timeout_ms_)
  {
    assert(client_instance_ != NULL && method_name_ != NULL && callback_ != NULL);
    std::string request;
    if (request_ != NULL && request_length_ != 0)
      request.assign(reinterpret_cast<const char*>(request_), request_length_);

    const auto callback = [callback_, callback_user_argument_](const eCAL::SServiceResponse& service_response_)
    {
      struct eCAL_SServiceResponse service_response_c;
      Assign_SServiceResponse(&service_response_c, service_response_);
      callback_(&service_response_c, callback_user_argument_);
    };

    return !static_cast<int>(client_instance_->handle->CallWithCallback(method_name_, request, callback, timeout_ms_ != NULL ? *timeout_ms_ : eCAL::CClientInstance::DEFAULT_TIME_ARGUMENT));
  }

  ECALC_API int eCAL_ClientInstance_CallWithCallbackAsync(eCAL_ClientInstance* client_instance_, const char* method_name_, const void* request_, size_t request_length_, eCAL_ResponseCallbackT callback_, void* callback_user_argument_)
  {
    assert(client_instance_ != NULL && method_name_ != NULL && callback_ != NULL);
    std::string request;
    if (request_ != NULL && request_length_ != 0)
      request.assign(reinterpret_cast<const char*>(request_), request_length_);
    const auto callback = [callback_, callback_user_argument_](const eCAL::SServiceResponse& service_response_)
    {
      struct eCAL_SServiceResponse service_response_c;
      Assign_SServiceResponse(&service_response_c, service_response_);
      callback_(&service_response_c, callback_user_argument_);
    };

    return !static_cast<int>(client_instance_->handle->CallWithCallbackAsync(method_name_, request, callback));
  }

  ECALC_API int eCAL_ClientInstance_IsConnected(eCAL_ClientInstance* client_instance_)
  {
    assert(client_instance_ != NULL);
    return static_cast<int>(client_instance_->handle->IsConnected());
  }

  ECALC_API const struct eCAL_SEntityId* eCAL_ClientInstance_GetClientID(eCAL_ClientInstance* client_instance_)
  {
    assert(client_instance_ != NULL);
    Assign_SEntityId(&client_instance_->entity_id, client_instance_->handle->GetClientID());
    return &client_instance_->entity_id;
  }
}
#endif // ECAL_CORE_SERVICE
