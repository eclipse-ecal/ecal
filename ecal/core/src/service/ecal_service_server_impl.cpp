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

#include "ecal_service_server_impl.h"
#include "ecal_register.h"
#include "ecal_servgate.h"
#include "ecal_global_accessors.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4146 4800)
#endif
#include "ecal/pb/ecal.pb.h"
#include "ecal/pb/service.pb.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif

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

    m_service_name = service_name_;

    m_tcp_server.Create();
    m_tcp_server.Start(std::bind(&CServiceServerImpl::RequestCallback, this, std::placeholders::_1, std::placeholders::_2));

    if (g_servgate()) g_servgate()->Register(service_name_, this);

    m_created = true;

    return(true);
  }

  bool CServiceServerImpl::Destroy()
  {
    if (!m_created) return(false);

    m_tcp_server.Stop();
    m_tcp_server.Destroy();

    if (g_servgate()) g_servgate()->Unregister(m_service_name, this);
    if (g_entity_register()) g_entity_register()->UnregisterService(m_service_name);

    m_created = false;

    return(true);
  }

  bool CServiceServerImpl::AddMethodCallback(const std::string& method_, const std::string& req_type_, const std::string& resp_type_, const MethodCallbackT& callback_)
  {
    SMethodCallback mcallback;
    mcallback.method.set_mname(method_);
    mcallback.method.set_req_type(req_type_);
    mcallback.method.set_resp_type(resp_type_);
    mcallback.callback = callback_;
    m_callback_map[method_] = mcallback;
    return true;
  }

  bool CServiceServerImpl::RemMethodCallback(const std::string& method_)
  {
    auto iter = m_callback_map.find(method_);
    if (iter != m_callback_map.end())
    {
      m_callback_map.erase(iter);
      return true;
    }
    return false;
  }

  void CServiceServerImpl::RefreshRegistration()
  {
    if (!m_created)             return;
    if (m_service_name.empty()) return;

    eCAL::pb::Sample service;
    service.set_cmd_type(eCAL::pb::bct_reg_service);
    auto service_mutable_service = service.mutable_service();
    service_mutable_service->set_hname(Process::GetHostName());
    service_mutable_service->set_pname(Process::GetProcessName());
    service_mutable_service->set_uname(Process::GetUnitName());
    service_mutable_service->set_pid(Process::GetProcessID());
    service_mutable_service->set_sname(m_service_name);
    service_mutable_service->set_tcp_port(m_tcp_server.GetTcpPort());
    for (auto iter : m_callback_map)
    {
      auto method = service_mutable_service->add_methods();
      method->set_mname(iter.first);
      method->set_req_type(iter.second.method.req_type());
      method->set_resp_type(iter.second.method.resp_type());
      method->set_call_count(iter.second.method.call_count());
    }

    if (g_entity_register()) g_entity_register()->RegisterService(m_service_name, service, false);
  }

  int CServiceServerImpl::RequestCallback(const std::string& request_, std::string& response_)
  {
    if (m_callback_map.empty()) return 0;

    int success(-1);
    eCAL::pb::Response response_pb;
    auto response_pb_mutable_header = response_pb.mutable_header();
    response_pb_mutable_header->set_hname(eCAL::Process::GetHostName());
    response_pb_mutable_header->set_sname(m_service_name);

    eCAL::pb::Request request_pb;
    if (request_pb.ParseFromString(request_))
    {
      // success == 0 means we could deserialize and
      // no need to try to read more bytes for the server
      // socket
      success = 0;
      auto request_pb_header = request_pb.header();
      response_pb_mutable_header->set_mname(request_pb_header.mname());

      auto iter = m_callback_map.find(request_pb_header.mname());
      if (iter != m_callback_map.end())
      {
        auto call_count = iter->second.method.call_count();
        iter->second.method.set_call_count(++call_count);

        std::string request_s = request_pb.request();
        std::string response_s;
        int service_return_state = m_callback_map[request_pb_header.mname()].callback(iter->second.method.mname(), iter->second.method.req_type(), iter->second.method.resp_type(), request_s, response_s);

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
