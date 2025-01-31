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
 * @brief  eCAL service client instance implementation
**/

#include <ecal/service/client_instance.h>
#include "ecal_service_client_impl.h"

namespace eCAL
{
  CClientInstance::CClientInstance(const SEntityId& entity_id_, const std::shared_ptr<CServiceClientImpl>& service_client_id_impl_)
    : m_entity_id(entity_id_), m_service_client_impl(service_client_id_impl_)
  {
    assert(m_service_client_impl && "service_client_id_impl_ must not be null");
  }

  std::pair<bool, SServiceResponse> CClientInstance::CallWithResponse(const std::string& method_name_, const std::string& request_, int timeout_)
  {
    return m_service_client_impl->CallWithCallback(m_entity_id, method_name_, request_, nullptr, timeout_);
  }

  bool CClientInstance::CallWithCallback(const std::string& method_name_, const std::string& request_, const ResponseCallbackT& response_callback_, int timeout_)
  {
    auto response = m_service_client_impl->CallWithCallback(m_entity_id, method_name_, request_, response_callback_, timeout_);
    return response.first;
  }

  ECAL_API bool CClientInstance::CallWithCallbackAsync(const std::string& method_name_, const std::string& request_, const ResponseCallbackT& response_callback_)
  {
    return m_service_client_impl->CallWithCallbackAsync(m_entity_id, method_name_, request_, response_callback_);
  }

  bool CClientInstance::IsConnected() const
  {
    return m_service_client_impl->IsConnected(m_entity_id);
  }

  SEntityId CClientInstance::GetClientID() const
  {
    return m_entity_id;
  }
}
