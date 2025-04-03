/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
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
#include "service/ecal_service_client_impl.h"

#include <atomic>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <vector>

namespace eCAL
{
  //////////////////////////////////////////////////////////////////
  // CClientGate
  //////////////////////////////////////////////////////////////////
  std::atomic<bool> CClientGate::m_created;
  CClientGate::CClientGate() = default;

  CClientGate::~CClientGate()
  {
    Stop();
  }

  void CClientGate::Start()
  {
    if (m_created) return;
    m_created = true;
  }

  void CClientGate::Stop()
  {
    if (!m_created) return;

    // destroy all remaining clients
    const std::unique_lock<std::shared_timed_mutex> lock(m_service_client_map_mutex);
    m_service_client_map.clear();

    m_created = false;
  }

  bool CClientGate::Register(const std::string& service_name_, const std::shared_ptr<CServiceClientImpl>& client_)
  {
    if (!m_created) return(false);

    // register internal client
    const std::unique_lock<std::shared_timed_mutex> lock(m_service_client_map_mutex);
    m_service_client_map.emplace(std::pair<std::string, std::shared_ptr<CServiceClientImpl>>(service_name_, client_));

    return(true);
  }

  bool CClientGate::Unregister(const std::string& service_name_, const std::shared_ptr<CServiceClientImpl>& client_)
  {
    if (!m_created) return(false);
    bool ret_state = false;

    const std::unique_lock<std::shared_timed_mutex> lock(m_service_client_map_mutex);
    auto res = m_service_client_map.equal_range(service_name_);
    for (auto iter = res.first; iter != res.second; ++iter)
    {
      if (iter->second == client_)
      {
        m_service_client_map.erase(iter);
        ret_state = true;
        break;
      }
    }

    return(ret_state);
  }

  void CClientGate::ApplyServiceRegistration(const Registration::Sample& ecal_sample_)
  {
    v5::SServiceAttr service;
    const auto& ecal_sample_service = ecal_sample_.service;
    const auto& ecal_sample_identifier = ecal_sample_.identifier;
    service.hname = ecal_sample_identifier.host_name;
    service.pname = ecal_sample_service.process_name;
    service.uname = ecal_sample_service.unit_name;
    service.sname = ecal_sample_service.service_name;
    service.sid   = ecal_sample_identifier.entity_id;
    service.pid   = static_cast<int>(ecal_sample_identifier.process_id);

    // internal protocol specifics
    service.version     = static_cast<unsigned int>(ecal_sample_service.version);
    service.tcp_port_v0 = static_cast<unsigned short>(ecal_sample_service.tcp_port_v0);
    service.tcp_port_v1 = static_cast<unsigned short>(ecal_sample_service.tcp_port_v1);

    // inform matching clients
    {
      const std::shared_lock<std::shared_timed_mutex> lock(m_service_client_map_mutex);
      auto res = m_service_client_map.equal_range(service.sname);
      for (ServiceNameClientIDImplMapT::const_iterator iter = res.first; iter != res.second; ++iter)
      {
        SEntityId service_entity;
        service_entity.entity_id  = service.sid;
        service_entity.process_id = service.pid;
        service_entity.host_name  = service.hname;
        iter->second->RegisterService(service_entity, service);
      }
    }
  }

  void CClientGate::GetRegistrations(Registration::SampleList& reg_sample_list_)
  {
    if (!m_created) return;

    // read client registrations
    {
      const std::shared_lock<std::shared_timed_mutex> lock(m_service_client_map_mutex);
      for (const auto& iter : m_service_client_map)
      {
        reg_sample_list_.push_back(iter.second->GetRegistration());
      }
    }
  }
}
