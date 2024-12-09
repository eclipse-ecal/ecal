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

#include "ecal_global_accessors.h"

#include "ecal_service_server_impl.h"
#include "registration/ecal_registration_provider.h"

namespace eCAL
{
  // Factory method to create a new instance of CServiceClientImpl
  std::shared_ptr<CServiceServerImpl> CServiceServerImpl::CreateInstance(
    const std::string& service_name_, const ServerEventIDCallbackT& event_callback_)
  {
    return std::shared_ptr<CServiceServerImpl>(new CServiceServerImpl(service_name_, event_callback_));
  }

  // Constructor: Initializes service ID and registers the service
  CServiceServerImpl::CServiceServerImpl(const std::string& service_name_, const ServerEventIDCallbackT& event_callback_)
    : m_service_name(service_name_), m_event_callback(event_callback_)
  {
  }

  // Destructor: Resets callbacks, unregisters the service, and clears data
  CServiceServerImpl::~CServiceServerImpl()
  {
    // reset event callback
    {
      const std::lock_guard<std::mutex> lock(m_event_callback_mutex);
      m_event_callback = nullptr;
    }

    // unregister client
    if (g_registration_provider() != nullptr)
    {
      g_registration_provider()->UnregisterSample(GetUnregistrationSample());
    }
  }

  bool CServiceServerImpl::AddMethodCallback(const std::string& method_, const SServiceMethodInformation& method_info_, const MethodCallbackT& callback_)
  {
    return false;
  }

  bool CServiceServerImpl::RemMethodCallback(const std::string& method_)
  {
    return false;
  }

  bool CServiceServerImpl::IsConnected() const
  {
    return false;
  }

  void CServiceServerImpl::RegisterClient(const std::string& key_, const SClientAttr& client_)
  {
  }

  Registration::Sample CServiceServerImpl::GetRegistration()
  {
    return Registration::Sample();
  }

  std::string CServiceServerImpl::GetServiceName() const
  {
    return m_service_name;
  }

  Registration::Sample CServiceServerImpl::GetRegistrationSample()
  {
    return Registration::Sample();
  }

  Registration::Sample CServiceServerImpl::GetUnregistrationSample()
  {
    return Registration::Sample();
  }
}
