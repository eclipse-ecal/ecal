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
 * @brief  eCAL service client implementation
**/

#include "ecal_global_accessors.h"

#include "ecal_service_client_impl.h"
#include "ecal_service_singleton_manager.h"
#include "registration/ecal_registration_provider.h"
#include "serialization/ecal_serialize_service.h"

#include <chrono>
#include <memory>
#include <mutex>
#include <string>
#include <utility>

namespace
{
  // Serializes the request data into a protocol buffer and returns a shared pointer to it
  std::shared_ptr<std::string> SerializeRequest(const std::string& method_name_, const std::string& request_)
  {
    eCAL::Service::Request request;
    request.header.method_name = method_name_;
    request.request = request_;
    auto request_shared_ptr = std::make_shared<std::string>();
    eCAL::SerializeToBuffer(request, *request_shared_ptr);
    return request_shared_ptr;
  }

  eCAL::SServiceResponse CreateErrorResponse(const eCAL::SEntityId& entity_id_, const std::string& service_name_, const std::string& method_name_, const std::string& error_message_)
  {
    eCAL::SServiceResponse error_response;
    // service/method id
    error_response.server_id.service_id   = entity_id_;
    error_response.server_id.service_name = service_name_;
    error_response.service_method_information.method_name  = method_name_;
    // TODO we need to fill SDatatypeInformation

    // error message, return state and call state
    error_response.error_msg                      = error_message_;
    error_response.ret_state                      = 0;
    error_response.call_state                     = eCAL::eCallState::failed;
    return error_response;
  }

  void ResponseError(const eCAL::SEntityId& entity_id_, const std::string& service_name_, const std::string& method_name_,
    const std::string& error_message_, const eCAL::ResponseCallbackT& response_callback_)
  {
    eCAL::Logging::Log(eCAL::Logging::log_level_error, "CServiceClientImpl: Response error for service: " + service_name_ + ", method: " + method_name_ + ", error: " + error_message_);
    response_callback_(CreateErrorResponse(entity_id_, service_name_, method_name_, error_message_));
  }
}

namespace eCAL
{
    // Factory method to create a new instance of CServiceClientImpl
    std::shared_ptr<CServiceClientImpl> CServiceClientImpl::CreateInstance(
        const std::string & service_name_, const ServiceMethodInformationSetT & method_information_map_, const ClientEventCallbackT & event_callback_)
    {
  #ifndef NDEBUG
      eCAL::Logging::Log(eCAL::Logging::log_level_debug2, "CServiceClientImpl::CreateInstance: Creating instance of CServiceClientImpl for service: " + service_name_);
  #endif
      return std::shared_ptr<CServiceClientImpl>(new CServiceClientImpl(service_name_, method_information_map_, event_callback_));
    }

  // Constructor: Initializes client ID, method call counts, and registers the client
  CServiceClientImpl::CServiceClientImpl(
      const std::string & service_name_, const ServiceMethodInformationSetT & method_information_set_, const ClientEventCallbackT & event_callback_)
      : m_service_name(service_name_), m_method_information_set(method_information_set_)
  {
#ifndef NDEBUG
    eCAL::Logging::Log(eCAL::Logging::log_level_debug2, "CServiceClientImpl::CServiceClientImpl: Initializing service client for: " + service_name_);
#endif

    // initialize method call counts
    for (const auto& method_information : m_method_information_set)
    {
      m_method_call_count_map[method_information.method_name] = 0;
    }

    // create unique client ID
    m_client_id = std::chrono::steady_clock::now().time_since_epoch().count();

    // add event callback
    {
      const std::lock_guard<std::mutex> lock(m_event_callback_mutex);
      m_event_callback = event_callback_;
    }

    // Send registration sample
    if (!m_service_name.empty() && g_registration_provider() != nullptr)
    {
      g_registration_provider()->RegisterSample(GetRegistrationSample());
#ifndef NDEBUG
      eCAL::Logging::Log(eCAL::Logging::log_level_debug2, "CServiceClientImpl::CServiceClientImpl: Registered client with service name: " + m_service_name);
#endif
    }
  }

