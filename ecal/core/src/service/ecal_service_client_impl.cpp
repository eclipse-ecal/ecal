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
 * @brief  eCAL service client implementation
**/

#include "ecal_register.h"
#include "ecal_clientgate.h"
#include "ecal_service_client_impl.h"

#include <chrono>
#include <sstream>

namespace eCAL
{
  /**
   * @brief Service client implementation class.
  **/
  CServiceClientImpl::CServiceClientImpl() :
    m_reply{},
    m_response_callback(nullptr),
    m_created(false)
  {
  }

  CServiceClientImpl::CServiceClientImpl(const std::string& service_name_) :
    m_reply{},
    m_response_callback(nullptr),
    m_created(false)
  {
    Create(service_name_);
  }

  CServiceClientImpl::~CServiceClientImpl()
  {
    Destroy();
  }

  bool CServiceClientImpl::Create(const std::string& service_name_)
  {
    if (m_created) return(false);

    // set service name
    m_service_name = service_name_;

    // create service id
    std::stringstream counter;
    counter << std::chrono::steady_clock::now().time_since_epoch().count();
    m_service_id = counter.str();

    if (g_clientgate()) g_clientgate()->Register(this);

    m_created = true;

    return(true);
  }

  bool CServiceClientImpl::Destroy()
  {
    if (!m_created) return(false);

    if (g_clientgate())      g_clientgate()->Unregister(this);
    if (g_entity_register()) g_entity_register()->UnregisterClient(m_service_name, m_service_id);

    // reset client map
    {
      std::lock_guard<std::mutex> lock(m_client_map_sync);
      m_client_map.clear();
    }

    // reset method callback map
    {
      std::lock_guard<std::mutex> lock(m_response_callback_sync);
      m_response_callback = nullptr;
    }

    // reset event callback map
    {
      std::lock_guard<std::mutex> lock(m_event_callback_map_sync);
      m_event_callback_map.clear();
    }

    m_service_name.clear();
    m_service_id.clear();
    m_host_name.clear();

    m_created = false;

    return(true);
  }

  bool CServiceClientImpl::SetHostName(const std::string& host_name_)
  {
    if (host_name_ == "*") m_host_name.clear();
    else                   m_host_name = host_name_;
    return(true);
  }

  // add callback function for service response
  bool CServiceClientImpl::AddResponseCallback(const ResponseCallbackT& callback_)
  {
    std::lock_guard<std::mutex> lock(m_response_callback_sync);
    m_response_callback = callback_;
    return true;
  }

  // remove callback function for service response
  bool CServiceClientImpl::RemResponseCallback()
  {
    std::lock_guard<std::mutex> lock(m_response_callback_sync);
    m_response_callback = nullptr;
    return true;
  }

  // add callback function for client events
  bool CServiceClientImpl::AddEventCallback(eCAL_Client_Event type_, ClientEventCallbackT callback_)
  {
    if (!m_created) return false;

    // store event callback
    {
      std::lock_guard<std::mutex> lock(m_event_callback_map_sync);
#ifndef NDEBUG
      // log it
      Logging::Log(log_level_debug2, m_service_name + "::CServiceClientImpl::AddEventCallback");
#endif
      m_event_callback_map[type_] = callback_;
    }

    return true;
  }

  // remove callback function for client events
  bool CServiceClientImpl::RemEventCallback(eCAL_Client_Event type_)
  {
    if (!m_created) return false;

    // reset event callback
    {
      std::lock_guard<std::mutex> lock(m_event_callback_map_sync);
#ifndef NDEBUG
      // log it
      Logging::Log(log_level_debug2, m_service_name + "::CServiceClientImpl::RemEventCallback");
#endif
      m_event_callback_map[type_] = nullptr;
    }

    return true;
  }

  // blocking call, no broadcast, first matching service only, response will be returned in service_response_
  [[deprecated]]
  bool CServiceClientImpl::Call(const std::string& method_name_, const std::string& request_, struct SServiceResponse& service_response_)
  {
    if (!g_clientgate()) return false;
    if (!m_created)      return false;

    if (m_service_name.empty()
      || method_name_.empty()
      )
      return false;

    // check for new server
    CheckForNewServices();

    std::vector<SServiceAttr> service_vec = g_clientgate()->GetServiceAttr(m_service_name);
    for (auto& iter : service_vec)
    {
      if (m_host_name.empty() || (m_host_name == iter.hname))
      {
        std::lock_guard<std::mutex> lock(m_client_map_sync);
        auto client = m_client_map.find(iter.key);
        if (client != m_client_map.end())
        {
          if (SendRequest(client->second, method_name_, request_, -1, service_response_))
          {
            return true;
          }
        }
      }
    }
    return false;
  }

