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

#include <ecal/ecal_config.h>

#include "registration/ecal_registration_provider.h"
#include "ecal_global_accessors.h"
#include "ecal_service_server_impl.h"
#include "ecal_service_singleton_manager.h"
#include "serialization/ecal_serialize_service.h"

#include <chrono>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <utility>

namespace eCAL
{
  std::shared_ptr<CServiceServerImpl> CServiceServerImpl::CreateInstance()
  {
    return std::shared_ptr<CServiceServerImpl>(new CServiceServerImpl());
  }

  std::shared_ptr<CServiceServerImpl> CServiceServerImpl::CreateInstance(const std::string& service_name_)
  {
    auto instance = std::shared_ptr<CServiceServerImpl> (new CServiceServerImpl());
    instance->Start(service_name_);
    return instance;
  }

  CServiceServerImpl::CServiceServerImpl() :
    m_created(false)
  {
  }

  CServiceServerImpl::~CServiceServerImpl()
  {
    Stop();
  }

  bool CServiceServerImpl::Start(const std::string& service_name_)
  {
    if (m_created) return(false);

    // set service name
    m_service_name = service_name_;

    // create service id
    std::stringstream counter;
    counter << std::chrono::steady_clock::now().time_since_epoch().count();
    m_service_id = counter.str();

    // Get global server manager
    auto server_manager = eCAL::service::ServiceManager::instance()->get_server_manager();

    if (!server_manager || server_manager->is_stopped())
      return false;

    // Create callback functions
    const eCAL::service::Server::EventCallbackT event_callback
            = [weak_me = std::weak_ptr<CServiceServerImpl>(shared_from_this())]
              (eCAL::service::ServerEventType event, const std::string& message)
              {
                auto me = weak_me.lock();

                eCAL_Server_Event ecal_server_event = eCAL_Server_Event::server_event_none;
                switch (event)
                {
                case eCAL::service::ServerEventType::Connected:
                  ecal_server_event = eCAL_Server_Event::server_event_connected;
                  break;
                case eCAL::service::ServerEventType::Disconnected:
                  ecal_server_event = eCAL_Server_Event::server_event_disconnected;
                  break;
                default:
                  break;
                }

                if (me)
                  me->EventCallback(ecal_server_event, message);
              };
    
    const eCAL::service::Server::ServiceCallbackT service_callback
            = [weak_me = std::weak_ptr<CServiceServerImpl>(shared_from_this())]
              (const std::shared_ptr<const std::string>& request, const std::shared_ptr<std::string>& response) -> int
              {
                auto me = weak_me.lock();
                if (me)
                  return me->RequestCallback(*request, *response);
                else
                  return -1;
              };

    // start service protocol version 0
    if (Config::IsServiceProtocolV0Enabled())
    {
      m_tcp_server_v0 = server_manager->create_server(0, 0, service_callback, true, event_callback);
    }

    // start service protocol version 1
    if (Config::IsServiceProtocolV1Enabled())
    {
      m_tcp_server_v1 = server_manager->create_server(1, 0, service_callback, true, event_callback);
    }

    // mark as created
    m_created = true;

    return(true);
  }

  bool CServiceServerImpl::Stop()
  {
    if (!m_created) return(false);

    if (m_tcp_server_v0)
      m_tcp_server_v0->stop();

    if (m_tcp_server_v1)
      m_tcp_server_v1->stop();

    // reset method callback map
    {
      std::lock_guard<std::mutex> const lock(m_method_map_sync);
      m_method_map.clear();
    }

    // reset event callback map
    {
      std::lock_guard<std::mutex> const lock(m_event_callback_map_sync);
      m_event_callback_map.clear();
    }

    // mark as no more created
    m_created = false;

    // and unregister this service
    Unregister();

    // reset internals
    m_service_name.clear();
    m_service_id.clear();

    {
      const std::lock_guard<std::mutex> connected_lock(m_connected_mutex);
      m_connected_v0 = false;
      m_connected_v1 = false;
    }

    return(true);
  }

