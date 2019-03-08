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
#include "service/ecal_service_server_impl.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4146 4800)
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
  class CServGate
  {
  public:
    CServGate();
    ~CServGate();

    void Create();
    void Destroy();

    bool Register(const std::string& service_name_, CServiceServerImpl* service_);
    bool Unregister(const std::string& service_name_, CServiceServerImpl* service_);

    struct SService
    {
      SService() : pid(0), tcp_port(0) {};
      std::string    hname;
      std::string    pname;
      std::string    uname;
      int            pid;
      std::string    sname;
      unsigned short tcp_port;
    };
    std::vector<SService> GetServiceInfo(const std::string& service_name_);

    void ApplyServiceRegistration(const eCAL::pb::Sample& ecal_sample_);
    void RefreshRegistrations();

  protected:
    static std::atomic<bool>    m_created;

    typedef std::multimap<std::string, CServiceServerImpl*> ServiceNameServiceImplMapT;
    std::mutex                  m_internal_service_sync;
    ServiceNameServiceImplMapT  m_internal_service_map;

    typedef std::chrono::steady_clock RegClockT;
    typedef std::map<std::string, std::pair<SService, RegClockT::time_point>> ServiceNameServiceMapT;
    std::mutex                  m_service_register_sync;
    ServiceNameServiceMapT      m_service_register_map;
  };
};
