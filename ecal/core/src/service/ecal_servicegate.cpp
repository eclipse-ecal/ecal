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
 * @brief  eCAL service gateway class
**/

#include "ecal_servicegate.h"
#include "service/ecal_service_server_impl.h"

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

  bool CServiceGate::Register(CServiceServerImpl* service_)
  {
    if(!m_created) return(false);

    // register internal service
    std::unique_lock<std::shared_timed_mutex> lock(m_service_set_sync);
    m_service_set.insert(service_);

    return(true);
  }

  bool CServiceGate::Unregister(CServiceServerImpl* service_)
  {
    if(!m_created) return(false);
    bool ret_state(false);

    // unregister internal service
    std::unique_lock<std::shared_timed_mutex> lock(m_service_set_sync);
    for (auto iter = m_service_set.begin(); iter != m_service_set.end();)
    {
      if (*iter == service_)
      {
        iter = m_service_set.erase(iter);
        ret_state = true;
      }
      else
      {
        iter++;
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
    client.sid   = ecal_sample_client.sid();
    client.pid   = static_cast<int>(ecal_sample_client.pid());

    // create unique client key
    client.key = client.sname + ":" + client.sid + "@" + std::to_string(client.pid) + "@" + client.hname;

    // inform matching services
    {
      std::shared_lock<std::shared_timed_mutex> lock(m_service_set_sync);
      for (auto& iter : m_service_set)
      {
        if (iter->GetServiceName() == client.sname)
        {
          iter->RegisterClient(client.key, client);
        }
      }
    }
  }

  void CServiceGate::RefreshRegistrations()
  {
    if (!m_created) return;

    // refresh service registrations
    std::shared_lock<std::shared_timed_mutex> lock(m_service_set_sync);
    for (auto& iter : m_service_set)
    {
      iter->RefreshRegistration();
    }
  }
};