  bool CServiceServerImpl::AddDescription(const std::string& method_, const SDataTypeInformation& request_type_information_, const SDataTypeInformation& response_type_information_)
  {
    {
      std::lock_guard<std::mutex> const lock(m_method_map_sync);
      auto iter = m_method_map.find(method_);
      if (iter != m_method_map.end())
      {
        iter->second.method.mname     = method_;
        iter->second.method.req_type  = request_type_information_.name;
        iter->second.method.req_desc  = request_type_information_.descriptor;
        iter->second.method.resp_type = response_type_information_.name;
        iter->second.method.resp_desc = response_type_information_.descriptor;
      }
      else
      {
        SMethod method;
        method.method.mname     = method_;
        method.method.req_type  = request_type_information_.name;
        method.method.req_desc  = request_type_information_.descriptor;
        method.method.resp_type = response_type_information_.name;
        method.method.resp_desc = response_type_information_.descriptor;
        m_method_map[method_] = method;
      }
    }

    return true;
  }

  // add callback function for server method calls
  bool CServiceServerImpl::AddMethodCallback(const std::string& method_, const std::string& req_type_, const std::string& resp_type_, const MethodCallbackT& callback_)
  {
    std::string req_desc;
    std::string resp_desc;
    {
      std::lock_guard<std::mutex> const lock(m_method_map_sync);
      auto iter = m_method_map.find(method_);
      if (iter != m_method_map.end())
      {
        // should we overwrite this ?
        iter->second.method.mname     = method_;
        iter->second.method.req_type  = req_type_;
        iter->second.method.resp_type = resp_type_;
        // set callback
        iter->second.callback = callback_;

        // read descriptors back from existing service method
        req_desc  = iter->second.method.req_desc;
        resp_desc = iter->second.method.resp_desc;
      }
      else
      {
        SMethod method;
        method.method.mname     = method_;
        method.method.req_type  = req_type_;
        method.method.resp_type = resp_type_;
        method.callback         = callback_;
        m_method_map[method_] = method;
      }
    }

    return true;
  }

  // remove callback function for server method calls
  bool CServiceServerImpl::RemMethodCallback(const std::string& method_)
  {
    std::lock_guard<std::mutex> const lock(m_method_map_sync);

    auto iter = m_method_map.find(method_);
    if (iter != m_method_map.end())
    {
      m_method_map.erase(iter);
      return true;
    }
    return false;
  }

  // add callback function for server events
  bool CServiceServerImpl::AddEventCallback(eCAL_Server_Event type_, ServerEventCallbackT callback_)
  {
    if (!m_created) return false;

    // store event callback
    {
      std::lock_guard<std::mutex> const lock(m_event_callback_map_sync);
#ifndef NDEBUG
      // log it
      Logging::Log(log_level_debug2, m_service_name + "::CServiceServerImpl::AddEventCallback");
#endif
      m_event_callback_map[type_] = std::move(callback_);
    }

    return true;
  }

  // remove callback function for server events
  bool CServiceServerImpl::RemEventCallback(eCAL_Server_Event type_)
  {
    if (!m_created) return false;

    // reset event callback
    {
      std::lock_guard<std::mutex> const lock(m_event_callback_map_sync);
#ifndef NDEBUG
      // log it
      Logging::Log(log_level_debug2, m_service_name + "::CServiceServerImpl::RemEventCallback");
#endif
      m_event_callback_map[type_] = nullptr;
    }

    return true;
  }

  // check connection state
  bool CServiceServerImpl::IsConnected()
  {
    if (!m_created) return false;

    return (m_tcp_server_v0 && m_tcp_server_v0->is_connected())
            || (m_tcp_server_v1 && m_tcp_server_v1->is_connected());
  }

  // called by the eCAL::CServiceGate to register a client
  void CServiceServerImpl::RegisterClient(const std::string& /*key_*/, const SClientAttr& /*client_*/)
  {
    // this function is just a placeholder to implement logic if a new client connects
    // currently there is no need to do so
  }

  // called by eCAL:CServiceGate every second to update registration layer
  Registration::Sample CServiceServerImpl::GetRegistration()
  {
    return GetRegistrationSample();
  }

