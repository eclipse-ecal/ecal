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

#include "ecal_monitoring_def.h"
#include "ecal_monitoring_impl.h"
#include "ecal_global_accessors.h"

namespace eCAL
{
  CMonitoring::CMonitoring()
  {
    m_monitoring_impl = new CMonitoringImpl;
  }

  CMonitoring::~CMonitoring()
  {
    delete m_monitoring_impl;
    m_monitoring_impl = nullptr;
  }

  void CMonitoring::Create()
  {
    m_monitoring_impl->Create();
  }

  void CMonitoring::Destroy()
  {
    m_monitoring_impl->Destroy();
  }

  void CMonitoring::SetExclFilter(const std::string& filter_)
  {
    m_monitoring_impl->SetExclFilter(filter_);
  }

  void CMonitoring::SetInclFilter(const std::string& filter_)
  {
    m_monitoring_impl->SetInclFilter(filter_);
  }

  void CMonitoring::SetFilterState(bool state_)
  {
    m_monitoring_impl->SetFilterState(state_);
  }

  void CMonitoring::Monitor(eCAL::pb::Monitoring& monitoring_)
  {
    m_monitoring_impl->GetMonitoringMsg(monitoring_);
  }

  void CMonitoring::Monitor(eCAL::pb::Logging& logging_)
  {
    m_monitoring_impl->GetLoggingMsg(logging_);
  }

  int CMonitoring::PubMonitoring(bool state_, std::string& name_)
  {
    return(m_monitoring_impl->PubMonitoring(state_, name_));
  }

  int CMonitoring::PubLogging(bool state_, std::string& name_)
  {
    return(m_monitoring_impl->PubLogging(state_, name_));
  }

  size_t CMonitoring::ApplySample(const eCAL::pb::Sample & ecal_sample_)
  {
    if(m_monitoring_impl) return m_monitoring_impl->ApplySample(ecal_sample_, eCAL::pb::eTLayerType::tl_none);
    return 0;
  }

  namespace Monitoring
  {
    ////////////////////////////////////////////////////////
    // static library interface
    ////////////////////////////////////////////////////////
    int SetExclFilter(const std::string& filter_)
    {
      if (g_monitoring()) g_monitoring()->SetExclFilter(filter_);
      return(0);
    }

    int SetInclFilter(const std::string& filter_)
    {
      if (g_monitoring()) g_monitoring()->SetInclFilter(filter_);
      return(0);
    }

    int SetFilterState(const bool state_)
    {
      if (g_monitoring()) g_monitoring()->SetFilterState(state_);
      return(0);
    }

    int GetMonitoring(std::string& mon_)
    {
      eCAL::pb::Monitoring monitoring;
      if (g_monitoring()) g_monitoring()->Monitor(monitoring);

      mon_ = monitoring.SerializeAsString();
      return((int)mon_.size());
    }

    int GetLogging(std::string& log_)
    {
      eCAL::pb::Logging logging;
      if (g_monitoring()) g_monitoring()->Monitor(logging);

      log_ = logging.SerializeAsString();
      return((int)log_.size());
    }

    int PubMonitoring(bool state_, std::string name_ /* = "ecal.monitoring"*/)
    {
      if (g_monitoring()) return(g_monitoring()->PubMonitoring(state_, name_));
      return -1;
    }

    int PubLogging(bool state_, std::string name_ /* = "ecal.logging"*/)
    {
      if (g_monitoring()) return(g_monitoring()->PubLogging(state_, name_));
      return -1;
    }
  }
}