  // Destructor: Resets callbacks, unregisters the client, and clears data
  CServiceClientImpl::~CServiceClientImpl()
  {
#ifndef NDEBUG
    eCAL::Logging::Log(eCAL::Logging::log_level_debug2, "CServiceClientImpl::~CServiceClientImpl: Destroying service client for: " + m_service_name);
#endif

    // reset client map
    {
      const std::lock_guard<std::mutex> lock(m_client_session_map_mutex);
      m_client_session_map.clear();
    }

    // reset event callback
    {
      const std::lock_guard<std::mutex> lock(m_event_callback_mutex);
      m_event_callback = nullptr;
    }

    // Send unregistration sample
    if (g_registration_provider() != nullptr)
    {
      g_registration_provider()->UnregisterSample(GetUnregistrationSample());
#ifndef NDEBUG
      eCAL::Logging::Log(eCAL::Logging::log_level_debug2, "CServiceClientImpl::~CServiceClientImpl: Unregistered client for service name: " + m_service_name);
#endif
    }
  }

  // Retrieve service IDs of all matching services
  std::vector<SEntityId> CServiceClientImpl::GetServiceIDs()
  {
    std::vector<SEntityId> entity_vector;

    // lock client map
    const std::lock_guard<std::mutex> lock(m_client_session_map_mutex);
    // copy session entities into return vector
    for (const auto& client_session : m_client_session_map)
    {
      entity_vector.push_back(client_session.first);
    }

    return entity_vector;
  }

  // TODO: We need to reimplment this function. It makes no sense to call a service with response callback and to return a pair<bool, SServiceResponse>
  // Calls a service method synchronously, blocking until a response is received or timeout occurs
  std::pair<bool, SServiceResponse> CServiceClientImpl::CallWithCallback(
      const SEntityId& entity_id_, const std::string& method_name_,
      const std::string& request_, const ResponseCallbackT& response_callback_, int timeout_ms_)
  {
#ifndef NDEBUG
    eCAL::Logging::Log(eCAL::Logging::log_level_debug1, "CServiceClientImpl::CallWithCallback: Performing synchronous call for service: " + m_service_name + ", method: " + method_name_);
#endif

    SClient client;
    if (!GetClientByEntity(entity_id_, client))
    {
      eCAL::Logging::Log(Logging::log_level_warning, "CServiceClientImpl::CallWithCallback: Failed to find client for entity ID: " + entity_id_.entity_id);
      return { false, SServiceResponse() };
    }

    auto response = CallMethodWithTimeout(entity_id_, client, method_name_, request_, std::chrono::milliseconds(timeout_ms_));

    // If a callback is provided and the call was successful, invoke the callback
    if (response_callback_ && response.first)
    {
      response_callback_(response.second);
    }

    // Handle timeout event
    if (!response.first && response.second.call_state == eCallState::timeouted)
    {
      SServiceId service_id;
      service_id.service_name = m_service_name;
      service_id.service_id = entity_id_;
      NotifyEventCallback(service_id, eClientEvent::timeout);
#ifndef NDEBUG
      eCAL::Logging::Log(eCAL::Logging::log_level_debug1, "CServiceClientImpl::CallWithCallback: Synchronous call for service: " + m_service_name + ", method: " + method_name_ + " timed out.");
#endif
    }

    return response;
  }