  Registration::Sample CServiceServerImpl::GetRegistrationSample()
  {
    // create registration sample
    Registration::Sample ecal_reg_sample;
    ecal_reg_sample.cmd_type = bct_reg_service;

    // might be zero in contruction phase
    unsigned short const server_tcp_port_v0(m_tcp_server_v0 ? m_tcp_server_v0->get_port() : 0);
    if ((Config::IsServiceProtocolV0Enabled()) && (server_tcp_port_v0 == 0)) return ecal_reg_sample;

    unsigned short const server_tcp_port_v1(m_tcp_server_v1 ? m_tcp_server_v1->get_port() : 0);
    if ((Config::IsServiceProtocolV1Enabled()) && (server_tcp_port_v1 == 0)) return ecal_reg_sample;

    auto& service       = ecal_reg_sample.service;
    service.version     = m_server_version;
    service.hname       = Process::GetHostName();
    service.pname       = Process::GetProcessName();
    service.uname       = Process::GetUnitName();
    service.pid         = Process::GetProcessID();
    service.sname       = m_service_name;
    service.sid         = m_service_id;
    service.tcp_port_v0 = server_tcp_port_v0;
    service.tcp_port_v1 = server_tcp_port_v1;

    // add methods
    {
      std::lock_guard<std::mutex> const lock(m_method_map_sync);
      for (const auto& iter : m_method_map)
      {
        Service::Method method;
        method.mname      = iter.first;
        method.req_type   = iter.second.method.req_type;
        method.req_desc   = iter.second.method.req_desc;
        method.resp_type  = iter.second.method.resp_type;
        method.resp_desc  = iter.second.method.resp_desc;
        method.call_count = iter.second.method.call_count;
        service.methods.push_back(method);
      }
    }

    return ecal_reg_sample;
  }

  Registration::Sample CServiceServerImpl::GetUnregistrationSample()
  {
    // create registration sample
    Registration::Sample ecal_reg_sample;
    ecal_reg_sample.cmd_type = bct_unreg_service;

    auto& service = ecal_reg_sample.service;
    service.version = m_server_version;
    service.hname = Process::GetHostName();
    service.pname = Process::GetProcessName();
    service.uname = Process::GetUnitName();
    service.pid   = Process::GetProcessID();
    service.sname = m_service_name;
    service.sid   = m_service_id;

    return ecal_reg_sample;
  }

  void CServiceServerImpl::Register()
  {
#if ECAL_CORE_REGISTRATION
    if (g_registration_provider() != nullptr) g_registration_provider()->RegisterSample(GetRegistrationSample());

#ifndef NDEBUG
    // log it
    Logging::Log(log_level_debug4, m_service_name + "::CServiceServerImpl::Register");
#endif
#endif // ECAL_CORE_REGISTRATION
  }

  void CServiceServerImpl::Unregister()
  {
#if ECAL_CORE_REGISTRATION
    if (g_registration_provider() != nullptr) g_registration_provider()->UnregisterSample(GetUnregistrationSample());

#ifndef NDEBUG
    // log it
    Logging::Log(log_level_debug4, m_service_name + "::CServiceServerImpl::Unregister");
#endif
#endif // ECAL_CORE_REGISTRATION
  }

