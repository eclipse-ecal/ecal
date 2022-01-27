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

#pragma once

#include "ecal_def.h"

#include <ecal/ecal_callback.h>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100 4127 4146 4505 4800 4189 4592) // disable proto warnings
#endif
#include "ecal/pb/ecal.pb.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <atomic>
#include <shared_mutex>
#include <set>

namespace eCAL
{
  class CServiceServerImpl;

  class CServiceGate
  {
  public:
    CServiceGate();
    ~CServiceGate();

    void Create();
    void Destroy();

    bool Register  (CServiceServerImpl* service_);
    bool Unregister(CServiceServerImpl* service_);

    void ApplyClientRegistration(const eCAL::pb::Sample& ecal_sample_);

    void RefreshRegistrations();

  protected:
    static std::atomic<bool>    m_created;

    typedef std::set<CServiceServerImpl*> ServiceNameServiceImplSetT;
    std::shared_timed_mutex     m_service_set_sync;
    ServiceNameServiceImplSetT  m_service_set;
  };
};