  // blocking call, all responses will be returned in service_response_vec_
  bool CServiceClientImpl::Call(const std::string& method_name_, const std::string& request_, int timeout_, ServiceResponseVecT* service_response_vec_)
  {
    if (!g_clientgate()) return false;
    if (!m_created)      return false;

    if (m_service_name.empty()
      || method_name_.empty()
      )
      return false;

    // reset response
    if(service_response_vec_) service_response_vec_->clear();

    // check for new server
    CheckForNewServices();

    bool called(false);
    std::vector<SServiceAttr> service_vec = g_clientgate()->GetServiceAttr(m_service_name);
    for (auto& iter : service_vec)
    {
      if (m_host_name.empty() || (m_host_name == iter.hname))
      {
        std::lock_guard<std::mutex> lock(m_client_map_sync);
        auto client = m_client_map.find(iter.key);
        if (client != m_client_map.end())
        {
          struct SServiceResponse service_response;
          if (SendRequest(client->second, method_name_, request_, timeout_, service_response))
          {
            if(service_response_vec_) service_response_vec_->push_back(service_response);
            called = true;
          }
        }
      }
    }
    return called;
  }

  // blocking call, using callback
  bool CServiceClientImpl::Call(const std::string& method_name_, const std::string& request_, int timeout_)
  {
    if (!g_clientgate()) return false;
    if (!m_created)      return false;

    if (m_service_name.empty()
      || method_name_.empty()
      )
      return false;

    // check for new server
    CheckForNewServices();

    // send request to every single service
    return SendRequests(m_host_name, method_name_, request_, timeout_);
  }

  // asynchronously call, using callback
  bool CServiceClientImpl::CallAsync(const std::string& method_name_, const std::string& request_ /*, int timeout_*/)
  {
    // TODO: implement timeout

    if (!g_clientgate())
    {
      ErrorCallback(method_name_, "Clientgate error.");
      return false;
    }

    if (!m_created)
    {
      ErrorCallback(method_name_, "Client hasn't been created yet.");
      return false;
    }

    if (m_service_name.empty()
      || method_name_.empty())
    {
      ErrorCallback(method_name_, "Invalid service or method name.");
      return false;
    }

    // check for new server
    CheckForNewServices();

    bool called(false);
    std::vector<SServiceAttr> service_vec = g_clientgate()->GetServiceAttr(m_service_name);
    for (auto& iter : service_vec)
    {
      if (m_host_name.empty() || (m_host_name == iter.hname))
      {
        std::lock_guard<std::mutex> lock(m_client_map_sync);
        auto client = m_client_map.find(iter.key);
        if (client != m_client_map.end())
        {
          SendRequestAsync(client->second, method_name_, request_ /*, timeout_*/, -1);
          called = true;
        }
      }
    }
    return(called);
  }

  // check connection state
  bool CServiceClientImpl::IsConnected()
  {
    if (!m_created) return false;

    // check for connected clients
    std::lock_guard<std::mutex> lock(m_connected_services_map_sync);
    return !m_connected_services_map.empty();
  }

  // called by the eCAL::CClientGate to register a service
  void CServiceClientImpl::RegisterService(const std::string& key_, const SServiceAttr& service_)
  {
    // check connections
    std::lock_guard<std::mutex> lock(m_connected_services_map_sync);

    // is this a new connection ?
    if (m_connected_services_map.find(key_) == m_connected_services_map.end())
    {
      // call connect event
      std::lock_guard<std::mutex> lock_eb(m_event_callback_map_sync);
      auto e_iter = m_event_callback_map.find(client_event_connected);
      if (e_iter != m_event_callback_map.end())
      {
        SClientEventCallbackData sdata;
        sdata.type = client_event_connected;
        sdata.time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
        sdata.attr = service_;
        (e_iter->second)(m_service_name.c_str(), &sdata);
      }
      // add service
      m_connected_services_map[key_] = service_;
    }
  }

