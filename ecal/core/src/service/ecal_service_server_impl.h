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
 * @brief  eCAL service server implementation
**/

#pragma once

#include <ecal/ecal_callback.h>
#include <ecal/ecal_service_info.h>

#include "serialization/ecal_serialize_sample_registration.h"
#include "serialization/ecal_struct_service.h"

#include <memory>
#include <mutex>
#include <string>
#include <map>
#include <functional>

namespace eCAL
{
  /**
   * @brief Implementation class for eCAL service server.
   */
  class CServiceServerImpl : public std::enable_shared_from_this<CServiceServerImpl>
  {
  public:
    // Factory method to create an instance of the client implementation
    static std::shared_ptr<CServiceServerImpl> CreateInstance(
      const std::string& service_name_, const ServerEventIDCallbackT& event_callback_);

  private:
    // Private constructor to enforce creation through factory method
    CServiceServerImpl(const std::string& service_name_, const ServerEventIDCallbackT& event_callback_);

  public:
    ~CServiceServerImpl();

    bool AddMethodCallback(const std::string& method_, const SServiceMethodInformation& method_info_, const MethodCallbackT& callback_);
    bool RemMethodCallback(const std::string& method_);

    // Check connection state of a specific service
    bool IsConnected() const;

    // Called by the registration receiver to process a service registration
    void RegisterClient(const std::string& key_, const SClientAttr& client_);

    // Called by the registration provider to get a registration sample
    Registration::Sample GetRegistration();

    // Retrieves the service name
    std::string GetServiceName() const;

    // Prevent copy and move operations
    CServiceServerImpl(const CServiceServerImpl&) = delete;
    CServiceServerImpl& operator=(const CServiceServerImpl&) = delete;
    CServiceServerImpl(CServiceServerImpl&&) = delete;
    CServiceServerImpl& operator=(CServiceServerImpl&&) = delete;

  private:
    // Prepare and retrieve registration and unregistration samples
    Registration::Sample GetRegistrationSample();
    Registration::Sample GetUnregistrationSample();

    // Service attributes
    std::string m_service_name;

    // Event callback map and synchronization
    mutable std::mutex m_event_callback_mutex;
    ServerEventIDCallbackT m_event_callback;
  };
}
