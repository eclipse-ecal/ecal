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
    request.header.mname = method_name_;
    request.request = request_;
    auto request_shared_ptr = std::make_shared<std::string>();
    eCAL::SerializeToBuffer(request, *request_shared_ptr);
    return request_shared_ptr;
  }

  // DeSerializes the response string into a service response
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

  eCAL::SServiceResponse CreateErrorResponse(const eCAL::Registration::SEntityId& entity_id_, const std::string& service_name_, const std::string& method_name_, const std::string& error_message_)
  {
    eCAL::SServiceResponse error_response;
    error_response.host_name    = entity_id_.host_name;
    error_response.service_name = service_name_;
    error_response.service_id   = entity_id_.entity_id;
    error_response.method_name  = method_name_;
    error_response.error_msg    = error_message_;
    error_response.ret_state    = 0;
    error_response.call_state   = call_state_failed;
    return error_response;
  }

  void ResponseError(const eCAL::Registration::SEntityId& entity_id_, const std::string& service_name_, const std::string& method_name_,
    const std::string& error_message_, const eCAL::ResponseIDCallbackT& response_callback_)
  {
    response_callback_(entity_id_, CreateErrorResponse(entity_id_, service_name_, method_name_, error_message_));
  }
}

namespace eCAL
{
  // Factory method to create a new instance of CServiceClientImpl
  std::shared_ptr<CServiceClientImpl> CServiceClientImpl::CreateInstance(
      const std::string& service_name_, const ServiceMethodInformationMapT& method_information_map_, const ClientEventIDCallbackT& event_callback_)
  {
    return std::shared_ptr<CServiceClientImpl>(new CServiceClientImpl(service_name_, method_information_map_, event_callback_));
  }

  // Constructor: Initializes client ID, method call counts, and registers the client
  CServiceClientImpl::CServiceClientImpl(
      const std::string& service_name_, const ServiceMethodInformationMapT& method_information_map_, const ClientEventIDCallbackT& event_callback_)
      : m_service_name(service_name_), m_method_information_map(method_information_map_)
  {
    // initialize method call counts
    for (const auto& method_information_pair : m_method_information_map)
    {
      m_method_call_count_map[method_information_pair.first] = 0;
    }

    // create unique client ID
    std::stringstream counter;
    counter << std::chrono::steady_clock::now().time_since_epoch().count();
    m_client_id = counter.str();

    // add event callback
    {
      const std::lock_guard<std::mutex> lock(m_event_callback_sync);
      m_event_callback = event_callback_;
    }

    // register client
    if (!m_service_name.empty() && g_registration_provider() != nullptr)
    {
      g_registration_provider()->RegisterSample(GetRegistrationSample());
    }
  }

  // Destructor: Resets callbacks, unregisters the client, and clears data
  CServiceClientImpl::~CServiceClientImpl()
  {
    // reset client map
    {
      const std::lock_guard<std::mutex> lock(m_client_session_map_sync);
      m_client_session_map.clear();
    }

    // reset event callback
    {
      const std::lock_guard<std::mutex> lock(m_event_callback_sync);
      m_event_callback = nullptr;
    }

    // unregister client
    if (g_registration_provider() != nullptr)
    {
      g_registration_provider()->UnregisterSample(GetUnregistrationSample());
    }
  }

  // Retrieve service IDs of all matching services
  std::vector<Registration::SEntityId> CServiceClientImpl::GetServiceIDs()
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
  std::pair<bool, SServiceResponse> CServiceClientImpl::CallWithCallback(
      const Registration::SEntityId& entity_id_, const std::string& method_name_,
      const std::string& request_, int timeout_ms_, const ResponseIDCallbackT& response_callback_)
  {
    SClient client;
    if (!GetClientByEntity(entity_id_, client))
      return { false, SServiceResponse() };

    auto response = CallMethodWithTimeout(entity_id_, client, method_name_, request_, std::chrono::milliseconds(timeout_ms_));

    // If a callback is provided and the call was successful, invoke the callback
    if (response_callback_ && response.first)
    {
      response_callback_(entity_id_, response.second);
    }

    // Handle timeout event
    if (!response.first && response.second.call_state == eCallState::call_state_timeouted)
    {
      NotifyEventCallback(entity_id_, eCAL_Client_Event::client_event_timeout, client.service_attr);
    }

    return response;
  }

