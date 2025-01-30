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
 * @brief  eCAL service server interface
**/

#include <ecal/ecal.h>

#include "ecal_servicegate.h"
#include "ecal_global_accessors.h"
#include "ecal_service_server_impl.h"

namespace eCAL
{
  CServiceServer::CServiceServer(const std::string & service_name_, const ServerEventCallbackT& event_callback_)
    : m_service_server_impl(nullptr)
  {
    // create server implementation
    m_service_server_impl = CServiceServerImpl::CreateInstance(service_name_, event_callback_);

    // register server
    if (g_servicegate() != nullptr) g_servicegate()->Register(service_name_, m_service_server_impl);
  }

  CServiceServer::~CServiceServer()
  {
    // could be already destroyed by move
    if (m_service_server_impl == nullptr) return;

    // unregister server
    if (g_servicegate() != nullptr) g_servicegate()->Unregister(m_service_server_impl->GetServiceName(), m_service_server_impl);
  }

  CServiceServer::CServiceServer(CServiceServer && rhs) noexcept
    : m_service_server_impl(std::move(rhs.m_service_server_impl))
  {
  }

  CServiceServer & CServiceServer::operator=(CServiceServer && rhs) noexcept
  {
    if (this != &rhs)
    {
      m_service_server_impl = std::move(rhs.m_service_server_impl);
    }
    return *this;
  }

  bool CServiceServer::SetMethodCallback(const SServiceMethodInformation& method_info_, const ServiceMethodCallbackT & callback_)
  {
    if (m_service_server_impl == nullptr) return false;
    return m_service_server_impl->SetMethodCallback(method_info_, callback_);
  }

  bool CServiceServer::RemoveMethodCallback(const std::string & method_)
  {
    if (m_service_server_impl == nullptr) return false;
    return m_service_server_impl->RemoveMethodCallback(method_);
  }

  std::string CServiceServer::GetServiceName()
  {
    if (m_service_server_impl == nullptr) return "";
    return m_service_server_impl->GetServiceName();
  }

  SServiceId CServiceServer::GetServiceId() const
  {
    if (m_service_server_impl == nullptr) return SServiceId();
    return m_service_server_impl->GetServiceId();
  }

  bool CServiceServer::IsConnected()
  {
    if (m_service_server_impl == nullptr) return false;
    return m_service_server_impl->IsConnected();
  }
}
