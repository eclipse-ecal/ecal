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

#include "ecal_service_client_impl.h"

#include <ecal/ecal.h>
#include "ecal_servgate.h"
#include "ecal_global_accessors.h"

namespace eCAL
{
  /**
  * @brief Service client implementation class.
  **/
  CServiceClientImpl::CServiceClientImpl() :
    m_reply{},
    m_callback(nullptr),
    m_created(false)
  {
  }

  CServiceClientImpl::CServiceClientImpl(const std::string& service_name_) :
    m_reply{},
    m_callback(nullptr),
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

    m_service_name = service_name_;
    m_callback = nullptr;

    m_created = true;

    return(true);
  }

  bool CServiceClientImpl::Destroy()
  {
    if (!m_created) return(false);

    m_client_map.clear();
    m_service_hname.clear();
    m_service_name.clear();
    m_callback = nullptr;

    m_created = false;

    return(true);
  }

  bool CServiceClientImpl::SetHostName(const std::string& host_name_)
  {
    if (host_name_ == "*") m_service_hname.clear();
    else                   m_service_hname = host_name_;
    return(true);
  }

  bool CServiceClientImpl::AddResponseCallback(const ResponseCallbackT& callback_)
  {
    m_callback = callback_;
    return true;
  }

  bool CServiceClientImpl::RemResponseCallback()
  {
    m_callback = nullptr;
    return true;
  }

  bool CServiceClientImpl::Call(const std::string& host_name_, const std::string& method_name_, const std::string& request_, struct SServiceInfo& service_info_, std::string& response_)
  {
    if (!g_servgate()) return false;
    if (!m_created)    return false;

    if (m_service_name.empty()
      || method_name_.empty()
      )
      return false;

    // check for new server
    RefreshClientMap();

    std::vector<CServGate::SService> service_vec = g_servgate()->GetServiceInfo(m_service_name);
    for (auto iter : service_vec)
    {
      if (host_name_.empty() || (host_name_ == iter.hname))
      {
        std::string key = iter.sname + ":" + std::to_string(iter.tcp_port) + "@" + std::to_string(iter.pid) + "@" + iter.hname;
        auto client = m_client_map.find(key);
        if (client != m_client_map.end())
        {
          return SendRequest(client->second, method_name_, request_, service_info_, response_);
        }
      }
    }
    return false;
  }

  bool CServiceClientImpl::Call(const std::string& method_name_, const std::string& request_)
  {
    if (!g_servgate()) return false;
    if (!m_created)    return false;

    if (m_service_name.empty()
      || method_name_.empty()
      )
      return false;

    std::lock_guard<std::mutex> req_lock(m_req_mtx);

    // check for new server
    RefreshClientMap();

    // send request to every single service
    return SendRequests(method_name_, request_);
  }

  void CServiceClientImpl::CallAsync(const std::string& host_name_, const std::string& method_name_, const std::string& request_)
  {
    if (!g_servgate())
    {
      ErrorCallback(method_name_, "Servgate error.");
      return;
    }
    if (!m_created)
    {
      ErrorCallback(method_name_, "Client hasn't been created yet.");
      return;
    }

    if (m_service_name.empty()
      || method_name_.empty())
    {
      ErrorCallback(method_name_, "Invalid service or method name.");
      return;
    }

    // check for new server
    RefreshClientMap();

    std::vector<CServGate::SService> service_vec = g_servgate()->GetServiceInfo(m_service_name);
    for (auto iter : service_vec)
    {
      if (host_name_.empty() || (host_name_ == iter.hname))
      {
        std::string key = iter.sname + ":" + std::to_string(iter.tcp_port) + "@" + std::to_string(iter.pid) + "@" + iter.hname;
        auto client = m_client_map.find(key);
        if (client != m_client_map.end())
        {
          SendRequestAsync(client->second, method_name_, request_);
        }
      }
    }
  }

  void CServiceClientImpl::CallAsync(const std::string& method_name_, const std::string& request_)
  {
    if (!g_servgate())
    {
      ErrorCallback(method_name_, "Servgate error.");
      return;
    }
    if (!m_created)
    {
      ErrorCallback(method_name_, "Client hasn't been created yet.");
      return;
    }

    if (m_service_name.empty()
      || method_name_.empty()
      )
    {
      ErrorCallback(method_name_, "Invalid service or method name.");
      return;
    }
    std::lock_guard<std::mutex> req_lock(m_req_mtx);

    // check for new server
    RefreshClientMap();

    // send request to every single service
    SendRequestsAsync(method_name_, request_);

  }

  void CServiceClientImpl::RefreshClientMap()
  {
    if (!g_servgate()) return;

    // check for new services
    std::vector<CServGate::SService> service_vec = g_servgate()->GetServiceInfo(m_service_name);
    for (auto iter : service_vec)
    {
      if (m_service_hname.empty() || (m_service_hname == iter.hname))
      {
        std::string key = iter.sname + ":" + std::to_string(iter.tcp_port) + "@" + std::to_string(iter.pid) + "@" + iter.hname;
        auto client = m_client_map.find(key);
        if (client == m_client_map.end())
        {
          std::shared_ptr<CTcpClient> new_client = std::make_shared<CTcpClient>(iter.hname, iter.tcp_port);
          m_client_map[key] = new_client;
        }
      }
    }
  }

  bool CServiceClientImpl::SendRequests(const std::string& method_name_, const std::string& request_)
  {
    if (!g_servgate()) return false;

    bool ret_state(false);
    for (auto client : m_client_map)
    {
      if (client.second->IsConnected())
      {
        if (m_service_hname.empty() || (m_service_hname == client.second->GetHostName()))
        {
          // execute request
          SServiceInfo service_info;
          std::string  response;
          bool ret = SendRequest(client.second, method_name_, request_, service_info, response);
          if (ret == false)
          {
            std::cerr << "CServiceClientImpl::SendRequests failed." << std::endl;
            return false;
          }
          // call response callback
          if (service_info.call_state != call_state_none)
          {
            if (m_callback) m_callback(service_info, response);
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

  bool CServiceClientImpl::SendRequest(std::shared_ptr<CTcpClient> client_, const std::string& method_name_, const std::string& request_, struct SServiceInfo& service_info_, std::string& response_)
  {
    // create request protocol buffer
    eCAL::pb::Request request_pb;
    request_pb.mutable_header()->set_mname(method_name_);
    request_pb.set_request(request_);
    std::string request_s = request_pb.SerializeAsString();

    // execute request
    std::string response_s;
    size_t sent = client_->ExecuteRequest(request_s, response_s);
    if (sent == 0) return false;

    // parse response protocol buffer
    eCAL::pb::Response response_pb;
    if (!response_pb.ParseFromString(response_s))
    {
      std::cerr << "CServiceClientImpl::SendRequest Could not parse server response !" << std::endl;
      return false;
    }

    auto response_pb_header = response_pb.header();
    service_info_.host_name = response_pb_header.hname();
    service_info_.service_name = response_pb_header.sname();
    service_info_.method_name = response_pb_header.mname();
    service_info_.error_msg = response_pb_header.error();
    service_info_.ret_state = static_cast<int>(response_pb.ret_state());
    switch (response_pb_header.state())
    {
    case eCAL::pb::ServiceHeader_eCallState_executed:
      service_info_.call_state = call_state_executed;
      break;
    case eCAL::pb::ServiceHeader_eCallState_failed:
      service_info_.call_state = call_state_failed;
      break;
    default:
      break;
    }
    response_ = response_pb.response();

    return (service_info_.call_state == call_state_executed);
  }

  void CServiceClientImpl::SendRequestsAsync(const std::string& method_name_, const std::string& request_)
  {
    if (!g_servgate())
    {
      ErrorCallback(method_name_, "Servgate error.");
      return;
    }

    if(m_client_map.empty())
    {
      ErrorCallback(method_name_, "Service not available.");
      return;
    }
    for (auto client : m_client_map)
    {
      if (client.second->IsConnected())
      {
        if (m_service_hname.empty() || (m_service_hname == client.second->GetHostName()))
        {
          // execute request
          SendRequestAsync(client.second, method_name_, request_);
        }
      }
    }
  }

  void CServiceClientImpl::SendRequestAsync(std::shared_ptr<CTcpClient> client_, const std::string& method_name_, const std::string& request_)
  {
    // create request protocol buffer
    eCAL::pb::Request request_pb;
    request_pb.mutable_header()->set_mname(method_name_);
    request_pb.set_request(request_);
    std::string request_s = request_pb.SerializeAsString();

    client_->ExecuteRequestAsync(request_s, [this, client_, method_name_](const std::string &response, bool success)
    {
      if (m_callback)
      {
        SServiceInfo service_info;
        if(!success)
        {
          auto error_msg = "Async request failed !";
          service_info.call_state = call_state_failed;
          service_info.error_msg = error_msg;
          service_info.ret_state = 0;
          service_info.method_name = method_name_;
          m_callback(service_info, "");
          return;
        }

        eCAL::pb::Response response_pb;
        if (!response_pb.ParseFromString(response))
        {
          auto error_msg = "CServiceClientImpl::SendRequestAsync could not parse server response !";
          std::cerr << error_msg << "\n";
          service_info.call_state = call_state_failed;
          service_info.error_msg = error_msg;
          service_info.ret_state = 0;
          service_info.method_name = method_name_;
          m_callback(service_info, "");
          return;
        }

        auto response_pb_header = response_pb.header();
        service_info.host_name = response_pb_header.hname();
        service_info.service_name = response_pb_header.sname();
        service_info.method_name = response_pb_header.mname();
        service_info.error_msg = response_pb_header.error();
        service_info.ret_state = static_cast<int>(response_pb.ret_state());
        switch (response_pb_header.state())
        {
        case eCAL::pb::ServiceHeader_eCallState_executed:
          service_info.call_state = call_state_executed;
          break;
        case eCAL::pb::ServiceHeader_eCallState_failed:
          service_info.call_state = call_state_failed;
          break;
        default:
          break;
        }

        m_callback(service_info, response_pb.response());
      }
    });
  }

  void CServiceClientImpl::ErrorCallback(const std::string &method_name_, const std::string &error_message_)
  {
    if(m_callback)
    {
      SServiceInfo service_info;
      service_info.call_state = call_state_failed;
      service_info.error_msg = error_message_;
      service_info.ret_state = 0;
      service_info.method_name = method_name_;
      m_callback(service_info, "");
    }
  }
}
