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
 * @brief  Global monitoring class
**/

#pragma once

#include <string>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100 4127 4146 4505 4800 4189 4592) // disable proto warnings
#endif
#include "ecal/pb/ecal.pb.h"
#include "ecal/pb/monitoring.pb.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif

namespace eCAL
{
  ////////////////////////////////////////
  // global database class
  ////////////////////////////////////////
  class CMonitoringImpl;
  class CMonitoring
  {
  public:
    CMonitoring();
    ~CMonitoring();
    
    void Create();
    void Destroy();

    void SetExclFilter(const std::string& filter_);
    void SetInclFilter(const std::string& filter_);
    void SetFilterState(bool state_);

    void Monitor(eCAL::pb::Monitoring& monitoring_);
    void Monitor(eCAL::pb::Logging& logging_);

    int PubMonitoring(bool state_, std::string& name_);
    int PubLogging(bool state_, std::string& name_);

    size_t ApplySample(const eCAL::pb::Sample& ecal_sample_);

  protected:
    CMonitoringImpl* m_monitoring_impl;

  private:
    CMonitoring(const CMonitoring&);                 // prevent copy-construction
    CMonitoring& operator=(const CMonitoring&);      // prevent assignment
  };
}
