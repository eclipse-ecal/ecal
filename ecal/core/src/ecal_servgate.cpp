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
 * @brief  eCAL server gateway class
**/

#include <ecal/ecal.h>

#include "ecal_def.h"
#include "ecal_servgate.h"
#include "ecal_config_hlp.h"

#include <iterator>
#include <atomic>

namespace eCAL
{
  //////////////////////////////////////////////////////////////////
  // CServGate
  //////////////////////////////////////////////////////////////////
  std::atomic<bool> CServGate::m_created;
  CServGate::CServGate()
  {
  };

  CServGate::~CServGate()
  {
    Destroy();
  }

  void CServGate::Create()
  {
    if(m_created) return;
    m_created = true;
  }

  void CServGate::Destroy()
  {
    if(!m_created) return;
    m_created = false;
  }

  bool CServGate::Register(const std::string& service_name_, CServiceServerImpl* service_)
  {
    if(!m_created) return(false);

    // register internal service implementation
    std::lock_guard<std::mutex> lock(m_internal_service_sync);
    m_internal_service_map.emplace(std::pair<std::string, CServiceServerImpl*>(service_name_, service_));

    return(true);
  }

  bool CServGate::Unregister(const std::string& service_name_, CServiceServerImpl* service_)
  {
    if(!m_created) return(false);
    bool ret_state = false;

    std::lock_guard<std::mutex> lock(m_internal_service_sync);
    auto res = m_internal_service_map.equal_range(service_name_);
    for(ServiceNameServiceImplMapT::iterator iter = res.first; iter != res.second; ++iter)
    {
      if(iter->second == service_)
      {
        m_internal_service_map.erase(iter);
        break;
      }
    }

    return(ret_state);
  }

  std::vector<CServGate::SService> CServGate::GetServiceInfo(const std::string& service_name_)
  {
    auto now = RegClockT::now();

    std::vector<CServGate::SService> ret_vec;
    std::lock_guard<std::mutex> lock(m_service_register_sync);

    // Cleanup old services
    auto service_reg_it = m_service_register_map.begin();
    while (service_reg_it != m_service_register_map.end())
    {
      if (now - service_reg_it->second.second > std::chrono::milliseconds(eCALPAR(CMN, REGISTRATION_TO)))
      {
#ifndef NDEBUG
        // log it
        Logging::Log(log_level_debug1, "CServGate::GetServiceInfo - Removing service due to timeout");
#endif // !NDEBUG
        m_service_register_map.erase(service_reg_it++);
      }
      else
      {
        service_reg_it++;
      }
    }

    // Look for requested services
    for (auto& service : m_service_register_map)
    {
      if (service.second.first.sname == service_name_)
      {
        ret_vec.push_back(service.second.first);
      }
    }
    return(ret_vec);
  }

  void CServGate::ApplyServiceRegistration(const eCAL::pb::Sample & ecal_sample_)
  {
    auto registration_time = RegClockT::now();

    SService service;
    auto ecal_sample_service = ecal_sample_.service();
    service.hname    = ecal_sample_service.hname();
    service.pname    = ecal_sample_service.pname();
    service.uname    = ecal_sample_service.uname();
    service.pid      = static_cast<int>(ecal_sample_service.pid());
    service.sname    = ecal_sample_service.sname();
    service.tcp_port = static_cast<unsigned short>(ecal_sample_service.tcp_port());

    // register service
    std::lock_guard<std::mutex> lock(m_service_register_sync);
    std::string key = service.sname + ":" + std::to_string(service.tcp_port) + "@" + std::to_string(service.pid) + "@" + service.hname;

    auto existing_service_it = m_service_register_map.find(key);
    if (existing_service_it == m_service_register_map.end())
    {
      // Create new entry
#ifndef NDEBUG
      // log it
      Logging::Log(log_level_debug1, "CServGate::GetServiceInfo - Received new service registration");
#endif // !NDEBUG

      m_service_register_map.emplace(key, std::make_pair(service, registration_time));
    }
    else
    {
      // Update existing entry
      existing_service_it->second.second = registration_time;
    }
  }

  void CServGate::RefreshRegistrations()
  {
    if (!m_created) return;

    // refresh service registrations
    std::lock_guard<std::mutex> lock(m_internal_service_sync);
    for (auto iter : m_internal_service_map)
    {
      iter.second->RefreshRegistration();
    }
  }
};
