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

#pragma once

#include <ecal/ecal_deprecate.h>
#include <ecal/ecal_os.h>
#include <ecal/ecal_server.h>

#include <string>

namespace eCAL
{
  namespace v5
  {
    /**
     * @brief Service server implementation class.
    **/
    class CServiceServerImpl
    {
    public:
      CServiceServerImpl();
      explicit CServiceServerImpl(const std::string& service_name_);
      virtual ~CServiceServerImpl();

      bool Create(const std::string& service_name_);
      bool Destroy();

      bool AddDescription(const std::string& method_, const std::string& req_type_, const std::string& req_desc_, const std::string& resp_type_, const std::string& resp_desc_);
      bool AddMethodCallback(const std::string& method_, const std::string& req_type_, const std::string& resp_type_, const MethodCallbackT& callback_);
      bool RemMethodCallback(const std::string& method_);
      bool AddEventCallback(eCAL_Server_Event type_, ServerEventCallbackT callback_);
      bool RemEventCallback(eCAL_Server_Event type_);

      std::string GetServiceName();
      bool IsConnected();

      // Prevent copy and move operations
      CServiceServerImpl(const CServiceServerImpl&) = delete;
      CServiceServerImpl& operator=(const CServiceServerImpl&) = delete;
      CServiceServerImpl(CServiceServerImpl&&) = delete;
      CServiceServerImpl& operator=(CServiceServerImpl&&) = delete;

    private:
      std::shared_ptr<eCAL::CServiceServer> m_service_server_impl;
    };
  }
}
