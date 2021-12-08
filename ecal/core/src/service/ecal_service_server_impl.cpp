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
    m_created(false)
  {
  }

  CServiceServerImpl::CServiceServerImpl(const std::string& service_name_) :
    m_created(false)
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

    std::chrono::milliseconds registration_timeout(eCALPAR(CMN, REGISTRATION_REFRESH) * 5);
    m_connected_clients_map.set_expiration(registration_timeout);

    m_tcp_server.Create();
    m_tcp_server.Start(std::bind(&CServiceServerImpl::RequestCallback, this, std::placeholders::_1, std::placeholders::_2));

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

    // check for connected services
    std::lock_guard<std::mutex> lock(m_connected_clients_map_sync);
    return !m_connected_clients_map.empty();
  }

  // called by the eCAL::CServiceGate to register a client
  void CServiceServerImpl::RegisterClient(const std::string& key_, const SClientAttr& client_)
  {
    // check connections
    std::lock_guard<std::mutex> lock(m_connected_clients_map_sync);

    // is this a new connection ?
    if (m_connected_clients_map.find(key_) == m_connected_clients_map.end())
    {
      // call connect event
      std::lock_guard<std::mutex> lock_cb(m_event_callback_map_sync);
      auto e_iter = m_event_callback_map.find(server_event_connected);
      if (e_iter != m_event_callback_map.end())
      {
        SServerEventCallbackData sdata;
        sdata.type = server_event_connected;
        sdata.time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
        sdata.attr = client_;
        (e_iter->second)(m_service_name.c_str(), &sdata);
      }
    }

    // add / update client (time expiration !)
    m_connected_clients_map[key_] = client_;
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
    if (g_entity_register()) g_entity_register()->RegisterServer(m_service_name, m_service_id ,sample, false);

    // check for disconnected services
    {
      std::lock_guard<std::mutex> lock(m_connected_clients_map_sync);

      // last connections
      ClientAttrMapT last_connected_clients_map = m_connected_clients_map;

      // remove timeouted clients
      m_connected_clients_map.remove_deprecated();

      // did we remove some ?
      if (last_connected_clients_map.size() != m_connected_clients_map.size())
      {
        for (auto client : last_connected_clients_map)
        {
          if (m_connected_clients_map.find(client.first) == m_connected_clients_map.end())
          {
            // call disconnect event
            std::lock_guard<std::mutex> lock_cb(m_event_callback_map_sync);
            auto e_iter = m_event_callback_map.find(server_event_disconnected);
            if (e_iter != m_event_callback_map.end())
            {
              SServerEventCallbackData sdata;
              sdata.type = server_event_disconnected;
              sdata.time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
              sdata.attr = client.second;
              (e_iter->second)(m_service_name.c_str(), &sdata);
            }
          }
        }
      }
    }
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
};
