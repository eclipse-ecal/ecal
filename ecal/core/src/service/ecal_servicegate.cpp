/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2024 Continental Corporation
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
#include <atomic>
#include <mutex>
#include <shared_mutex>

namespace eCAL
{
  //////////////////////////////////////////////////////////////////
  // CServerGate
  //////////////////////////////////////////////////////////////////
  std::atomic<bool> CServiceGate::m_created;
  CServiceGate::CServiceGate() = default;

  CServiceGate::~CServiceGate()
  {
    Stop();
  }

  void CServiceGate::Start()
  {
    if(m_created) return;
    m_created = true;
  }

  void CServiceGate::Stop()
  {
    if(!m_created) return;

    // destroy all remaining server
    const std::shared_lock<std::shared_timed_mutex> lock(m_service_set_sync);
    for (const auto& service : m_service_set)
    {
      service->Stop();
    }

    m_created = false;
  }

  bool CServiceGate::Register(CServiceServerImpl* service_)
  {
    if(!m_created) return(false);

    // register internal service
    const std::unique_lock<std::shared_timed_mutex> lock(m_service_set_sync);
    m_service_set.insert(service_);

    return(true);
  }

  bool CServiceGate::Unregister(CServiceServerImpl* service_)
  {
    if(!m_created) return(false);
    bool ret_state(false);

    // unregister internal service
    const std::unique_lock<std::shared_timed_mutex> lock(m_service_set_sync);
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

  void CServiceGate::GetRegistrations(Registration::SampleList& reg_sample_list_)
  {
    if (!m_created) return;

    // read service registrations
    std::shared_lock<std::shared_timed_mutex> const lock(m_service_set_sync);
    for (const auto& service_server_impl : m_service_set)
    {
      reg_sample_list_.samples.emplace_back(service_server_impl->GetRegistration());
    }
  }
}
