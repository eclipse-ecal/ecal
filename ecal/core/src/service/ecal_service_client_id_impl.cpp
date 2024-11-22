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
 * @brief  eCAL service client implementation
**/

#include "ecal_global_accessors.h"

#include "ecal_service_client_id_impl.h"
#include "ecal_service_singleton_manager.h"
#include "registration/ecal_registration_provider.h"
#include "serialization/ecal_serialize_service.h"

#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace
{
  eCAL::SServiceResponse DeserializedResponse(const std::string& response_pb_)
  {
    eCAL::SServiceResponse service_reponse;
    eCAL::Service::Response response;
    if (eCAL::DeserializeFromBuffer(response_pb_.c_str(), response_pb_.size(), response))
    {
      const auto& response_header = response.header;
      service_reponse.host_name    = response_header.hname;
      service_reponse.service_name = response_header.sname;
      service_reponse.service_id   = response_header.sid;
      service_reponse.method_name  = response_header.mname;
      service_reponse.error_msg    = response_header.error;
      service_reponse.ret_state    = static_cast<int>(response.ret_state);

      switch (response_header.state)
      {
      case eCAL::Service::eMethodCallState::executed:
        service_reponse.call_state = call_state_executed;
        break;
      case eCAL::Service::eMethodCallState::failed:
        service_reponse.call_state = call_state_failed;
        break;
      default:
        break;
      }

      service_reponse.response = std::string(response.response.data(), response.response.size());
    }
    else
    {
      service_reponse.error_msg  = "Could not parse server response";
      service_reponse.ret_state  = 0;
      service_reponse.call_state = eCallState::call_state_failed;
      service_reponse.response   = "";
    }

    return service_reponse;
  }
}

namespace eCAL
{
  // Factory method to create a new instance of CServiceClientIDImpl
  std::shared_ptr<CServiceClientIDImpl> CServiceClientIDImpl::CreateInstance(
      const std::string& service_name_, const ServiceMethodInformationMapT& method_information_map_)
  {
    return std::shared_ptr<CServiceClientIDImpl>(new CServiceClientIDImpl(service_name_, method_information_map_));
  }

  // Constructor: Initializes client ID, method call counts, and registers the client
  CServiceClientIDImpl::CServiceClientIDImpl(
      const std::string& service_name_, const ServiceMethodInformationMapT& method_information_map_)
      : m_service_name(service_name_), m_method_information_map(method_information_map_)
  {
    InitializeMethodCallCounts();
    GenerateClientID();
    Register();
  }

  // Destructor: Resets callbacks, unregisters the client, and clears data
  CServiceClientIDImpl::~CServiceClientIDImpl()
  {
    ResetAllCallbacks();
    Unregister();
  }

  // Initializes the method call count map to track the number of calls for each method
  void CServiceClientIDImpl::InitializeMethodCallCounts()
  {
    for (const auto& method_information_pair : m_method_information_map)
    {
      m_method_call_count_map[method_information_pair.first] = 0;
    }
  }

  // Generates a unique client ID based on the current time
  void CServiceClientIDImpl::GenerateClientID()
  {
    std::stringstream counter;
    counter << std::chrono::steady_clock::now().time_since_epoch().count();
    m_client_id = counter.str();
  }

  // Resets all callbacks and clears stored client information
  void CServiceClientIDImpl::ResetAllCallbacks()
  {
    {
      std::lock_guard<std::mutex> lock(m_client_session_map_sync);
      m_client_session_map.clear();
    }
    {
      std::lock_guard<std::mutex> lock(m_event_callback_map_sync);
      m_event_callback_map.clear();
    }
    m_service_name.clear();
    m_client_id.clear();
  }

  // Adds a callback function for a specific client event type
  bool CServiceClientIDImpl::AddEventCallback(eCAL_Client_Event type_, ClientEventIDCallbackT callback_)
  {
    std::lock_guard<std::mutex> lock(m_event_callback_map_sync);
    m_event_callback_map[type_] = std::move(callback_);
    return true;
  }

  // Removes a callback function for a specific client event type
  bool CServiceClientIDImpl::RemoveEventCallback(eCAL_Client_Event type_)
  {
    std::lock_guard<std::mutex> lock(m_event_callback_map_sync);
    m_event_callback_map[type_] = nullptr;
    return true;
  }