  int CServiceServerImpl::RequestCallback(const std::string& request_pb_, std::string& response_pb_)
  {
    // prepare response
    Service::Response response;
    auto& response_header = response.header;
    response_header.hname = Process::GetHostName();
    response_header.sname = m_service_name;
    response_header.sid   = m_service_id;

    // try to parse request
    Service::Request request;
    if (!DeserializeFromBuffer(request_pb_.c_str(), request_pb_.size(), request))
    {
      Logging::Log(log_level_error, m_service_name + "::CServiceServerImpl::RequestCallback failed to parse request message");

      response_header.state = Service::eMethodCallState::failed;
      std::string const emsg = "Service '" + m_service_name + "' request message could not be parsed.";
      response_header.error = emsg;

      // TODO: The next version of the service protocol should omit the double-serialization (i.e. copying the binary data in a protocol buffer and then serializing that again)
      // serialize response and return "request message could not be parsed"
      SerializeToBuffer(response, response_pb_);

      // Return Failed (error_code = -1), as parsing the request failed. The
      // return value is not propagated to the remote caller.
      return -1;
    }

    // get method
    SMethod method;
    const auto& request_header = request.header;
    response_header.mname = request_header.mname;
    {
      std::lock_guard<std::mutex> const lock(m_method_map_sync);

      auto requested_method_iterator = m_method_map.find(request_header.mname);
      if (requested_method_iterator == m_method_map.end())
      {
        // set method call state 'failed'
        response_header.state = Service::eMethodCallState::failed;
        // set error message
        std::string const emsg = "Service '" + m_service_name + "' has no method named '" + request_header.mname + "'";
        response_header.error = emsg;

        // TODO: The next version of the service protocol should omit the double-serialization (i.e. copying the binary data in a protocol buffer and then serializing that again)
        // serialize response and return "method not found"
        SerializeToBuffer(response, response_pb_);

        // Return Success (error_code = 0), as parsing the request worked. The
        // return value is not propagated to the remote caller.
        return 0;
      }
      else
      {
        // increase call count
        auto call_count = requested_method_iterator->second.method.call_count;
        requested_method_iterator->second.method.call_count = ++call_count;

        // store (copy) the method object, so we can release the mutex before calling the function
        method = requested_method_iterator->second;
      }
    }

    // execute method (outside lock guard)
    const std::string& request_s = request.request;
    std::string response_s;
    int const service_return_state = method.callback(method.method.mname, method.method.req_type, method.method.resp_type, request_s, response_s);

    // set method call state 'executed'
    response_header.state = Service::eMethodCallState::executed;
    // set method response and return state
    response.response  = response_s;
    response.ret_state = service_return_state;

    // TODO: The next version of the service protocol should omit the double-serialization (i.e. copying the binary data in a protocol buffer and then serializing that again)
    // serialize response and return "method not found"
    SerializeToBuffer(response, response_pb_);

    // return success (error code 0)
    return 0;
  }

  void CServiceServerImpl::EventCallback(eCAL_Server_Event event_, const std::string& /*message_*/)
  {
    bool mode_changed(false);

    {
      const std::lock_guard<std::mutex> connected_lock(m_connected_mutex);

      // protocol version 0
      if (m_connected_v0)
      {
        if (m_tcp_server_v0 && !m_tcp_server_v0->is_connected())
        {
          mode_changed   = true;
          m_connected_v0 = false;
          Logging::Log(log_level_debug2, m_service_name + ": " + "client with protocol version 0 disconnected");
        }
      }
      else
      {
        if (m_tcp_server_v0 && m_tcp_server_v0->is_connected())
        {
          mode_changed   = true;
          m_connected_v0 = true;
          Logging::Log(log_level_debug2, m_service_name + ": " + "client with protocol version 0 connected");
        }
      }

      // protocol version 1
      if (m_connected_v1)
      {
        if (m_tcp_server_v1 && !m_tcp_server_v1->is_connected())
        {
          mode_changed   = true;
          m_connected_v1 = false;
          Logging::Log(log_level_debug2, m_service_name + ": " + "client with protocol version 1 disconnected");
        }
      }
      else
      {
        if (m_tcp_server_v1 && m_tcp_server_v1->is_connected())
        {
          mode_changed   = true;
          m_connected_v1 = true;
          Logging::Log(log_level_debug2, m_service_name + ": " + "client with protocol version 1 connected");
        }
      }
    }

    if (mode_changed)
    {
      // call event
      std::lock_guard<std::mutex> const lock_cb(m_event_callback_map_sync);
      auto e_iter = m_event_callback_map.find(event_);
      if (e_iter != m_event_callback_map.end())
      {
        SServerEventCallbackData sdata;
        sdata.type = event_;
        sdata.time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
        (e_iter->second)(m_service_name.c_str(), &sdata);
      }
    }
  }
}
