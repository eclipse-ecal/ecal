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
#include "ecal_servicegate.h"
#include "ecal_config_hlp.h"
#include "service/ecal_service_server_impl.h"

#include <iterator>
#include <atomic>

namespace eCAL
{
  //////////////////////////////////////////////////////////////////
  // CServerGate
  //////////////////////////////////////////////////////////////////
  std::atomic<bool> CServiceGate::m_created;
  CServiceGate::CServiceGate() = default;

  CServiceGate::~CServiceGate()
  {
    Destroy();
  }

  void CServiceGate::Create()
  {
    if(m_created) return;
    m_created = true;
  }

  void CServiceGate::Destroy()
  {
    if(!m_created) return;
    m_created = false;
  }

  bool CServiceGate::Register(const std::string& service_name_, CServiceServerImpl* service_)
  {
    if(!m_created) return(false);

    // register internal service implementation
    std::lock_guard<std::mutex> lock(m_service_sync);
    m_service_map.emplace(std::pair<std::string, CServiceServerImpl*>(service_name_, service_));

    return(true);
  }

  bool CServiceGate::Unregister(const std::string& service_name_, CServiceServerImpl* service_)
  {
    if(!m_created) return(false);
    bool ret_state = false;

    std::lock_guard<std::mutex> lock(m_service_sync);
    auto res = m_service_map.equal_range(service_name_);
    for(ServiceNameServiceImplMapT::iterator iter = res.first; iter != res.second; ++iter)
    {
      if(iter->second == service_)
      {
        m_service_map.erase(iter);
        break;
      }
    }

    return(ret_state);
  }

  void CServiceGate::ApplyClientRegistration(const eCAL::pb::Sample& ecal_sample_)
  {
    SClientAttr client;
    auto& ecal_sample_client = ecal_sample_.client();
    client.hname = ecal_sample_client.hname();
    client.pname = ecal_sample_client.pname();
    client.uname = ecal_sample_client.uname();
    client.sname = ecal_sample_client.sname();
    client.pid = static_cast<int>(ecal_sample_client.pid());

    // set client key
    client.key = client.sname + ":" + std::to_string(client.pid) + "@" + client.hname;

    // inform matching services
    std::lock_guard<std::mutex> lock(m_service_sync);
    auto res = m_service_map.equal_range(client.sname);
    for (ServiceNameServiceImplMapT::iterator iter = res.first; iter != res.second; ++iter)
    {
      iter->second->RegisterClient(client.key, client);
    }
  }

  void CServiceGate::RefreshRegistrations()
  {
    if (!m_created) return;

    // refresh service registrations
    std::lock_guard<std::mutex> lock(m_service_sync);
    for (auto iter : m_service_map)
    {
      iter.second->RefreshRegistration();
    }
  }
};