  // Retrieve service IDs of all matching services
  std::vector<Registration::SEntityId> CServiceClientIDImpl::GetServiceIDs()
  {
    std::vector<Registration::SEntityId> entity_vector;

    // lock client map
    const std::lock_guard<std::mutex> lock(m_client_session_map_sync);
    // copy session entities into return vector
    for (const auto& client_session : m_client_session_map)
    {
      entity_vector.push_back(client_session.first);
    }

    return entity_vector;
  }

  // Calls a service method synchronously, blocking until a response is received or timeout occurs
  std::pair<bool, SServiceResponse> CServiceClientIDImpl::CallWithResponse(
      const Registration::SEntityId& entity_id_, const std::string& method_name_,
      const std::string& request_, int timeout_ms_)
  {
    SClient client;
    if (!TryGetClient(entity_id_, client))
      return { false, SServiceResponse() };

    return CallBlocking(client, method_name_, request_, std::chrono::milliseconds(timeout_ms_));
  }

  // Synchronously calls a service and uses a callback for handling the response
  bool CServiceClientIDImpl::CallWithCallback(const Registration::SEntityId& entity_id_, const std::string& method_name_,
    const std::string& request_, int timeout_ms_, const ResponseIDCallbackT& repsonse_callback_)
  {
    SClient client;
    if (!TryGetClient(entity_id_, client))
      return false;

    auto response = CallBlocking(client, method_name_, request_, std::chrono::milliseconds(timeout_ms_));

    // Call the provided callback if the response is successful
    if (response.first)
    {
      repsonse_callback_(entity_id_, response.second);
      return true;
    }

    // Handle timeout event
    if (response.second.call_state == eCallState::call_state_timeouted)
    {
      NotifyEventCallback(eCAL_Client_Event::client_event_timeout, client.service_attr);
    }

    return false;
  }

  // Blocking call to a service with a specified timeout
  std::pair<bool, SServiceResponse> CServiceClientIDImpl::CallBlocking(
      SClient& client_, const std::string& method_name_,
      const std::string& request_, std::chrono::nanoseconds timeout_)
  {
    if (method_name_.empty()) return {false, SServiceResponse()};

    auto request_shared_ptr = SerializeRequest(method_name_, request_);
    if (!request_shared_ptr) return {false, SServiceResponse()};

    auto response = WaitForResponse(client_, method_name_, timeout_, request_shared_ptr);
    IncrementMethodCallCount(method_name_);
    return response;
  }

  // Attempts to retrieve a client session for a given entity ID
  bool CServiceClientIDImpl::TryGetClient(const Registration::SEntityId& entity_id_, SClient& client_)
  {
    std::lock_guard<std::mutex> lock(m_client_session_map_sync);
    auto iter = m_client_session_map.find(entity_id_);
    if (iter == m_client_session_map.end())
      return false;

    client_ = iter->second;
    return true;
  }

  // Serializes the request data into a protocol buffer and returns a shared pointer to it
  std::shared_ptr<std::string> CServiceClientIDImpl::SerializeRequest(
      const std::string& method_name_, const std::string& request_)
  {
    Service::Request request;
    request.header.mname = method_name_;
    request.request = request_;
    auto request_shared_ptr = std::make_shared<std::string>();
    SerializeToBuffer(request, *request_shared_ptr);
    return request_shared_ptr;
  }

  // Waits for the service response with a specified timeout, updating response on success or timeout
  std::pair<bool, SServiceResponse> CServiceClientIDImpl::WaitForResponse(
      SClient& client_, const std::string& method_name_,
      std::chrono::nanoseconds timeout_, std::shared_ptr<std::string> request_shared_ptr_)
  {
    auto response_data = PrepareInitialResponse(client_, method_name_);
    auto response_callback = CreateResponseCallback(response_data);
    
    const bool call_success = client_.client_session->async_call_service(request_shared_ptr_, response_callback);
    if (!call_success) return {false, PrepareErrorResponse("Call failed")};

    std::unique_lock<std::mutex> lock(*response_data->mutex);
    if (timeout_ > std::chrono::nanoseconds::zero())
    {
      if (!response_data->condition_variable->wait_for(lock, timeout_, [&] { return *response_data->finished; }))
      {
        response_data->response->first = false;
        response_data->response->second.error_msg = "Timeout";
        response_data->response->second.call_state = eCallState::call_state_timeouted;
      }
    }
    else
    {
      response_data->condition_variable->wait(lock, [&] { return *response_data->finished; });
    }

    return *response_data->response;
  }

