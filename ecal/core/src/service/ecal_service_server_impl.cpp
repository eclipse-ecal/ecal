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

#include "ecal_descgate.h"
#include "ecal_registration_provider.h"
#include "ecal_servicegate.h"
#include "ecal_global_accessors.h"
#include "ecal_service_server_impl.h"

#include <chrono>
#include <sstream>
#include <utility>

namespace eCAL
{
  /**
   * @brief Service server implementation class.
  **/
  CServiceServerImpl::CServiceServerImpl() :
    m_connected(false), m_created(false)
  {
  }

  CServiceServerImpl::CServiceServerImpl(const std::string& service_name_) :
    m_connected(false), m_created(false)
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

    if (g_servicegate() != nullptr) g_servicegate()->Register(this);

    m_created = true;

    return(true);
  }

  bool CServiceServerImpl::Destroy()
  {
    if (!m_created) return(false);

    m_tcp_server.Stop();
    m_tcp_server.Destroy();

    if (g_servicegate() != nullptr)           g_servicegate()->Unregister(this);
    if (g_registration_provider() != nullptr) g_registration_provider()->UnregisterServer(m_service_name, m_service_id);

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

    m_service_name.clear();
    m_service_id.clear();

    m_connected = false;
    m_created   = false;

    return(true);
  }

  bool CServiceServerImpl::AddDescription(const std::string& method_, const std::string& req_type_, const std::string& req_desc_, const std::string& resp_type_, const std::string& resp_desc_)
  {
    {
      std::lock_guard<std::mutex> const lock(m_method_map_sync);
      auto iter = m_method_map.find(method_);
      if (iter != m_method_map.end())
      {
        iter->second.method_pb.set_mname(method_);
        iter->second.method_pb.set_req_type(req_type_);
        iter->second.method_pb.set_req_desc(req_desc_);
        iter->second.method_pb.set_resp_type(resp_type_);
        iter->second.method_pb.set_resp_desc(resp_desc_);
      }
      else
      {
        SMethod method;
        method.method_pb.set_mname(method_);
        method.method_pb.set_req_type(req_type_);
        method.method_pb.set_req_desc(req_desc_);
        method.method_pb.set_resp_type(resp_type_);
        method.method_pb.set_resp_desc(resp_desc_);
        m_method_map[method_] = method;
      }
    }

    // update descgate infos
    return ApplyServiceToDescGate(method_, req_type_, req_desc_, resp_type_, resp_desc_);
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
        iter->second.method_pb.set_mname(method_);
        iter->second.method_pb.set_req_type(req_type_);
        iter->second.method_pb.set_resp_type(resp_type_);
        // set callback
        iter->second.callback = callback_;

        // read descriptors back from existing service method
        req_desc = iter->second.method_pb.req_desc();
        resp_desc = iter->second.method_pb.resp_desc();
      }
      else
      {
        SMethod method;
        method.method_pb.set_mname(method_);
        method.method_pb.set_req_type(req_type_);
        method.method_pb.set_resp_type(resp_type_);
        method.callback = callback_;
        m_method_map[method_] = method;
      }
    }

    // update descgate infos
    ApplyServiceToDescGate(method_, req_type_, req_desc, resp_type_, resp_desc);

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
    return m_tcp_server.IsConnected();
  }

  // called by the eCAL::CServiceGate to register a client
  void CServiceServerImpl::RegisterClient(const std::string& /*key_*/, unsigned int /*version_*/, const SClientAttr& /*client_*/)
  {
    // TODO: CHECK COMPATIBILITY HERE
  }

  // called by eCAL:CServiceGate every second to update registration layer
  void CServiceServerImpl::RefreshRegistration()
  {
    if (!m_created)             return;
    if (m_service_name.empty()) return;

    // might be zero in contruction phase
    unsigned short const server_tcp_port(m_tcp_server.GetTcpPort());
    if (server_tcp_port == 0) return;

    // create service registration sample
    eCAL::pb::Sample sample;
    sample.set_cmd_type(eCAL::pb::bct_reg_service);
    auto *service_mutable_service = sample.mutable_service();
    service_mutable_service->set_version(m_version);
    service_mutable_service->set_hname(Process::GetHostName());
    service_mutable_service->set_pname(Process::GetProcessName());
    service_mutable_service->set_uname(Process::GetUnitName());
    service_mutable_service->set_pid(Process::GetProcessID());
    service_mutable_service->set_sname(m_service_name);
    service_mutable_service->set_sid(m_service_id);
    service_mutable_service->set_tcp_port(server_tcp_port);

    // add methods
    {
      std::lock_guard<std::mutex> const lock(m_method_map_sync);
      for (const auto& iter : m_method_map)
      {
        auto *method = service_mutable_service->add_methods();
        method->set_mname(iter.first);
        method->set_req_type(iter.second.method_pb.req_type());
        method->set_req_desc(iter.second.method_pb.req_desc());
        method->set_resp_type(iter.second.method_pb.resp_type());
        method->set_resp_desc(iter.second.method_pb.resp_desc());
        method->set_call_count(iter.second.method_pb.call_count());
      }
    }

    // register entity
    if (g_registration_provider() != nullptr) g_registration_provider()->RegisterServer(m_service_name, m_service_id, sample, false);
  }

  int CServiceServerImpl::RequestCallback(const std::string& request_, std::string& response_)
  {
    // prepare response
    eCAL::pb::Response response_pb;
    auto* response_pb_mutable_header = response_pb.mutable_header();
    response_pb_mutable_header->set_hname(eCAL::Process::GetHostName());
    response_pb_mutable_header->set_sname(m_service_name);
    response_pb_mutable_header->set_sid(m_service_id);

    // try to parse request
    eCAL::pb::Request  request_pb;
    if (!request_pb.ParseFromString(request_))
    {
      Logging::Log(log_level_error, m_service_name + "::CServiceServerImpl::RequestCallback failed to parse request message");

      response_pb_mutable_header->set_state(eCAL::pb::ServiceHeader_eCallState_failed);
      std::string const emsg = "Service '" + m_service_name + "' request message could not be parsed.";
      response_pb_mutable_header->set_error(emsg);

      // serialize response and return "request message could not be parsed"
      response_ = response_pb.SerializeAsString();

      // Return Failed (error_code = -1), as parsing the request failed. The
      // return value is not propagated to the remote caller.
      return -1;
    }

    // get method
    SMethod method;
    const auto& request_pb_header = request_pb.header();
    response_pb_mutable_header->set_mname(request_pb_header.mname());
    {
      std::lock_guard<std::mutex> const lock(m_method_map_sync);

      auto requested_method_iterator = m_method_map.find(request_pb_header.mname());
      if (requested_method_iterator == m_method_map.end())
      {
        // set method call state 'failed'
        response_pb_mutable_header->set_state(eCAL::pb::ServiceHeader_eCallState_failed);
        // set error message
        std::string const emsg = "Service '" + m_service_name + "' has no method named '" + request_pb_header.mname() + "'";
        response_pb_mutable_header->set_error(emsg);

        // serialize response and return "method not found"
        response_ = response_pb.SerializeAsString();

        // Return Success (error_code = 0), as parsing the request worked. The
        // return value is not propagated to the remote caller.
        return 0;
      }
      else
      {
        // increase call count
        auto call_count = requested_method_iterator->second.method_pb.call_count();
        requested_method_iterator->second.method_pb.set_call_count(++call_count);

        // store (copy) the method object, so we can release the mutex before calling the function
        method = requested_method_iterator->second;
      }
    }

    // execute method (outside lock guard)
    const std::string& request_s = request_pb.request();
    std::string response_s;
    int const service_return_state = method.callback(method.method_pb.mname(), method.method_pb.req_type(), method.method_pb.resp_type(), request_s, response_s);

    // set method call state 'executed'
    response_pb_mutable_header->set_state(eCAL::pb::ServiceHeader_eCallState_executed);
    // set method response and return state
    response_pb.set_response(response_s);
    response_pb.set_ret_state(service_return_state);

    // serialize response and return
    response_ = response_pb.SerializeAsString();

    // return success (error code 0)
    return 0;
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

  bool CServiceServerImpl::ApplyServiceToDescGate(const std::string& method_name_
    , const std::string& req_type_name_
    , const std::string& req_type_desc_
    , const std::string& resp_type_name_
    , const std::string& resp_type_desc_)
  {
    if (g_descgate() != nullptr)
    {
      // Calculate the quality of the current info
      ::eCAL::CDescGate::QualityFlags quality = ::eCAL::CDescGate::QualityFlags::NO_QUALITY;
      if (!(req_type_name_.empty() && resp_type_name_.empty()))
        quality |= ::eCAL::CDescGate::QualityFlags::TYPE_AVAILABLE;
      if (!(req_type_desc_.empty() && resp_type_desc_.empty()))
        quality |= ::eCAL::CDescGate::QualityFlags::DESCRIPTION_AVAILABLE;
      quality |= ::eCAL::CDescGate::QualityFlags::INFO_COMES_FROM_THIS_PROCESS;

      return g_descgate()->ApplyServiceDescription(m_service_name, method_name_, req_type_name_, req_type_desc_, resp_type_name_, resp_type_desc_, quality);
    }
    return false;
  }
};
