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
#include <ecal/ecal_client_deprecated.h>
#include <ecal/ecal_service_info.h>

namespace eCAL
{
  CServiceClient::CServiceClient()
    : m_service_client_impl(nullptr)
    , m_created(false)
  {
  }

  CServiceClient::CServiceClient(const std::string& service_name_)
    : m_service_client_impl(nullptr)
    , m_created(false)
  {
    Create(service_name_);
  }

  CServiceClient::CServiceClient(const std::string& service_name_, const ServiceMethodInformationMapT& method_information_map_)
    : m_service_client_impl(nullptr)
    , m_created(false)
  {
    Create(service_name_, method_information_map_);
  }

  CServiceClient::~CServiceClient()
  {
    Destroy();
  }

  bool CServiceClient::Create(const std::string& service_name_)
  {
    return Create(service_name_, ServiceMethodInformationMapT());
  }

  bool CServiceClient::Create(const std::string& service_name_, const ServiceMethodInformationMapT& method_information_map_)
  {
    if (m_created) return false;

    // Define the event callback to pass to CServiceClientNew
    ClientEventIDCallbackT event_callback = [this](const Registration::SServiceId& service_id_, const struct SClientEventCallbackData& data_)
      {
        // Lock the mutex to safely access m_event_callbacks
        std::lock_guard<std::mutex> lock(m_event_callback_map_mutex);

        // Check if there's a callback registered for the event type
        const auto& callback = m_event_callback_map.find(data_.type);
        if (callback != m_event_callback_map.end())
        {
          // Call the user's callback
          callback->second(service_id_.service_name.c_str(), &data_);
        }
      };

    // Create the new service client implementation with the event callback
    m_service_client_impl = std::make_shared<eCAL::CServiceClientNew>(
      service_name_,
      method_information_map_,
      event_callback
    );

    m_created = (m_service_client_impl != nullptr);
    return m_created;
  }

  bool CServiceClient::Destroy()
  {
    if (!m_created) return false;

    // Reset the service client implementation
    m_service_client_impl.reset();
    m_created = false;

    // Clear stored callbacks
    m_response_callback = nullptr;
    //m_event_callback = nullptr;
    m_host_name.clear();

    return true;
  }

  bool CServiceClient::SetHostName(const std::string& host_name_)
  {
    if (!m_created) return false;

    // Store the host name filter
    m_host_name = host_name_;

    return true;
  }

  bool CServiceClient::Call(const std::string& method_name_, const std::string& request_, int timeout_)
  {
    if (!m_created || !m_response_callback) return false;

    // Wrap the response callback to filter by host name if necessary
    ResponseIDCallbackT wrapped_callback = [this](const Registration::SEntityId& /*entity_id_*/, const struct SServiceResponse& service_response_)
      {
        if (m_host_name.empty() || service_response_.host_name == m_host_name)
        {
          // Call the stored response callback
          m_response_callback(service_response_);
        }
      };

    // Call the method using the new API
    return m_service_client_impl->CallWithCallback(method_name_, request_, timeout_, wrapped_callback);
  }

  bool CServiceClient::Call(const std::string& method_name_, const std::string& request_, int timeout_, ServiceResponseVecT* service_response_vec_)
  {
    if (!m_created || !service_response_vec_) return false;

    // Call the method using the new API
    ServiceResponseVecT all_responses;
    bool success = m_service_client_impl->CallWithResponse(method_name_, request_, timeout_, all_responses);

    // Filter responses based on host name if necessary
    if (!m_host_name.empty())
    {
      for (const auto& response : all_responses)
      {
        if (response.host_name == m_host_name)
        {
          service_response_vec_->push_back(response);
        }
      }
    }
    else
    {
      *service_response_vec_ = std::move(all_responses);
    }

    return success;
  }

  bool CServiceClient::CallAsync(const std::string& method_name_, const std::string& request_, int /*timeout_*/)
  {
    if (!m_created || !m_response_callback) return false;

    // Wrap the response callback to filter by host name if necessary
    ResponseIDCallbackT wrapped_callback = [this](const Registration::SEntityId& /*entity_id_*/, const struct SServiceResponse& service_response_)
      {
        if (m_host_name.empty() || service_response_.host_name == m_host_name)
        {
          // Call the stored response callback
          m_response_callback(service_response_);
        }
      };

    // Call the method asynchronously using the new API
    return m_service_client_impl->CallWithCallbackAsync(method_name_, request_, wrapped_callback);
  }

  bool CServiceClient::AddResponseCallback(const ResponseCallbackT& callback_)
  {
    if (!m_created) return false;

    {
      const std::lock_guard<std::mutex> lock(m_response_callback_mutex);
      m_response_callback = callback_;
    }

    return true;
  }

  bool CServiceClient::RemResponseCallback()
  {
    if (!m_created) return false;

    {
      const std::lock_guard<std::mutex> lock(m_response_callback_mutex);
      m_response_callback = nullptr;

    }

    return true;
  }

  bool CServiceClient::AddEventCallback(eCAL_Client_Event type_, ClientEventCallbackT callback_)
  {
    if (!m_created) return false;

    {
      const std::lock_guard<std::mutex> lock(m_event_callback_map_mutex);
      m_event_callback_map[type_] = callback_;
    }

    return true;
  }

  bool CServiceClient::RemEventCallback(eCAL_Client_Event type_)
  {
    if (!m_created) return false;

    {
      const std::lock_guard<std::mutex> lock(m_event_callback_map_mutex);
      m_event_callback_map.erase(type_);
    }

    return true;
  }

  std::string CServiceClient::GetServiceName()
  {
    if (!m_created) return "";

    return m_service_client_impl->GetServiceName();
  }

  bool CServiceClient::IsConnected()
  {
    if (!m_created) return false;

    return m_service_client_impl->IsConnected();
  }
}
