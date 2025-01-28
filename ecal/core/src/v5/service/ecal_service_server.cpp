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
 * @brief  eCAL service server interface (deprecated eCAL5 version)
**/

#include <ecal/v5/ecal_server.h>
#include <ecal/ecal.h>
#include <string>

#include "service/ecal_servicegate.h"
#include "ecal_global_accessors.h"
#include "ecal_service_server_impl.h"

namespace eCAL
{
  ECAL_CORE_NAMESPACE_V5
  {
    CServiceServer::CServiceServer()
      : m_service_server_impl(nullptr)
    {
    }

    CServiceServer::CServiceServer(const std::string& service_name_)
      : m_service_server_impl(nullptr)
    {
      Create(service_name_);
    }

    CServiceServer::~CServiceServer()
    {
      Destroy();
    }

    bool CServiceServer::Create(const std::string& service_name_)
    {
      if (m_service_server_impl != nullptr) return false;
      m_service_server_impl = std::make_shared<CServiceServerImpl>(service_name_);
      return m_service_server_impl != nullptr;
    }

    bool CServiceServer::Destroy()
    {
      if (m_service_server_impl == nullptr) return false;
      m_service_server_impl.reset();
      return true;
    }

    bool CServiceServer::AddDescription(const std::string& method_, const std::string& req_type_, const std::string& req_desc_, const std::string& resp_type_, const std::string& resp_desc_)
    {
      if (m_service_server_impl == nullptr) return false;
      return m_service_server_impl->AddDescription(method_, req_type_, req_desc_, resp_type_, resp_desc_);
    }

    bool CServiceServer::AddMethodCallback(const std::string& method_, const std::string& req_type_, const std::string& resp_type_, const MethodCallbackT& callback_)
    {
      if (m_service_server_impl == nullptr) return false;
      return m_service_server_impl->AddMethodCallback(method_, req_type_, resp_type_, callback_);
    }

    bool CServiceServer::RemMethodCallback(const std::string& method_)
    {
      if (m_service_server_impl == nullptr) return false;
      return m_service_server_impl->RemMethodCallback(method_);
    }

    bool CServiceServer::AddEventCallback(eServerEvent type_, ServerEventCallbackT callback_)
    {
      if (m_service_server_impl == nullptr) return false;
      return m_service_server_impl->AddEventCallback(type_, callback_);
    }

    bool CServiceServer::RemEventCallback(eServerEvent type_)
    {
      if (m_service_server_impl == nullptr) return false;
      return m_service_server_impl->RemEventCallback(type_);
    }

    std::string CServiceServer::GetServiceName()
    {
      if (m_service_server_impl == nullptr) return "";
      return m_service_server_impl->GetServiceName();
    }

    bool CServiceServer::IsConnected()
    {
      if (m_service_server_impl == nullptr) return false;
      return m_service_server_impl->IsConnected();
    }
  }
}