  // called by eCAL:CClientGate every second to update registration layer
  void CServiceClientImpl::RefreshRegistration()
  {
    if (!m_created)             return;
    if (m_service_name.empty()) return;

    eCAL::pb::Sample sample;
    sample.set_cmd_type(eCAL::pb::bct_reg_client);
    auto service_mutable_client = sample.mutable_client();
    service_mutable_client->set_hname(Process::GetHostName());
    service_mutable_client->set_pname(Process::GetProcessName());
    service_mutable_client->set_uname(Process::GetUnitName());
    service_mutable_client->set_pid(Process::GetProcessID());
    service_mutable_client->set_sname(m_service_name);
    service_mutable_client->set_sid(m_service_id);

    // register entity
    if (g_entity_register()) g_entity_register()->RegisterClient(m_service_name, m_service_id, sample, false);

    // refresh connected services map
    CheckForNewServices();

    // check for disconnected services
    {
      std::lock_guard<std::mutex> lock(m_client_map_sync);
      for (auto& client : m_client_map)
      {
        if (!client.second->IsConnected())
        {
          std::string service_key = client.first;

          // is the service still in the connecting map ?
          auto iter = m_connected_services_map.find(service_key);
          if (iter != m_connected_services_map.end())
          {
            // call disconnect event
            std::lock_guard<std::mutex> lock_cb(m_event_callback_map_sync);
            auto e_iter = m_event_callback_map.find(client_event_disconnected);
            if (e_iter != m_event_callback_map.end())
            {
              SClientEventCallbackData sdata;
              sdata.type = client_event_disconnected;
              sdata.time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
              sdata.attr = iter->second;
              (e_iter->second)(m_service_name.c_str(), &sdata);
            }
            // remove service
            m_connected_services_map.erase(iter);
          }
        }
      }
    }
  }

  void CServiceClientImpl::CheckForNewServices()
  {
    if (!g_clientgate()) return;

    // check for new services
    std::vector<SServiceAttr> service_vec = g_clientgate()->GetServiceAttr(m_service_name);
    for (auto& iter : service_vec)
    {
      std::lock_guard<std::mutex> lock(m_client_map_sync);
      auto client = m_client_map.find(iter.key);
      if (client == m_client_map.end())
      {
        // create new client for that service
        std::shared_ptr<CTcpClient> new_client = std::make_shared<CTcpClient>(iter.hname, iter.tcp_port);
        m_client_map[iter.key] = new_client;
      }
    }
  }

  bool CServiceClientImpl::SendRequests(const std::string& host_name_, const std::string& method_name_, const std::string& request_, int timeout_)
  {
    if (!g_clientgate()) return false;

    bool ret_state(false);

    std::lock_guard<std::mutex> lock(m_client_map_sync);
    for (auto& client : m_client_map)
    {
      if (client.second->IsConnected())
      {
        if (host_name_.empty() || (host_name_ == client.second->GetHostName()))
        {
          // execute request
          SServiceResponse service_response;
          bool ret = SendRequest(client.second, method_name_, request_, timeout_, service_response);
          if (ret == false)
          {
            std::cerr << "CServiceClientImpl::SendRequests failed." << std::endl;
            return false;
          }
          // call response callback
          if (service_response.call_state != call_state_none)
          {
            std::lock_guard<std::mutex> lock_cb(m_response_callback_sync);
            if (m_response_callback) m_response_callback(service_response);
          }
          else
          {
            // call_state_none means service no more available
            // we destroy the client here
            client.second->Destroy();
          }
          // collect return state
          ret_state = true;
        }
      }
    }
    return ret_state;
  }

  bool CServiceClientImpl::SendRequest(std::shared_ptr<CTcpClient> client_, const std::string& method_name_, const std::string& request_, int timeout_, struct SServiceResponse& service_response_)
  {
    // create request protocol buffer
    eCAL::pb::Request request_pb;
    request_pb.mutable_header()->set_mname(method_name_);
    request_pb.set_request(request_);
    std::string request_s = request_pb.SerializeAsString();

    // catch events
    client_->AddEventCallback([&](eCAL_Client_Event event, const std::string& /*message*/)
      {
        switch (event)
        {
        case client_event_timeout:
        {
          std::lock_guard<std::mutex> lock_eb(m_event_callback_map_sync);
          auto e_iter = m_event_callback_map.find(client_event_timeout);
          if (e_iter != m_event_callback_map.end())
          {
            SClientEventCallbackData sdata;
            sdata.type = client_event_timeout;
            sdata.time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
            (e_iter->second)(m_service_name.c_str(), &sdata);
          }
        }
          break;
        default:
          break;
        }
      });

    // execute request
    std::string response_s;
    size_t sent = client_->ExecuteRequest(request_s, timeout_, response_s);
    if (sent == 0) return false;

    // parse response protocol buffer
    eCAL::pb::Response response_pb;
    if (!response_pb.ParseFromString(response_s))
    {
      std::cerr << "CServiceClientImpl::SendRequest Could not parse server response !" << std::endl;
      return false;
    }

    auto& response_pb_header = response_pb.header();
    service_response_.host_name    = response_pb_header.hname();
    service_response_.service_name = response_pb_header.sname();
    service_response_.service_id   = response_pb_header.sid();
    service_response_.method_name  = response_pb_header.mname();
    service_response_.error_msg    = response_pb_header.error();
    service_response_.ret_state    = static_cast<int>(response_pb.ret_state());
    switch (response_pb_header.state())
    {
    case eCAL::pb::ServiceHeader_eCallState_executed:
      service_response_.call_state = call_state_executed;
      break;
    case eCAL::pb::ServiceHeader_eCallState_failed:
      service_response_.call_state = call_state_failed;
      break;
    default:
      break;
    }
    service_response_.response = response_pb.response();

    return (service_response_.call_state == call_state_executed);
  }

