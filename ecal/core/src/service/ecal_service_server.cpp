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
#include <string>

#include "ecal_servicegate.h"
#include "ecal_global_accessors.h"
#include "ecal_service_server_impl.h"

namespace eCAL
{
  CServiceServer::CServiceServer(const std::string& service_name_, const ServerEventCallbackT callback_) :
    m_service_server_impl(nullptr)
  {
  }

  CServiceServer::~CServiceServer()
  {
  }

  bool CServiceServer::AddMethodCallback(const std::string& method_, const SServiceMethodInformation& method_info_, const MethodCallbackT& callback_)
  {
    return false;
  }

  bool CServiceServer::RemMethodCallback(const std::string& method_)
  {
    return false;
  }

  std::string CServiceServer::GetServiceName()
  {
    return "";
    //return m_service_server_impl->GetServiceName();
  }

  bool CServiceServer::IsConnected()
  {
    return false;
    //return m_service_server_impl->IsConnected();
  }
}
