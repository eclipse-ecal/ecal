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
 * @brief  eCAL service client interface
**/

#include <ecal/ecal.h>
#include <string>

#include "ecal_clientgate.h"
#include "ecal_global_accessors.h"
#include "ecal_service_client_id_impl.h"

namespace eCAL
{
  /**
   * @brief Constructor.
   *
   * @param service_name_            Service name.
   * @param method_information_map_  Map of method names and corresponding datatype information.
   * @param event_callback_          The client event callback funtion.
  **/
  CServiceClientID::CServiceClientID(const std::string& service_name_, const ServiceMethodInformationMapT method_information_map_, const ClientEventIDCallbackT event_callback_)
    : m_service_name(service_name_)
  {
    // Create client implementation
    m_service_client_impl = CServiceClientIDImpl::CreateInstance(m_service_name, method_information_map_, event_callback_);

    // Register client
    if (g_clientgate() != nullptr)
    {
      g_clientgate()->Register(m_service_name, m_service_client_impl);
    }
  }

  /**
   * @brief Destructor.
  **/
  CServiceClientID::~CServiceClientID()
  {
    // Unregister client
    if (g_clientgate() != nullptr)
    {
      g_clientgate()->Unregister(m_service_name, m_service_client_impl);
    }

    // Reset client implementation
    m_service_client_impl.reset();
  }

  /**
   * @brief Move constructor
  **/
  CServiceClientID::CServiceClientID(CServiceClientID&& rhs) noexcept
    : m_service_name(std::move(rhs.m_service_name))
    , m_service_client_impl(std::move(rhs.m_service_client_impl))
  {
    rhs.m_service_client_impl = nullptr;
  }

  /**
   * @brief Move assignment operator
  **/
  CServiceClientID& CServiceClientID::operator=(CServiceClientID&& rhs) noexcept
  {
    if (this != &rhs)
    {
      // Unregister current client
      if (g_clientgate() != nullptr && m_service_client_impl)
      {
        g_clientgate()->Unregister(m_service_name, m_service_client_impl);
      }

      // Move data
      m_service_name = std::move(rhs.m_service_name);
      m_service_client_impl = std::move(rhs.m_service_client_impl);

      rhs.m_service_client_impl = nullptr;
    }
    return *this;
  }

  /**
   * @brief Get the client instances for all matching services
   *
   * @return  Vector of client instances
  **/
  std::vector<CServiceClientInstance> CServiceClientID::GetServiceClientInstances() const
  {
    std::vector<CServiceClientInstance> instances;
    if (!m_service_client_impl) return instances;

    auto entity_ids = m_service_client_impl->GetServiceIDs();
    instances.reserve(entity_ids.size());
    for (const auto& entity_id : entity_ids)
    {
      instances.emplace_back(entity_id, m_service_client_impl);
    }
    return instances;
  }

  /**
   * @brief Blocking call (with timeout) of a service method for all existing service instances, using callback
   *
   * @param method_name_        Method name.
   * @param request_            Request string.
   * @param timeout_            Maximum time before operation returns (in milliseconds, -1 means infinite).
   * @param response_callback_  Callback function for the service method response.
   *
   * @return  True if all calls were successful.
  **/
  bool CServiceClientID::CallWithCallback(const std::string& method_name_, const std::string& request_, int timeout_, const ResponseIDCallbackT& response_callback_) const
  {
    bool return_state = true;
    auto instances = GetServiceClientInstances();
    for (auto& instance : instances)
    {
      return_state &= instance.CallWithCallback(method_name_, request_, timeout_, response_callback_);
    }
    return return_state;
  }

  /**
   * @brief Asynchronous call of a service method for all existing service instances, using callback
   *
   * @param method_name_        Method name.
   * @param request_            Request string.
   * @param response_callback_  Callback function for the service method response.
   *
   * @return  True if all calls were successful.
  **/
  bool CServiceClientID::CallWithCallbackAsync(const std::string& method_name_, const std::string& request_, const ResponseIDCallbackT& response_callback_) const
  {
    bool return_state = true;
    auto instances = GetServiceClientInstances();
    for (auto& instance : instances)
    {
      return_state &= instance.CallWithCallbackAsync(method_name_, request_, response_callback_);
    }
    return return_state;
  }

  /**
   * @brief Retrieve service name.
   *
   * @return  The service name.
  **/
  std::string CServiceClientID::GetServiceName() const
  {
    return m_service_name;
  }

  /**
   * @brief Check connection to at least one service.
   *
   * @return  True if at least one service client instance is connected.
  **/
  bool CServiceClientID::IsConnected() const
  {
    const auto instances = GetServiceClientInstances();
    for (const auto& instance : instances)
    {
      if (instance.IsConnected()) return true;
    }
    return false;
  }
}
