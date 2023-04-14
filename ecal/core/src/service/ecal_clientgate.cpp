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
 * @brief  eCAL client gateway class
**/

#include "ecal_clientgate.h"
#include "ecal_descgate.h"
#include "service/ecal_service_client_impl.h"

namespace eCAL
{
  //////////////////////////////////////////////////////////////////
  // CClientGate
  //////////////////////////////////////////////////////////////////
  std::atomic<bool> CClientGate::m_created;
  CClientGate::CClientGate() = default;

  CClientGate::~CClientGate()
  {
    Destroy();
  }

  void CClientGate::Create()
  {
    if (m_created) return;
    m_created = true;
  }

  void CClientGate::Destroy()
  {
    if (!m_created) return;
    m_created = false;
  }

  bool CClientGate::Register(CServiceClientImpl* client_)
  {
    if (!m_created) return(false);

    // register internal client
    std::unique_lock<std::shared_timed_mutex> const lock(m_client_set_sync);
    m_client_set.insert(client_);

    return(true);
  }

  bool CClientGate::Unregister(CServiceClientImpl* client_)
  {
    if (!m_created) return(false);
    bool ret_state(false);

    // unregister internal service
    std::unique_lock<std::shared_timed_mutex> const lock(m_client_set_sync);
    for (auto iter = m_client_set.begin(); iter != m_client_set.end();)
    {
      if (*iter == client_)
      {
        iter = m_client_set.erase(iter);
        ret_state = true;
      }
      else
      {
        iter++;
      }
    }

    return(ret_state);
  }

  void CClientGate::ApplyServiceRegistration(const eCAL::pb::Sample& ecal_sample_)
  {

    SServiceAttr service;
    const auto& ecal_sample_service = ecal_sample_.service();
    service.hname    = ecal_sample_service.hname();
    service.pname    = ecal_sample_service.pname();
    service.uname    = ecal_sample_service.uname();
    service.sname    = ecal_sample_service.sname();
    service.sid      = ecal_sample_service.sid();
    service.pid      = static_cast<int>(ecal_sample_service.pid());
    service.tcp_port = static_cast<unsigned short>(ecal_sample_service.tcp_port());

    // service protocol version
    unsigned int service_version = ecal_sample_service.version();

    // store description
    for (const auto& method : ecal_sample_service.methods())
    {
      ApplyServiceToDescGate(ecal_sample_service.sname(), method.mname(), method.req_type(), method.req_desc(), method.resp_type(), method.resp_desc());
    }

    // create service key
    service.key = service.sname + ":" + service.sid + "@" + std::to_string(service.pid) + "@" + service.hname;

    // add or remove (timeouted) services
    {
      std::unique_lock<std::shared_timed_mutex> const lock(m_service_register_map_sync);

      // add / update service
      m_service_register_map[service.key] = service;

      // remove timeouted services
      m_service_register_map.remove_deprecated();
    }

    // inform matching clients
    {
      std::shared_lock<std::shared_timed_mutex> const lock(m_client_set_sync);
      for (const auto& iter : m_client_set)
      {
        if (iter->GetServiceName() == service.sname)
        {
          iter->RegisterService(service.key, service_version, service);
        }
      }
    }
  }

  std::vector<SServiceAttr> CClientGate::GetServiceAttr(const std::string& service_name_)
  {
    std::vector<SServiceAttr> ret_vec;
    std::shared_lock<std::shared_timed_mutex> const lock(m_service_register_map_sync);

    // look for requested services
    for (auto service : m_service_register_map)
    {
      if (service.second.sname == service_name_)
      {
        ret_vec.push_back(service.second);
      }
    }
    return(ret_vec);
  }

  void CClientGate::RefreshRegistrations()
  {
    if (!m_created) return;

    // refresh service registrations
    std::shared_lock<std::shared_timed_mutex> const lock(m_client_set_sync);
    for (auto *iter : m_client_set)
    {
      iter->RefreshRegistration();
    }
  }

  bool CClientGate::ApplyServiceToDescGate(const std::string& service_name_
    , const std::string& method_name_
    , const std::string& req_type_name_
    , const std::string& req_type_desc_
    , const std::string& resp_type_name_
    , const std::string& resp_type_desc_)
  {
    if (g_descgate() != nullptr)
    {
      // calculate the quality of the current info
      ::eCAL::CDescGate::QualityFlags quality = ::eCAL::CDescGate::QualityFlags::NO_QUALITY;
      if (!(req_type_name_.empty() && resp_type_name_.empty()))
        quality |= ::eCAL::CDescGate::QualityFlags::TYPE_AVAILABLE;
      if (!(req_type_desc_.empty() && resp_type_desc_.empty()))
        quality |= ::eCAL::CDescGate::QualityFlags::DESCRIPTION_AVAILABLE;

      return g_descgate()->ApplyServiceDescription(service_name_, method_name_, req_type_name_, req_type_desc_, resp_type_name_, resp_type_desc_, quality);
    }
    return false;
  }
};
