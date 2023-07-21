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

#include <ecal/types/monitoring.h>
#include <string>

#ifdef _MSC_VER
#pragma warning(push, 0) // disable proto warnings
#endif
#include <ecal/core/pb/ecal.pb.h>
#include <ecal/core/pb/monitoring.pb.h>
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

    void GetMonitoring(eCAL::pb::Monitoring& monitoring_, unsigned int entities_ = Monitoring::Entity::All);
    void GetMonitoring(eCAL::Monitoring::SMonitoring& monitoring_, unsigned int entities_ = Monitoring::Entity::All);
    void GetLogging(eCAL::pb::Logging& logging_);

    int PubMonitoring(bool state_, std::string& name_);
    int PubLogging(bool state_, std::string& name_);

    bool ApplySample(const eCAL::pb::Sample& ecal_sample_);

  protected:
    CMonitoringImpl* m_monitoring_impl = nullptr;

  private:
    CMonitoring(const CMonitoring&);                 // prevent copy-construction
    CMonitoring& operator=(const CMonitoring&);      // prevent assignment
  };
}
