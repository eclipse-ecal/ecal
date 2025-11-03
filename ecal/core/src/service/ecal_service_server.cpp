/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
 * Copyright 2025 AUMOVIO and subsidiaries. All rights reserved.
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
  CServiceServer::CServiceServer(const std::string& service_name_, const ServerEventCallbackT& event_callback_)
  {
    // create server implementation
    auto service_server_impl = CServiceServerImpl::CreateInstance(service_name_, event_callback_);
    m_service_server_impl = service_server_impl;

    // register server
    auto servicegate = g_servicegate();
    if (servicegate) servicegate->Register(service_name_, service_server_impl);
  }

  CServiceServer::~CServiceServer()
  {
    auto service_server_impl = m_service_server_impl.lock();
    // could be already destroyed by move
    if (service_server_impl == nullptr) return;

    // unregister server
    auto servicegate = g_servicegate();
    if (servicegate) servicegate->Unregister(service_server_impl->GetServiceName(), service_server_impl);
  }

  CServiceServer::CServiceServer(CServiceServer&& rhs) noexcept
    : m_service_server_impl(std::move(rhs.m_service_server_impl))
  {
  }

  CServiceServer& CServiceServer::operator=(CServiceServer&& rhs) noexcept
  {
    if (this != &rhs)
    {
      m_service_server_impl = std::move(rhs.m_service_server_impl);
    }
    return *this;
  }

  bool CServiceServer::SetMethodCallback(const SServiceMethodInformation& method_info_, const ServiceMethodCallbackT& callback_)
  {
    auto service_server_impl = m_service_server_impl.lock();
    if (service_server_impl) return service_server_impl->SetMethodCallback(method_info_, callback_);
    return false;
  }

  bool CServiceServer::RemoveMethodCallback(const std::string& method_)
  {
    auto service_server_impl = m_service_server_impl.lock();
    if (service_server_impl) return service_server_impl->RemoveMethodCallback(method_);
    return false;
  }

  const std::string& CServiceServer::GetServiceName() const
  {
    auto service_server_impl = m_service_server_impl.lock();
    static const std::string empty_service_name {};
    if (service_server_impl) return service_server_impl->GetServiceName();
    return empty_service_name;
  }

  const SServiceId& CServiceServer::GetServiceId() const
  {
    auto service_server_impl = m_service_server_impl.lock();
    static const SServiceId empty_service_id {};
    if (service_server_impl) return service_server_impl->GetServiceId();
    return empty_service_id;
  }

  bool CServiceServer::IsConnected() const
  {
    auto service_server_impl = m_service_server_impl.lock();
    if (service_server_impl) return service_server_impl->IsConnected();
    return false;
  }
}