  std::shared_ptr<CServiceClientIDImpl::SResponseData> CServiceClientIDImpl::PrepareInitialResponse(SClient& client_, const std::string& method_name_)
  {
    auto data = std::make_shared<SResponseData>();
    data->response->first = false;
    data->response->second.host_name    = client_.service_attr.hname;
    data->response->second.service_name = client_.service_attr.sname;
    data->response->second.service_id   = client_.service_attr.key;
    data->response->second.method_name  = method_name_;
    data->response->second.call_state   = eCallState::call_state_none;
    return data;
  }

  eCAL::service::ClientResponseCallbackT CServiceClientIDImpl::CreateResponseCallback(std::shared_ptr<SResponseData> response_data_)
  {
    return [response_data_](const eCAL::service::Error& error, const std::shared_ptr<std::string>& response_)
    {
      std::lock_guard<std::mutex> lock(*response_data_->mutex);
      if (!*response_data_->block_modifying_response)
      {
        if (error)
        {
          response_data_->response->first = false;
          response_data_->response->second.error_msg  = error.ToString();
          response_data_->response->second.call_state = eCallState::call_state_failed;
          response_data_->response->second.ret_state  = 0;
        }
        else
        {
          response_data_->response->first  = true;
          response_data_->response->second = DeserializedResponse(*response_);
        }
      }
      *response_data_->finished = true;
      response_data_->condition_variable->notify_all();
    };
  }

  SServiceResponse CServiceClientIDImpl::PrepareErrorResponse(const std::string& error_message_)
  {
    SServiceResponse error_response;
    error_response.error_msg  = error_message_;
    error_response.ret_state  = 0;
    error_response.call_state = eCallState::call_state_failed;
    return error_response;
  }

  void CServiceClientIDImpl::IncrementMethodCallCount(const std::string& method_name_)
  {
    std::lock_guard<std::mutex> lock(m_method_information_map_sync);
    m_method_call_count_map[method_name_]++;
  }

  // Check if a specific service is connected
  bool CServiceClientIDImpl::IsConnected(const Registration::SEntityId& entity_id_)
  {
    std::lock_guard<std::mutex> lock(m_client_session_map_sync);
    auto iter = m_client_session_map.find(entity_id_);
    return (iter != m_client_session_map.end() && iter->second.connected);
  }

  void CServiceClientIDImpl::RegisterService(const Registration::SEntityId& entity_id_, const SServiceAttr& service_)
  {
    std::lock_guard<std::mutex> lock(m_client_session_map_sync);

    if (m_client_session_map.find(entity_id_) == m_client_session_map.end())
    {
      SClient client;
      client.service_attr = service_;
      auto client_manager = eCAL::service::ServiceManager::instance()->get_client_manager();
      if (client_manager == nullptr || client_manager->is_stopped()) return;

      // Event callback (unused)
      const eCAL::service::ClientSession::EventCallbackT event_callback = [] (eCAL::service::ClientEventType /*event*/, const std::string& /*message*/) -> void
        {
          // TODO: Replace current connect/disconnect state logic with this client event callback logic
        };

      const auto protocol_version = (service_.tcp_port_v1 != 0 ? service_.version : 0);
      const auto port_to_use = (protocol_version == 0 ? service_.tcp_port_v0 : service_.tcp_port_v1);

      const std::vector<std::pair<std::string, uint16_t>> endpoint_list
      {
        {service_.hname, port_to_use},
        {service_.hname + ".local", port_to_use},
      };
      client.client_session = client_manager->create_client(static_cast<uint8_t>(protocol_version), endpoint_list, event_callback);

      if (client.client_session)
      {
        m_client_session_map.insert({ entity_id_, client });
      }
    }
  }

  // Retrieves registration information for the client
  Registration::Sample CServiceClientIDImpl::GetRegistration()
  {
    UpdateConnectionStates();
    return GetRegistrationSample();
  }

