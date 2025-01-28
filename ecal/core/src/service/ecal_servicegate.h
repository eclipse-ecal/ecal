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
 * @brief  eCAL server gateway class
**/

#pragma once

#include <ecal/namespace.h>
#include "ecal_def.h"
#include "serialization/ecal_struct_sample_registration.h"

#include <atomic>
#include <map>
#include <memory>
#include <shared_mutex>
#include <string>

namespace eCAL
{
  class CServiceServerImpl;

  class CServiceGate
  {
  public:
    CServiceGate();
    ~CServiceGate();

    void Start();
    void Stop();

    bool Register  (const std::string& service_name_, const std::shared_ptr<eCAL::CServiceServerImpl>& server_);
    bool Unregister(const std::string& service_name_, const std::shared_ptr<eCAL::CServiceServerImpl>& server_);

    void GetRegistrations(Registration::SampleList& reg_sample_list_);

  protected:
    static std::atomic<bool>      m_created;

    using ServiceNameServiceImplMapT = std::multimap<std::string, std::shared_ptr<eCAL::CServiceServerImpl>>;
    std::shared_timed_mutex       m_service_server_map_mutex;
    ServiceNameServiceImplMapT    m_service_server_map;
  };
}
