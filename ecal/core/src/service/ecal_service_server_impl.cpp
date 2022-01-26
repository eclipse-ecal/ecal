/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2019 Continental Corporation
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

#include "ecal_def.h"
#include "ecal_config_hlp.h"
#include "ecal_register.h"
#include "ecal_servicegate.h"
#include "ecal_global_accessors.h"
#include "ecal_service_server_impl.h"

#include <chrono>
#include <sstream>

namespace eCAL
{
  /**
   * @brief Service server implementation class.
  **/
  CServiceServerImpl::CServiceServerImpl() :
    m_created(false), m_connected(false)
  {
  }

  CServiceServerImpl::CServiceServerImpl(const std::string& service_name_) :
    m_created(false), m_connected(false)
  {
    Create(service_name_);
  }

  CServiceServerImpl::~CServiceServerImpl()
  {
    Destroy();
  }

  bool CServiceServerImpl::Create(const std::string& service_name_)
  {
    if (m_created) return(false);

    // set service name
    m_service_name = service_name_;

    // create service id
    std::stringstream counter;
    counter << std::chrono::steady_clock::now().time_since_epoch().count();
    m_service_id = counter.str();

    m_tcp_server.Create();
    m_tcp_server.Start(std::bind(&CServiceServerImpl::RequestCallback, this, std::placeholders::_1, std::placeholders::_2),
                       std::bind(&CServiceServerImpl::EventCallback,   this, std::placeholders::_1, std::placeholders::_2));

    if (g_servicegate()) g_servicegate()->Register(this);

    m_created = true;

    return(true);
  }

  bool CServiceServerImpl::Destroy()
  {
    if (!m_created) return(false);

    m_tcp_server.Stop();
    m_tcp_server.Destroy();

    if (g_servicegate())     g_servicegate()->Unregister(this);
    if (g_entity_register()) g_entity_register()->UnregisterServer(m_service_name, m_service_id);

    // reset method callback map
    {
      std::lock_guard<std::mutex> lock(m_method_callback_map_sync);
      m_method_callback_map.clear();
    }

    // reset event callback map
    {
      std::lock_guard<std::mutex> lock(m_event_callback_map_sync);
      m_event_callback_map.clear();
    }

    m_service_name.clear();
    m_service_id.clear();

    m_connected = false;
    m_created   = false;

    return(true);
  }

  // add callback function for server method calls
  bool CServiceServerImpl::AddMethodCallback(const std::string& method_, const std::string& req_type_, const std::string& resp_type_, const MethodCallbackT& callback_)
  {
    SMethodCallback mcallback;
    mcallback.method.set_mname(method_);
    mcallback.method.set_req_type(req_type_);
    mcallback.method.set_resp_type(resp_type_);
    mcallback.callback = callback_;

    std::lock_guard<std::mutex> lock(m_method_callback_map_sync);
    m_method_callback_map[method_] = mcallback;

    return true;
  }

  // remove callback function for server method calls
  bool CServiceServerImpl::RemMethodCallback(const std::string& method_)
  {
    std::lock_guard<std::mutex> lock(m_method_callback_map_sync);

    auto iter = m_method_callback_map.find(method_);
    if (iter != m_method_callback_map.end())
    {
      m_method_callback_map.erase(iter);
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
      std::lock_guard<std::mutex> lock(m_event_callback_map_sync);
#ifndef NDEBUG
      // log it
      Logging::Log(log_level_debug2, m_service_name + "::CServiceServerImpl::AddEventCallback");
#endif
      m_event_callback_map[type_] = callback_;
    }

    return true;
  }

