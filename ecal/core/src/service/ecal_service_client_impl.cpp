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

#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace eCAL
{
  std::shared_ptr<CServiceClientImpl> CServiceClientImpl::CreateInstance(const std::string& service_name_, const ServiceMethodInformationMapT& method_information_map_)
  {
    auto instance = std::shared_ptr<CServiceClientImpl>(new CServiceClientImpl(service_name_, method_information_map_));

    // set service name and methods
    m_service_name = service_name_;
    m_method_information_map = method_information_map_;

    // initialize call count map
    for (const auto& method_information_pair : m_method_information_map)
    {
      m_method_call_count_map[method_information_pair.first] = 0;
    }

    // create client id
    std::stringstream counter;
    counter << std::chrono::steady_clock::now().time_since_epoch().count();
    m_client_id = counter.str();

    // register this client
    Register();

    return instance;
  }

  CServiceClientImpl::~CServiceClientImpl()
  {
    // reset client map
    {
      std::lock_guard<std::mutex> const lock(m_client_session_map_sync);
      m_client_session_map.clear();
    }

    // reset method callback map
    {
      std::lock_guard<std::mutex> const lock(m_response_callback_sync);
      m_response_callback = nullptr;
    }

    // reset event callback map
    {
      std::lock_guard<std::mutex> const lock(m_event_callback_map_sync);
      m_event_callback_map.clear();
    }

    // and unregister this client
    Unregister();

    // reset internals
    m_service_name.clear();
    m_client_id.clear();
  }

  // add callback function for service response
  bool CServiceClientImpl::AddResponseCallback(const ResponseCallbackT& callback_)
  {
    std::lock_guard<std::mutex> const lock(m_response_callback_sync);
    m_response_callback = callback_;
    return true;
  }

  // remove callback function for service response
  bool CServiceClientImpl::RemoveResponseCallback()
  {
    std::lock_guard<std::mutex> const lock(m_response_callback_sync);
    m_response_callback = nullptr;
    return true;
  }

  // add callback function for client events
  bool CServiceClientImpl::AddEventCallback(eCAL_Client_Event type_, ClientEventCallbackT callback_)
  {
    // store event callback
    std::lock_guard<std::mutex> const lock(m_event_callback_map_sync);
#ifndef NDEBUG
    // log it
    Logging::Log(log_level_debug2, m_service_name + "::CServiceClientImpl::AddEventCallback");
#endif
    m_event_callback_map[type_] = std::move(callback_);
    return true;
  }

  // remove callback function for client events
  bool CServiceClientImpl::RemoveEventCallback(eCAL_Client_Event type_)
  {
    // reset event callback
    std::lock_guard<std::mutex> const lock(m_event_callback_map_sync);
#ifndef NDEBUG
    // log it
    Logging::Log(log_level_debug2, m_service_name + "::CServiceClientImpl::RemEventCallback");
#endif
    m_event_callback_map[type_] = nullptr;
    return true;
  }

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

  // blocking call specific service, response will be returned as pair<bool, SServiceReponse>
  std::pair<bool, SServiceResponse> CServiceClientImpl::CallWithResponse(const Registration::SEntityId& entity_id_, const std::string& method_name_, const std::string& request_, int timeout_ms_)
  {
    return std::pair<bool, SServiceResponse>(false, {});
  }

  // blocking call specific service, using callback
  bool CServiceClientImpl::CallWithCallback(const Registration::SEntityId& entity_id_, const std::string& method_name_, const std::string& request_, int timeout_ms_)
  {
    return false;
  }

  // asynchronously call specific service, using callback (timeout not supported yet)
  bool CServiceClientImpl::CallAsyncWithCallback(const Registration::SEntityId& entity_id_, const std::string& method_name_, const std::string& request_ /*, int timeout_ms_*/)
  {
    return false;
  }

  // check connection state of specific service
  bool CServiceClientImpl::IsConnected(const Registration::SEntityId& entity_id_)
  {
    return false;
  }

  // called by the registration receiver to process a service registration
  void CServiceClientImpl::RegisterService(const Registration::SEntityId& entity_id_, const SServiceAttr& service_)
  {
    // lock client map
    const std::lock_guard<std::mutex> lock(m_client_session_map_sync);

    // add service if it's a new registration
    if (m_client_session_map.find(entity_id_) == m_client_session_map.end())
    {
      // new client
      SClient client;

      // apply attributes
      client.service_attr = service_;

      // create client session
      auto client_manager = eCAL::service::ServiceManager::instance()->get_client_manager();
      if (client_manager == nullptr || client_manager->is_stopped()) return;

      // Event callback (unused)
      const eCAL::service::ClientSession::EventCallbackT event_callback
        = [/*this, service_ = iter*/] // Using the this pointer here is extremely unsafe, as it actually forces us to manage the lifetime of this object. UPDATE: this class now inherits from shared_from_this, so when implementing this function, we can store a weak_ptr to this class.
        (eCAL::service::ClientEventType /*event*/, const std::string& /*message*/) -> void
        {
          // I have no idea why, but for some reason the event callbacks of the actual connetions are not even used. The connect / disconnect callbacks are executed whenever a new connection is found, and not when the client has actually connected or disconnected. I am preserving the previous behavior.
        };

      // Only connect via V0 protocol / V0 port, if V1 port is not available
      const auto protocol_version = (service_.tcp_port_v1 != 0 ? service_.version : 0);
      const auto port_to_use = (protocol_version == 0 ? service_.tcp_port_v0 : service_.tcp_port_v1);

      // Create the client and add it to the map
      const std::vector<std::pair<std::string, uint16_t>> endpoint_list
      {
        {service_.hname, port_to_use},
        {service_.hname + ".local", port_to_use},   // TODO: Make this configurable from the ecal.yaml
      };
      client.client_session = client_manager->create_client(static_cast<uint8_t>(protocol_version), endpoint_list, event_callback);

      // insert new client if creation was successful
      if (client.client_session)
      {
        m_client_session_map.insert({ entity_id_, client });
      }
    }
  }

  // called by eCAL:CClientGate every second to update registration layer
  Registration::Sample CServiceClientImpl::GetRegistration()
  {
    return GetRegistrationSample();
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

    {
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
        method.call_count = m_method_call_count_map.at(method_name);
        service_client.methods.push_back(method);
      }
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
    service_client.pname   = Process::GetProcessName();
    service_client.uname   = Process::GetUnitName();
    service_client.sname   = m_service_name;
    service_client.version = m_client_version;

    return ecal_reg_sample;
  }

  void CServiceClientImpl::Register()
  {
    if (m_service_name.empty()) return;

    // register entity
    if (g_registration_provider() != nullptr) g_registration_provider()->RegisterSample(GetRegistrationSample());
  }

  void CServiceClientImpl::Unregister()
  {
    if (m_service_name.empty()) return;

    // unregister entity
    if (g_registration_provider() != nullptr) g_registration_provider()->UnregisterSample(GetUnregistrationSample());
  }

  void CServiceClientImpl::UpdateConnectionStates()
  {
    std::lock_guard<std::mutex> const lock(m_client_session_map_sync);

    for (auto it = m_client_session_map.begin(); it != m_client_session_map.end(); )
    {
      auto& client_data = it->second;

      // get current state of the client session
      auto state = client_data.client_session->get_state();

      // Transition from not connected to connected
      if (!client_data.connected && state == eCAL::service::State::CONNECTED)
      {
        client_data.connected = true;

        // call connect event
        {
          std::lock_guard<std::mutex> const lock_cb(m_event_callback_map_sync);
          auto e_iter = m_event_callback_map.find(client_event_connected);
          if (e_iter != m_event_callback_map.end())
          {
            SClientEventCallbackData sdata;
            sdata.type = client_event_connected;
            sdata.time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
            sdata.attr = client_data.service_attr;
            (e_iter->second)(m_service_name.c_str(), &sdata);
          }
        }

        // move to the next client
        ++it;
      }
      // transition from connected to not connected
      else if (client_data.connected && state == eCAL::service::State::FAILED)
      {
        client_data.connected = false;

        // call disconnect event
        {
          std::lock_guard<std::mutex> const lock_cb(m_event_callback_map_sync);
          auto e_iter = m_event_callback_map.find(client_event_disconnected);
          if (e_iter != m_event_callback_map.end())
          {
            SClientEventCallbackData sdata;
            sdata.type = client_event_disconnected;
            sdata.time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
            sdata.attr = client_data.service_attr;
            (e_iter->second)(m_service_name.c_str(), &sdata);
          }
        }

        // remove client and advance iterator
        it = m_client_session_map.erase(it);
      }
      else
      {
        // no state change; move to the next client
        ++it;
      }
    }
  }

  void CServiceClientImpl::ErrorCallback(const Registration::SEntityId& entity_id_, const std::string& method_name_, const std::string& error_message_)
  {
    std::lock_guard<std::mutex> const lock(m_response_callback_sync);
    if (m_response_callback)
    {
      SServiceResponse service_response;
      service_response.call_state = call_state_failed;
      service_response.error_msg = error_message_;
      service_response.ret_state = 0;
      service_response.method_name = method_name_;
      service_response.response.clear();
      m_response_callback(entity_id_, service_response);
    }
  }

  void CServiceClientImpl::IncrementMethodCallCount(const std::string& method_name_)
  {
    const std::lock_guard<std::mutex> lock(m_method_information_map_sync);
    if (m_method_information_map.find(method_name_) == m_method_information_map.end())
    {
      m_method_information_map[method_name_] = SServiceMethodInformation();
      m_method_call_count_map[method_name_] = 0;
    }
    ++m_method_call_count_map.at(method_name_);
  }
}
