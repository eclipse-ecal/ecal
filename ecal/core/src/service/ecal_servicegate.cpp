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
    const std::unique_lock<std::shared_timed_mutex> lock(m_service_server_map_mutex);
    m_service_server_map.clear();

    m_created = false;
  }

  bool CServiceGate::Register(const std::string& service_name_, const std::shared_ptr<CServiceServerImpl>& server_)
  {
    if(!m_created) return(false);

    // register internal service
    const std::unique_lock<std::shared_timed_mutex> lock(m_service_server_map_mutex);
    m_service_server_map.emplace(std::pair<std::string, std::shared_ptr<CServiceServerImpl>>(service_name_, server_));

    return(true);
  }

  bool CServiceGate::Unregister(const std::string& service_name_, const std::shared_ptr<CServiceServerImpl>& server_)
  {
    if (!m_created) return(false);
    bool ret_state = false;

    const std::unique_lock<std::shared_timed_mutex> lock(m_service_server_map_mutex);
    auto res = m_service_server_map.equal_range(service_name_);
    for (auto iter = res.first; iter != res.second; ++iter)
    {
      if (iter->second == server_)
      {
        m_service_server_map.erase(iter);
        ret_state = true;
        break;
      }
    }

    return(ret_state);
  }

  void CServiceGate::GetRegistrations(Registration::SampleList& reg_sample_list_)
  {
    if (!m_created) return;

    // read server registrations
    {
      const std::shared_lock<std::shared_timed_mutex> lock(m_service_server_map_mutex);
      for (const auto& iter : m_service_server_map)
      {
        reg_sample_list_.push_back(iter.second->GetRegistration());
      }
    }
  }
}