  // Asynchronous call to a service with a specified timeout
  bool CServiceClientImpl::CallWithCallbackAsync(const Registration::SEntityId& entity_id_, const std::string& method_name_, const std::string& request_, const ResponseIDCallbackT& response_callback_)
  {
    // Retrieve the client
    SClient client;
    if (!GetClientByEntity(entity_id_, client))
      return false;

    // Validate service and method names
    if (m_service_name.empty() || method_name_.empty())
    {
      ResponseError(entity_id_, m_service_name, method_name_, "Invalid service or method name.", response_callback_);
      return false;
    }

    // Serialize the request
    auto request_shared_ptr = SerializeRequest(method_name_, request_);
    if (!request_shared_ptr)
      return false;

    // Prepare response data
    auto response_data = PrepareInitialResponse(client, method_name_);

    // Create the response callback
    auto response = [response_data, entity_id_, response_callback_](const eCAL::service::Error& error, const std::shared_ptr<std::string>& response_)
      {
        const std::lock_guard<std::mutex> lock(*response_data->mutex);
        if (!*response_data->block_modifying_response)
        {
          if (error)
          {
            response_data->response->first = false;
            response_data->response->second.error_msg = error.ToString();
            response_data->response->second.call_state = eCallState::call_state_failed;
            response_data->response->second.ret_state = 0;
          }
          else
          {
            response_data->response->first = true;
            response_data->response->second = DeserializedResponse(*response_);
          }
        }
        *response_data->finished = true;
        response_data->condition_variable->notify_all();

        // Invoke the user-provided callback
        response_callback_(entity_id_, response_data->response->second);
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
  bool CServiceClientImpl::IsConnected(const Registration::SEntityId& entity_id_)
  {
    const std::lock_guard<std::mutex> lock(m_client_session_map_sync);
    auto iter = m_client_session_map.find(entity_id_);
    return (iter != m_client_session_map.end() && iter->second.connected);
  }

  void CServiceClientImpl::RegisterService(const Registration::SEntityId& entity_id_, const SServiceAttr& service_)
  {
    const std::lock_guard<std::mutex> lock(m_client_session_map_sync);

    if (m_client_session_map.find(entity_id_) == m_client_session_map.end())
    {
      SClient client;
      client.service_attr = service_;
      auto client_manager = eCAL::service::ServiceManager::instance()->get_client_manager();
      if (client_manager == nullptr || client_manager->is_stopped()) return;

      // Event callback (unused)
      const eCAL::service::ClientSession::EventCallbackT event_callback = [](eCAL::service::ClientEventType /*event*/, const std::string& /*message*/) -> void
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
  Registration::Sample CServiceClientImpl::GetRegistration()
  {
    UpdateConnectionStates();
    return GetRegistrationSample();
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
    service_identifier.entity_id  = m_client_id;
    service_identifier.process_id = Process::GetProcessID();
    service_identifier.host_name  = Process::GetHostName();

    auto& service_client = ecal_reg_sample.client;
    service_client.version = m_client_version;
    service_client.pname   = Process::GetProcessName();
    service_client.uname   = Process::GetUnitName();
    service_client.sname   = m_service_name;

    const std::lock_guard<std::mutex> lock(m_method_information_map_sync);
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

  Registration::Sample CServiceClientImpl::GetUnregistrationSample()
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

  // Attempts to retrieve a client session for a given entity ID
  bool CServiceClientImpl::GetClientByEntity(const Registration::SEntityId& entity_id_, SClient& client_)
  {
    const std::lock_guard<std::mutex> lock(m_client_session_map_sync);
    auto iter = m_client_session_map.find(entity_id_);
    if (iter == m_client_session_map.end())
      return false;

    client_ = iter->second;
    return true;
  }

  // Blocking call to a service with a specified timeout
  std::pair<bool, SServiceResponse> CServiceClientImpl::CallMethodWithTimeout(
    const Registration::SEntityId& entity_id_, SClient& client_, const std::string& method_name_,
    const std::string& request_, std::chrono::nanoseconds timeout_)
  {
    if (method_name_.empty())
      return { false, SServiceResponse() };

    // Serialize the request
    auto request_shared_ptr = SerializeRequest(method_name_, request_);
    if (!request_shared_ptr)
      return { false, SServiceResponse() };

    // Prepare response data
    auto response_data = PrepareInitialResponse(client_, method_name_);
    auto response_callback = CreateResponseCallback(response_data);

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
        response_data->response->second.call_state = eCallState::call_state_timeouted;
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
    const std::lock_guard<std::mutex> lock(m_client_session_map_sync);

    for (auto it = m_client_session_map.begin(); it != m_client_session_map.end(); )
    {
      auto& client_data = it->second;
      auto state = client_data.client_session->get_state();

      Registration::SEntityId entity_id;
      entity_id.entity_id = client_data.service_attr.sid;
      entity_id.process_id = client_data.service_attr.pid;
      entity_id.host_name = client_data.service_attr.hname;

      if (!client_data.connected && state == eCAL::service::State::CONNECTED)
      {
        client_data.connected = true;
        NotifyEventCallback(entity_id, client_event_connected, client_data.service_attr);
        ++it;
      }
      else if (client_data.connected && state == eCAL::service::State::FAILED)
      {
        client_data.connected = false;
        NotifyEventCallback(entity_id, client_event_disconnected, client_data.service_attr);
        it = m_client_session_map.erase(it);
      }
      else
      {
        ++it;
      }
    }
  }

  void CServiceClientImpl::IncrementMethodCallCount(const std::string& method_name_)
  {
    const std::lock_guard<std::mutex> lock(m_method_information_map_sync);
    m_method_call_count_map[method_name_]++;
  }


  // Helper function to notify event callback
  void CServiceClientImpl::NotifyEventCallback(const Registration::SEntityId& entity_id_, eCAL_Client_Event event_type_, const SServiceAttr& service_attr_)
  {
    const std::lock_guard<std::mutex> lock(m_event_callback_sync);
    if (m_event_callback == nullptr) return;

    SClientEventCallbackData callback_data;
    callback_data.type = event_type_;
    callback_data.time = std::chrono::duration_cast<std::chrono::microseconds>(
      std::chrono::steady_clock::now().time_since_epoch()).count();
    callback_data.attr = service_attr_;

    Registration::SServiceId service_id;
    service_id.service_id = entity_id_;
    service_id.service_name = m_service_name;
    service_id.method_name = "";

    m_event_callback(service_id, callback_data);
  }

  std::shared_ptr<CServiceClientImpl::SResponseData> CServiceClientImpl::PrepareInitialResponse(SClient& client_, const std::string& method_name_)
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

  eCAL::service::ClientResponseCallbackT CServiceClientImpl::CreateResponseCallback(const std::shared_ptr<SResponseData>& response_data_)
  {
    return [response_data_](const eCAL::service::Error& error, const std::shared_ptr<std::string>& response_)
    {
      const std::lock_guard<std::mutex> lock(*response_data_->mutex);
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
}