  // Asynchronous call to a service with a specified timeout
  bool CServiceClientImpl::CallWithCallbackAsync(const SEntityId & entity_id_, const std::string & method_name_, const std::string & request_, const ResponseCallbackT & response_callback_)
  {
#ifndef NDEBUG
    eCAL::Logging::Log(eCAL::Logging::log_level_debug2, "CServiceClientImpl::CallWithCallbackAsync: Performing asynchronous call for service: " + m_service_name + ", method: " + method_name_);
#endif

    // Retrieve the client
    SClient client;
    if (!GetClientByEntity(entity_id_, client))
    {
      eCAL::Logging::Log(Logging::log_level_warning, "CServiceClientImpl::CallWithCallbackAsync: Failed to find client for entity ID: " + entity_id_.entity_id);
      return false;
    }

    // Validate service and method names
    if (m_service_name.empty() || method_name_.empty())
    {
      ResponseError(entity_id_, m_service_name, method_name_, "Invalid service or method name.", response_callback_);
      return false;
    }

    // Serialize the request
    auto request_shared_ptr = SerializeRequest(method_name_, request_);
    if (!request_shared_ptr)
    {
      eCAL::Logging::Log(eCAL::Logging::log_level_error, "CServiceClientImpl::CallWithCallbackAsync: Request serialization failed.");
      return false;
    }

    // Prepare response data
    auto response_data = PrepareInitialResponse(client, method_name_);

    // Create the response callback
    auto response = [client, response_data, entity_id_, response_callback_](const ecal_service::Error& error, const std::shared_ptr<std::string>& response_)
      {
        const std::lock_guard<std::mutex> lock(*response_data->mutex);
        if (!*response_data->block_modifying_response)
        {
          if (error)
          {
            eCAL::Logging::Log(eCAL::Logging::log_level_error, "CServiceClientImpl::CallWithCallbackAsync: Asynchronous call returned an error: " + error.ToString());
            response_data->response->first = false;
            response_data->response->second.error_msg = error.ToString();
            response_data->response->second.call_state = eCallState::failed;
            response_data->response->second.ret_state = 0;
          }
          else
          {
#ifndef NDEBUG
            eCAL::Logging::Log(eCAL::Logging::log_level_debug1, "CServiceClientImpl::CallWithCallbackAsync: Asynchronous call succeded");
#endif
            response_data->response->first = true;
            response_data->response->second = DeserializedResponse(client, *response_);
          }
        }
        *response_data->finished = true;
        response_data->condition_variable->notify_all();

        // Invoke the user-provided callback
        response_callback_(response_data->response->second);
      };

    // Send the service call
    const bool call_success = client.client_session->async_call_service(request_shared_ptr, response);
    if (!call_success)
      return false;

    // Increment method call count
    IncrementMethodCallCount(method_name_);

    return true;
  }

  // Check if a specific service is connected
  bool CServiceClientImpl::IsConnected(const SEntityId & entity_id_)
  {
    const std::lock_guard<std::mutex> lock(m_client_session_map_mutex);
    auto iter = m_client_session_map.find(entity_id_);
    bool state((iter != m_client_session_map.end() && iter->second.connected));
#ifndef NDEBUG
    eCAL::Logging::Log(eCAL::Logging::log_level_debug2, "CServiceClientImpl::IsConnected: Returned: " + std::to_string(state));
#endif
    return state;
  }