  void CServiceClientImpl::SendRequestsAsync(const std::string& host_name_, const std::string& method_name_, const std::string& request_, int timeout_)
  {
    if (!g_clientgate())
    {
      ErrorCallback(method_name_, "Clientgate error.");
      return;
    }

    std::lock_guard<std::mutex> lock(m_client_map_sync);

    if (m_client_map.empty())
    {
      ErrorCallback(method_name_, "Service not available.");
      return;
    }

    for (auto& client : m_client_map)
    {
      if (client.second->IsConnected())
      {
        if (host_name_.empty() || (host_name_ == client.second->GetHostName()))
        {
          // execute request
          SendRequestAsync(client.second, method_name_, request_, timeout_);
        }
      }
    }
  }

  void CServiceClientImpl::SendRequestAsync(std::shared_ptr<CTcpClient> client_, const std::string& method_name_, const std::string& request_, int timeout_)
  {
    // create request protocol buffer
    eCAL::pb::Request request_pb;
    request_pb.mutable_header()->set_mname(method_name_);
    request_pb.set_request(request_);
    std::string request_s = request_pb.SerializeAsString();

    client_->ExecuteRequestAsync(request_s, timeout_, [this, client_, method_name_](const std::string& response, bool success)
      {
        std::lock_guard<std::mutex> lock(m_response_callback_sync);
        if (m_response_callback)
        {
          SServiceResponse service_response;
          if (!success)
          {
            auto error_msg = "CServiceClientImpl::SendRequestAsync failed !";
            service_response.call_state  = call_state_failed;
            service_response.error_msg   = error_msg;
            service_response.ret_state   = 0;
            service_response.method_name = method_name_;
            service_response.response.clear();
            m_response_callback(service_response);
            return;
          }

          eCAL::pb::Response response_pb;
          if (!response_pb.ParseFromString(response))
          {
            auto error_msg = "CServiceClientImpl::SendRequestAsync could not parse server response !";
            std::cerr << error_msg << "\n";
            service_response.call_state  = call_state_failed;
            service_response.error_msg   = error_msg;
            service_response.ret_state   = 0;
            service_response.method_name = method_name_;
            service_response.response.clear();
            m_response_callback(service_response);
            return;
          }

          auto& response_pb_header = response_pb.header();
          service_response.host_name    = response_pb_header.hname();
          service_response.service_name = response_pb_header.sname();
          service_response.service_id   = response_pb_header.sid();
          service_response.method_name  = response_pb_header.mname();
          service_response.error_msg    = response_pb_header.error();
          service_response.ret_state    = static_cast<int>(response_pb.ret_state());
          switch (response_pb_header.state())
          {
          case eCAL::pb::ServiceHeader_eCallState_executed:
            service_response.call_state = call_state_executed;
            break;
          case eCAL::pb::ServiceHeader_eCallState_failed:
            service_response.call_state = call_state_failed;
            break;
          default:
            break;
          }
          service_response.response = response_pb.response();

          m_response_callback(service_response);
        }
      });
  }

  void CServiceClientImpl::ErrorCallback(const std::string& method_name_, const std::string& error_message_)
  {
    std::lock_guard<std::mutex> lock(m_response_callback_sync);
    if (m_response_callback)
    {
      SServiceResponse service_response;
      service_response.call_state  = call_state_failed;
      service_response.error_msg   = error_message_;
      service_response.ret_state   = 0;
      service_response.method_name = method_name_;
      service_response.response.clear();
      m_response_callback(service_response);
    }
  }
}
