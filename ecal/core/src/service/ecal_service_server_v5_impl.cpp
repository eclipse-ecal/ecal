/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2024 Continental Corporation
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
 * @brief  eCAL service server implementation (deprecated eCAL5 version)
**/

#include "ecal_service_server_v5_impl.h"

namespace eCAL
{
  namespace v5
  {
    CServiceServerImpl::CServiceServerImpl()
      : m_service_server_impl(nullptr)
    {
    }

    CServiceServerImpl::CServiceServerImpl(const std::string& service_name_)
      : m_service_server_impl(nullptr)
    {
    }

    CServiceServerImpl::~CServiceServerImpl()
    {
    }

    bool CServiceServerImpl::Create(const std::string& service_name_)
    {
      return false;
    }

    bool CServiceServerImpl::Destroy()
    {
      return false;
    }

    bool CServiceServerImpl::AddDescription(const std::string& method_, const std::string& req_type_, const std::string& req_desc_, const std::string& resp_type_, const std::string& resp_desc_)
    {
      return false;
    }

    bool CServiceServerImpl::AddMethodCallback(const std::string& method_, const std::string& req_type_, const std::string& resp_type_, const MethodCallbackT& callback_)
    {
      return false;
    }

    bool CServiceServerImpl::RemMethodCallback(const std::string& method_)
    {
      return false;
    }

    bool CServiceServerImpl::AddEventCallback(eCAL_Server_Event type_, ServerEventCallbackT callback_)
    {
      return false;
    }

    bool CServiceServerImpl::RemEventCallback(eCAL_Server_Event type_)
    {
      return false;
    }


    std::string CServiceServerImpl::GetServiceName()
    {
      return "";
    }

    bool CServiceServerImpl::IsConnected()
    {
      return false;
    }
  }
}
