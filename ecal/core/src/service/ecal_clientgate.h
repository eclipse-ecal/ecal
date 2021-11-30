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

#pragma once

#include "ecal_def.h"
#include "ecal_expmap.h"

#include <ecal/ecal_callback.h>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100 4127 4146 4505 4800 4189 4592) // disable proto warnings
#endif
#include "ecal/pb/ecal.pb.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <mutex>
#include <atomic>
#include <map>

namespace eCAL
{
  class CServiceClientImpl;

  class CClientGate
  {
  public:
    CClientGate();
    ~CClientGate();

    void Create();
    void Destroy();

    bool Register  (const std::string& service_name_, CServiceClientImpl* service_);
    bool Unregister(const std::string& service_name_, CServiceClientImpl* service_);

    void ApplyServiceRegistration(const eCAL::pb::Sample& ecal_sample_);

    std::vector<SServiceAttr> GetServiceAttr(const std::string& service_name_);

    void RefreshRegistrations();

  protected:
    static std::atomic<bool>    m_created;

    typedef std::multimap<std::string, CServiceClientImpl*> ServiceNameServiceImplMapT;
    std::mutex                  m_client_sync;
    ServiceNameServiceImplMapT  m_client_map;

    typedef Util::CExpMap<std::string, SServiceAttr> ConnectedMapT;
    std::mutex                  m_service_register_sync;
    ConnectedMapT               m_service_register_map;
  };
};
