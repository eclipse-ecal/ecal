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
 * @brief  eCAL service server implementation (deprecated eCAL5 version)
**/

#pragma once

#include <ecal/deprecate.h>
#include <ecal/namespace.h>
#include <ecal/os.h>
#include <ecal/service/server.h>
#include <ecal/v5/ecal_callback.h>

#include <map>
#include <mutex>
#include <string>

namespace eCAL
{
  ECAL_CORE_NAMESPACE_V5
  {
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

      bool AddEventCallback(eServerEvent type_, ServerEventCallbackT callback_);
      bool RemEventCallback(eServerEvent type_);

      std::string GetServiceName();
      bool IsConnected();

      // Prevent copy and move operations
      CServiceServerImpl(const CServiceServerImpl&) = delete;
      CServiceServerImpl& operator=(const CServiceServerImpl&) = delete;
      CServiceServerImpl(CServiceServerImpl&&) = delete;
      CServiceServerImpl& operator=(CServiceServerImpl&&) = delete;

    private:
      // Pointer to the underlying service server implementation
      std::shared_ptr<eCAL::CServiceServer> m_service_server_impl;

      // Mutex and map for managing event callbacks
      std::mutex m_event_callback_map_mutex;
      std::map<eServerEvent, ServerEventCallbackT> m_event_callback_map;
    };
  }
}