  void CServiceClientImpl::RegisterService(const SEntityId & entity_id_, const v5::SServiceAttr & service_)
  {
    const std::lock_guard<std::mutex> lock(m_client_session_map_mutex);

    if (m_client_session_map.find(entity_id_) == m_client_session_map.end())
    {
      SClient client;
      client.service_attr = service_;
      auto client_manager = eCAL::service::ServiceManager::instance()->get_client_manager();
      if (client_manager == nullptr || client_manager->is_stopped()) return;

      // Event callback (unused)
      const ecal_service::ClientSession::EventCallbackT event_callback = [](ecal_service::ClientEventType /*event*/, const std::string& /*message*/) -> void
        {
        // TODO: Replace current connect/disconnect state logic with this client event callback logic
      };

      // use protocol version 1
      const auto protocol_version = 1;
      const auto port_to_use = service_.tcp_port_v1;

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
  Registration::Sample CServiceClientImpl::GetRegistration()
  {
#ifndef NDEBUG
    Logging::Log(eCAL::Logging::log_level_debug2, "CServiceClientImpl:::GetRegistration: Generating registration sample for: " + m_service_name);
#endif
    UpdateConnectionStates();
    return GetRegistrationSample();
  }

  SServiceId CServiceClientImpl::GetServiceId() const
  {
    SServiceId service_id;

    service_id.service_id.entity_id = m_client_id;
    service_id.service_id.process_id = Process::GetProcessID();
    service_id.service_id.host_name = Process::GetHostName();
    service_id.service_name = m_service_name;

    return service_id;
  }

  std::string CServiceClientImpl::GetServiceName() const
  {
    return m_service_name;
  }

  Registration::Sample CServiceClientImpl::GetRegistrationSample()
  {
    Registration::Sample ecal_reg_sample;
    ecal_reg_sample.cmd_type = bct_reg_client;

    auto& service_identifier = ecal_reg_sample.identifier;
    service_identifier.entity_id = m_client_id;
    service_identifier.process_id = Process::GetProcessID();
    service_identifier.host_name = Process::GetHostName();

    auto& service_client = ecal_reg_sample.client;
    service_client.version = m_client_version;
    service_client.process_name = Process::GetProcessName();
    service_client.unit_name = Process::GetUnitName();
    service_client.service_name = m_service_name;

    const std::lock_guard<std::mutex> lock(m_method_information_set_mutex);
    for (const auto& method_information : m_method_information_set)
    {
      const auto& method_name = method_information.method_name;

      Service::Method method;
      method.method_name = method_name;

      // old type and descriptor fields
      method.req_type = method_information.request_type.name;
      method.req_desc = method_information.request_type.descriptor;
      method.resp_type = method_information.response_type.name;
      method.resp_desc = method_information.response_type.descriptor;

      // new type and descriptor fields
      method.request_datatype_information = method_information.request_type;
      method.response_datatype_information = method_information.response_type;

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

  Registration::Sample CServiceClientImpl::GetUnregistrationSample()
  {
    Registration::Sample ecal_reg_sample;
    ecal_reg_sample.cmd_type = bct_unreg_client;

    auto& service_identifier = ecal_reg_sample.identifier;
    service_identifier.entity_id = m_client_id;
    service_identifier.process_id = Process::GetProcessID();
    service_identifier.host_name = Process::GetHostName();

    auto& service_client = ecal_reg_sample.client;
    service_client.version = m_client_version;
    service_client.process_name = Process::GetProcessName();
    service_client.unit_name = Process::GetUnitName();
    service_client.service_name = m_service_name;

    return ecal_reg_sample;
  }

  // Attempts to retrieve a client session for a given entity ID
  bool CServiceClientImpl::GetClientByEntity(const SEntityId & entity_id_, SClient & client_)
  {
    const std::lock_guard<std::mutex> lock(m_client_session_map_mutex);
    auto iter = m_client_session_map.find(entity_id_);
    if (iter == m_client_session_map.end())
      return false;

    client_ = iter->second;
    return true;
  }

  // Blocking call to a service with a specified timeout
  std::pair<bool, SServiceResponse> CServiceClientImpl::CallMethodWithTimeout(
    const SEntityId & entity_id_, SClient & client_, const std::string & method_name_,
    const std::string & request_, std::chrono::nanoseconds timeout_)
  {
    if (method_name_.empty())
      return { false, SServiceResponse() };

    // Serialize the request
    auto request_shared_ptr = SerializeRequest(method_name_, request_);
    if (!request_shared_ptr)
      return { false, SServiceResponse() };

    // Prepare response data
    auto response_data = PrepareInitialResponse(client_, method_name_);
    auto response_callback = CreateResponseCallback(client_, response_data);

    // Send the service call
    const bool call_success = client_.client_session->async_call_service(request_shared_ptr, response_callback);
    if (!call_success)
      return { false, CreateErrorResponse(entity_id_, m_service_name, method_name_, "Call failed") };

    // Wait for the response or timeout
    std::unique_lock<std::mutex> lock(*response_data->mutex);
    if (timeout_ > std::chrono::nanoseconds::zero())
    {
      if (!response_data->condition_variable->wait_for(lock, timeout_, [&] { return *response_data->finished; }))
      {
        response_data->response->first = false;
        response_data->response->second.error_msg = "Timeout";
        response_data->response->second.call_state = eCallState::timeouted;
      }
    }
    else
    {
      response_data->condition_variable->wait(lock, [&] { return *response_data->finished; });
    }

    // Increment method call count
    IncrementMethodCallCount(method_name_);

    return *response_data->response;
  }

  // Updates the connection states for the client sessions
  void CServiceClientImpl::UpdateConnectionStates()
  {
    const std::lock_guard<std::mutex> lock(m_client_session_map_mutex);

    for (auto it = m_client_session_map.begin(); it != m_client_session_map.end(); )
    {
      auto& client_data = it->second;
      auto state = client_data.client_session->get_state();

      SEntityId entity_id;
      entity_id.entity_id  = client_data.service_attr.sid;
      entity_id.process_id = client_data.service_attr.pid;
      entity_id.host_name  = client_data.service_attr.hname;

      SServiceId service_id;
      service_id.service_name = m_service_name;
      service_id.service_id   = entity_id;

      if (!client_data.connected && state == ecal_service::State::CONNECTED)
      {
        client_data.connected = true;
        NotifyEventCallback(service_id, eClientEvent::connected);
        ++it;
      }
      else if (client_data.connected && state == ecal_service::State::FAILED)
      {
        client_data.connected = false;
        NotifyEventCallback(service_id, eClientEvent::disconnected);
        it = m_client_session_map.erase(it);
      }
      else
      {
        ++it;
      }
    }
  }

  void CServiceClientImpl::IncrementMethodCallCount(const std::string & method_name_)
  {
    const std::lock_guard<std::mutex> lock(m_method_information_set_mutex);
    m_method_call_count_map[method_name_]++;
  }


  void CServiceClientImpl::NotifyEventCallback(const SServiceId & service_id_, eClientEvent event_type_)
  {
#ifndef NDEBUG
    Logging::Log(Logging::log_level_debug1, "CServiceClientImpl::NotifyEventCallback: Notifying event callback for: " + m_service_name + " Event Type: " + to_string(event_type_));
#endif

    const std::lock_guard<std::mutex> lock(m_event_callback_mutex);
    if (m_event_callback == nullptr) return;

    SClientEventCallbackData callback_data;
    callback_data.type = event_type_;
    // TODO: user eCAL Time!!!
    callback_data.time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    m_event_callback(service_id_, callback_data);
  }

  std::shared_ptr<CServiceClientImpl::SResponseData> CServiceClientImpl::PrepareInitialResponse(const SClient & client_, const std::string & method_name_)
  {
    auto data = std::make_shared<SResponseData>();
    data->response->first = false;

    data->response->second.server_id.service_id.entity_id = client_.service_attr.sid;
    data->response->second.server_id.service_id.process_id = client_.service_attr.pid;
    data->response->second.server_id.service_id.host_name = client_.service_attr.hname;

    data->response->second.server_id.service_name = client_.service_attr.sname;
    data->response->second.service_method_information.method_name = method_name_;
    // TODO we need to fill SDatatypeInformation

    data->response->second.call_state = eCallState::none;
    return data;
  }

  ecal_service::ClientResponseCallbackT CServiceClientImpl::CreateResponseCallback(const SClient & client_, const std::shared_ptr<SResponseData>&response_data_)
  {
    return [client_, response_data_](const ecal_service::Error& error, const std::shared_ptr<std::string>& response_)
    {
      const std::lock_guard<std::mutex> lock(*response_data_->mutex);
      if (!*response_data_->block_modifying_response)
      {
        if (error)
        {
          response_data_->response->first = false;
          response_data_->response->second.error_msg = error.ToString();
          response_data_->response->second.call_state = eCallState::failed;
          response_data_->response->second.ret_state = 0;
        }
        else
        {
          response_data_->response->first = true;
          response_data_->response->second = DeserializedResponse(client_, *response_);
        }
      }
      *response_data_->finished = true;
      response_data_->condition_variable->notify_all();
    };
  }

  // DeSerializes the response string into a service response
  eCAL::SServiceResponse CServiceClientImpl::DeserializedResponse(const SClient & client_, const std::string & response_pb_)
  {
    eCAL::SServiceResponse service_reponse;
    eCAL::Service::Response response;
    if (eCAL::DeserializeFromBuffer(response_pb_.c_str(), response_pb_.size(), response))
    {
      const auto& response_header = response.header;
      // service/method id
      service_reponse.server_id.service_id.entity_id = client_.service_attr.sid;
      service_reponse.server_id.service_id.process_id = client_.service_attr.pid;
      service_reponse.server_id.service_id.host_name = response_header.host_name;

      // service and method name
      service_reponse.server_id.service_name = response_header.service_name;
      service_reponse.service_method_information.method_name = response_header.method_name;
      // TODO fill in information about datatypes. Do we have them? from the other clients? we should???

      // error message and return state
      service_reponse.error_msg = response_header.error;
      service_reponse.ret_state = static_cast<int>(response.ret_state);

      switch (response_header.state)
      {
      case eCAL::Service::eMethodCallState::executed:
        service_reponse.call_state = eCallState::executed;
        break;
      case eCAL::Service::eMethodCallState::failed:
        service_reponse.call_state = eCallState::failed;
        break;
      default:
        break;
      }

      service_reponse.response = std::string(response.response.data(), response.response.size());
    }
    else
    {
      service_reponse.error_msg = "Could not parse server response";
      service_reponse.ret_state = 0;
      service_reponse.call_state = eCallState::failed;
      service_reponse.response = "";
    }

    return service_reponse;
  }
}