  // remove callback function for server events
  bool CServiceServerImpl::RemEventCallback(eCAL_Server_Event type_)
  {
    if (!m_created) return false;

    // reset event callback
    {
      std::lock_guard<std::mutex> lock(m_event_callback_map_sync);
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
    return m_tcp_server.IsConnected();
  }

  // called by the eCAL::CServiceGate to register a client
  void CServiceServerImpl::RegisterClient(const std::string& /*key_*/, const SClientAttr& /*client_*/)
  {
    // not used yet
  }

  // called by eCAL:CServiceGate every second to update registration layer
  void CServiceServerImpl::RefreshRegistration()
  {
    if (!m_created)             return;
    if (m_service_name.empty()) return;

    eCAL::pb::Sample sample;
    sample.set_cmd_type(eCAL::pb::bct_reg_service);
    auto service_mutable_service = sample.mutable_service();
    service_mutable_service->set_hname(Process::GetHostName());
    service_mutable_service->set_pname(Process::GetProcessName());
    service_mutable_service->set_uname(Process::GetUnitName());
    service_mutable_service->set_pid(Process::GetProcessID());
    service_mutable_service->set_sname(m_service_name);
    service_mutable_service->set_sid(m_service_id);
    service_mutable_service->set_tcp_port(m_tcp_server.GetTcpPort());

    {
      std::lock_guard<std::mutex> lock(m_method_callback_map_sync);
      for (auto iter : m_method_callback_map)
      {
        auto method = service_mutable_service->add_methods();
        method->set_mname(iter.first);
        method->set_req_type(iter.second.method.req_type());
        method->set_resp_type(iter.second.method.resp_type());
        method->set_call_count(iter.second.method.call_count());
      }
    }

    // register entity
    if (g_entity_register()) g_entity_register()->RegisterServer(m_service_name, m_service_id, sample, false);
  }

  int CServiceServerImpl::RequestCallback(const std::string& request_, std::string& response_)
  {
    std::lock_guard<std::mutex> lock(m_method_callback_map_sync);

    if (m_method_callback_map.empty()) return 0;

    int success(-1);
    eCAL::pb::Response response_pb;
    auto response_pb_mutable_header = response_pb.mutable_header();
    response_pb_mutable_header->set_hname(eCAL::Process::GetHostName());
    response_pb_mutable_header->set_sname(m_service_name);
    response_pb_mutable_header->set_sid(m_service_id);

    eCAL::pb::Request request_pb;
    if (request_pb.ParseFromString(request_))
    {
      // success == 0 means we could deserialize and
      // no need to try to read more bytes for the server
      // socket
      success = 0;
      auto request_pb_header = request_pb.header();
      response_pb_mutable_header->set_mname(request_pb_header.mname());

      auto iter = m_method_callback_map.find(request_pb_header.mname());
      if (iter != m_method_callback_map.end())
      {
        auto call_count = iter->second.method.call_count();
        iter->second.method.set_call_count(++call_count);

        std::string request_s = request_pb.request();
        std::string response_s;
        int service_return_state = m_method_callback_map[request_pb_header.mname()].callback(iter->second.method.mname(), iter->second.method.req_type(), iter->second.method.resp_type(), request_s, response_s);

        response_pb_mutable_header->set_state(eCAL::pb::ServiceHeader_eCallState_executed);
        response_pb.set_response(response_s);
        response_pb.set_ret_state(service_return_state);
      }
      else
      {
        response_pb_mutable_header->set_state(eCAL::pb::ServiceHeader_eCallState_failed);
        std::string emsg = "Service " + m_service_name + " has no method " + request_pb_header.mname();
        response_pb_mutable_header->set_error(emsg);
      }
      response_ = response_pb.SerializeAsString();
    }
    return success;
  }

  void CServiceServerImpl::EventCallback(eCAL_Server_Event event_, const std::string& /*message_*/)
  {
    bool mode_changed(false);
    if (m_connected)
    {
      if (!m_tcp_server.IsConnected())
      {
        mode_changed = true;
        m_connected  = false;
      }
    }
    else
    {
      if (m_tcp_server.IsConnected())
      {
        mode_changed = true;
        m_connected  = true;
      }
    }

    if (mode_changed)
    {
      // call event
      std::lock_guard<std::mutex> lock_cb(m_event_callback_map_sync);
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
};
