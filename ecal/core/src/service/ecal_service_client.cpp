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
 * @brief  eCAL service client interface
**/

#include <ecal/service/client.h>
#include <ecal/ecal.h>

#include "ecal_clientgate.h"
#include "ecal_global_accessors.h"
#include "ecal_service_client_impl.h"

#include <string>

namespace eCAL
{
  CServiceClient::CServiceClient(const std::string& service_name_, const ServiceMethodInformationSetT& method_information_set_, const ClientEventCallbackT& event_callback_)
  {
    // create client implementation
    m_service_client_impl = CServiceClientImpl::CreateInstance(service_name_, method_information_set_, event_callback_);

    // register client
    if (g_clientgate() != nullptr) g_clientgate()->Register(service_name_, m_service_client_impl);
  }

  CServiceClient::~CServiceClient()
  {
    // could be already destroyed by move
    if (m_service_client_impl == nullptr) return;

    // unregister client
    if (g_clientgate() != nullptr) g_clientgate()->Unregister(m_service_client_impl->GetServiceName(), m_service_client_impl);
  }

  CServiceClient::CServiceClient(CServiceClient&& rhs) noexcept
    : m_service_client_impl(std::move(rhs.m_service_client_impl))
  {
  }

  CServiceClient& CServiceClient::operator=(CServiceClient&& rhs) noexcept
  {
    if (this != &rhs)
    {
      m_service_client_impl = std::move(rhs.m_service_client_impl);
    }
    return *this;
  }

  std::vector<CClientInstance> CServiceClient::GetClientInstances() const
  {
    std::vector<CClientInstance> instances;
    if (m_service_client_impl == nullptr) return instances;

    auto entity_ids = m_service_client_impl->GetServiceIDs();
    instances.reserve(entity_ids.size());
    for (const auto& entity_id : entity_ids)
    {
      instances.emplace_back(entity_id, m_service_client_impl);
    }
    return instances;
  }

  bool CServiceClient::CallWithResponse(const std::string& method_name_, const std::string& request_, ServiceResponseVecT& service_response_vec_, int timeout_) const
  {
    auto instances = GetClientInstances();
    size_t num_instances = instances.size();

    // vector to hold futures for the return values and responses
    std::vector<std::future<std::pair<bool, SServiceResponse>>> futures;
    futures.reserve(num_instances);

    // launch asynchronous calls for each instance
    for (auto& instance : instances)
    {
      futures.emplace_back(std::async(std::launch::async,
        [&instance, method_name_ = method_name_, request_ = request_, timeout_]()
        {
          return instance.CallWithResponse(method_name_, request_, timeout_);
        }));
    }

    bool overall_success = true;
    // ensure the response vector is empty before populating it
    service_response_vec_.clear();

    // collect responses
    for (auto& future : futures)
    {
      try
      {
        // explicitly unpack the pair
        const std::pair<bool, SServiceResponse> result = future.get();
        bool success = result.first;
        const SServiceResponse response = result.second;

        // add response to the vector
        service_response_vec_.emplace_back(response);

        // aggregate success states
        overall_success &= success;
      }
      catch (const std::exception& e)
      {
        // handle exceptions and add an error response
        SServiceResponse error_response;
        error_response.error_msg = e.what();
        error_response.call_state = eCallState::failed;
        service_response_vec_.emplace_back(error_response);

        // mark overall success as false if any call fails
        overall_success = false;
      }
    }

    return overall_success;
  }

  bool CServiceClient::CallWithCallback(const std::string& method_name_, const std::string& request_, const ResponseCallbackT& response_callback_, int timeout_) const
  {
    auto instances = GetClientInstances();
    size_t num_instances = instances.size();

    // vector to hold futures for the return values
    std::vector<std::future<bool>> futures;
    futures.reserve(num_instances);

    for (auto& instance : instances)
    {
      futures.emplace_back(std::async(std::launch::async,
        [&instance, method_name_ = method_name_, request_ = request_, response_callback_, timeout_]()
        {
          return instance.CallWithCallback(method_name_, request_, response_callback_, timeout_);
        }));
    }

    bool return_state = true;
    for (auto& future : futures)
    {
      try
      {
        return_state &= future.get();
      }
      catch (const std::exception& /*e*/)
      {
        // handle exceptions
        return_state = false;
      }
    }

    return return_state;
  }

  bool CServiceClient::CallWithCallbackAsync(const std::string& method_name_, const std::string& request_, const ResponseCallbackT& response_callback_) const
  {
    bool return_state = true;
    auto instances = GetClientInstances();
    for (auto& instance : instances)
    {
      return_state &= instance.CallWithCallbackAsync(method_name_, request_, response_callback_);
    }
    return return_state;
  }

  std::string CServiceClient::GetServiceName() const
  {
    if (m_service_client_impl == nullptr) return "";
    return m_service_client_impl->GetServiceName();
  }

  SServiceId CServiceClient::GetServiceId() const
  {
    if (m_service_client_impl == nullptr) return SServiceId();
    return m_service_client_impl->GetServiceId();
  }

  bool CServiceClient::IsConnected() const
  {
    const auto instances = GetClientInstances();
    for (const auto& instance : instances)
    {
      if (instance.IsConnected()) return true;
    }
    return false;
  }
}
