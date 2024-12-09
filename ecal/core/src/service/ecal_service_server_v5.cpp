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
 * @brief  eCAL service server interface
**/

#include <ecal/ecal.h>
#include <ecal/ecal_server_v5.h>
#include <string>

#include "ecal_servicegate.h"
#include "ecal_global_accessors.h"
#include "ecal_service_server_v5_impl.h"

namespace eCAL
{
  namespace v5
  {
    CServiceServer::CServiceServer() :
                    m_service_server_impl(nullptr),
                    m_created(false)
    {
    }

    CServiceServer::CServiceServer(const std::string& service_name_) :
                    m_service_server_impl(nullptr),
                    m_created(false)
    {
      Create(service_name_);
    }

    CServiceServer::~CServiceServer()
    {
      Destroy();
    }

    bool CServiceServer::Create(const std::string& service_name_)
    {
      if(m_created) return(false);

      // create service
      m_service_server_impl = CServiceServerImpl::CreateInstance(service_name_);

      // register service
      //if (g_servicegate() != nullptr) g_servicegate()->Register(m_service_server_impl.get());

      // we made it :-)
      m_created = true;
      return(m_created);
    }

    bool CServiceServer::Destroy()
    {
      if(!m_created) return(false);
      m_created = false;

      // unregister service
      //if (g_servicegate() != nullptr) g_servicegate()->Unregister(m_service_server_impl.get());

      // stop & destroy service
      m_service_server_impl->Stop();
      m_service_server_impl.reset();

      return(true);
    }

    bool CServiceServer::AddDescription(const std::string& method_, const std::string& req_type_, const std::string& req_desc_, const std::string& resp_type_, const std::string& resp_desc_)
    {
      if (!m_created) return false;

      SDataTypeInformation request_type_information;
      request_type_information.name       = req_type_;
      request_type_information.descriptor = req_desc_;

      SDataTypeInformation response_type_information;
      response_type_information.name       = resp_type_;
      response_type_information.descriptor = resp_desc_;

      return m_service_server_impl->AddDescription(method_, request_type_information, response_type_information);
    }

    bool CServiceServer::AddMethodCallback(const std::string& method_, const std::string& req_type_, const std::string& resp_type_, const MethodCallbackT& callback_)
    {
      if (!m_created) return false;
      return m_service_server_impl->AddMethodCallback(method_, req_type_, resp_type_, callback_);
    }

    bool CServiceServer::RemMethodCallback(const std::string& method_)
    {
      if (!m_created) return false;
      return m_service_server_impl->RemMethodCallback(method_);
    }

    bool CServiceServer::AddEventCallback(eCAL_Server_Event type_, ServerEventCallbackT callback_)
    {
      if (!m_created) return false;
      return m_service_server_impl->AddEventCallback(type_, callback_);
    }

    bool CServiceServer::RemEventCallback(eCAL_Server_Event type_)
    {
      if (!m_created) return false;
      return m_service_server_impl->RemEventCallback(type_);
    }

    std::string CServiceServer::GetServiceName()
    {
      if (!m_created) return "";
      return m_service_server_impl->GetServiceName();
    }

    bool CServiceServer::IsConnected()
    {
      if (!m_created) return false;
      return m_service_server_impl->IsConnected();
    }
  }
}