  Registration::Sample CServiceClientIDImpl::GetRegistrationSample()
  {
    Registration::Sample ecal_reg_sample;
    ecal_reg_sample.cmd_type = bct_reg_client;

    auto& service_identifier = ecal_reg_sample.identifier;
    service_identifier.entity_id  = m_client_id;
    service_identifier.process_id = Process::GetProcessID();
    service_identifier.host_name  = Process::GetHostName();

    auto& service_client = ecal_reg_sample.client;
    service_client.version = m_client_version;
    service_client.pname   = Process::GetProcessName();
    service_client.uname   = Process::GetUnitName();
    service_client.sname   = m_service_name;

    std::lock_guard<std::mutex> lock(m_method_information_map_sync);
    for (const auto& method_information_pair : m_method_information_map)
    {
      const auto& method_name = method_information_pair.first;
      const auto& method_information = method_information_pair.second;

      Service::Method method;
      method.mname      = method_name;
      method.req_type   = method_information.request_type.name;
      method.req_desc   = method_information.request_type.descriptor;
      method.resp_type  = method_information.response_type.name;
      method.resp_desc  = method_information.response_type.descriptor;
      {
        const auto& call_count_iter = m_method_call_count_map.find(method_name);
        if (call_count_iter != m_method_call_count_map.end())
        {
          method.call_count = call_count_iter->second;
        }
      }
      service_client.methods.push_back(method);
    }
    return ecal_reg_sample;
  }

  Registration::Sample CServiceClientIDImpl::GetUnregistrationSample()
  {
    Registration::Sample ecal_reg_sample;
    ecal_reg_sample.cmd_type = bct_unreg_client;

    auto& service_identifier = ecal_reg_sample.identifier;
    service_identifier.entity_id  = m_client_id;
    service_identifier.process_id = Process::GetProcessID();
    service_identifier.host_name  = Process::GetHostName();

    auto& service_client = ecal_reg_sample.client;
    service_client.version = m_client_version;
    service_client.pname   = Process::GetProcessName();
    service_client.uname   = Process::GetUnitName();
    service_client.sname   = m_service_name;

    return ecal_reg_sample;
  }

  // Register client information
  void CServiceClientIDImpl::Register()
  {
    if (!m_service_name.empty() && g_registration_provider() != nullptr)
    {
      g_registration_provider()->RegisterSample(GetRegistrationSample());
    }
  }

  // Unregister client information
  void CServiceClientIDImpl::Unregister()
  {
    if (!m_service_name.empty() && g_registration_provider() != nullptr)
    {
      g_registration_provider()->UnregisterSample(GetUnregistrationSample());
    }
  }

  // Updates the connection states for the client sessions
  void CServiceClientIDImpl::UpdateConnectionStates()
  {
    std::lock_guard<std::mutex> lock(m_client_session_map_sync);

    for (auto it = m_client_session_map.begin(); it != m_client_session_map.end(); )
    {
      auto& client_data = it->second;
      auto state = client_data.client_session->get_state();

      if (!client_data.connected && state == eCAL::service::State::CONNECTED)
      {
        client_data.connected = true;
        NotifyEventCallback(client_event_connected, client_data.service_attr);
        ++it;
      }
      else if (client_data.connected && state == eCAL::service::State::FAILED)
      {
        client_data.connected = false;
        NotifyEventCallback(client_event_disconnected, client_data.service_attr);
        it = m_client_session_map.erase(it);
      }
      else
      {
        ++it;
      }
    }
  }

  // Helper function to notify event callback
  void CServiceClientIDImpl::NotifyEventCallback(eCAL_Client_Event event_type_, const SServiceAttr& service_attr_)
  {
    std::lock_guard<std::mutex> lock(m_event_callback_map_sync);
    auto callback_it = m_event_callback_map.find(event_type_);
    if (callback_it != m_event_callback_map.end())
    {
      SClientEventCallbackData callback_data;
      callback_data.type = event_type_;
      callback_data.time = std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::steady_clock::now().time_since_epoch()).count();
      callback_data.attr = service_attr_;

      Registration::SServiceId service_id;
      service_id.service_id.entity_id  = service_attr_.sid;
      service_id.service_id.process_id = service_attr_.pid;
      service_id.service_id.host_name  = service_attr_.hname;
      service_id.service_name          = m_service_name;
      service_id.method_name           = "";

      callback_it->second(service_id, &callback_data);
    }
  }

  //void CServiceClientIDImpl::ErrorCallback(const Registration::SEntityId& entity_id_,
  //                                         const std::string& method_name_, const std::string& error_message_)
  //{
  //  std::lock_guard<std::mutex> lock(m_response_callback_sync);
  //  if (m_response_callback)
  //  {
  //    SServiceResponse service_response;
  //    service_response.call_state  = call_state_failed;
  //    service_response.error_msg   = error_message_;
  //    service_response.ret_state   = 0;
  //    service_response.method_name = method_name_;
  //    service_response.response.clear();
  //    m_response_callback(entity_id_, service_response);
  //  }
  //}
}
