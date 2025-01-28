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
 * @brief  eCAL core functions
**/

#include "ecal/config/configuration.h"

#include "ecal_global_accessors.h"
#include "ecal_def.h"
#include "ecal_globals.h"
#include "ecal_utils/filesystem.h"

#include <atomic>
#include <string>

namespace eCAL
{
  CGlobals*                     g_globals_ctx(nullptr);
  std::atomic<int>              g_globals_ctx_ref_cnt;

  std::string                   g_default_ini_file(ECAL_DEFAULT_CFG);
  Configuration                 g_ecal_configuration{};

  std::string                   g_host_name;
  std::string                   g_unit_name;
  std::vector<std::string>      g_task_parameter;

  std::string                   g_process_name;
  std::string                   g_process_par;
  int                           g_process_id(0);
  std::string                   g_process_id_s;
  std::string                   g_process_info;

  eCAL::Process::eSeverity        g_process_severity(eCAL::Process::eSeverity::unknown);
  eCAL::Process::eSeverityLevel  g_process_severity_level(eCAL::Process::eSeverityLevel::level1);

  void InitGlobals()
  {
    if (g_globals_ctx == nullptr)
      g_globals_ctx = new CGlobals;
  }

  void SetGlobalUnitName(const char *unit_name_)
  {
    if(unit_name_ != nullptr) g_unit_name = unit_name_;
    
    if (g_unit_name.empty())
    {
      g_unit_name = EcalUtils::Filesystem::BaseName(Process::GetProcessName());
    }
  }

  CGlobals* g_globals()
  {
    return g_globals_ctx;
  }

  Logging::CLogReceiver* g_log_udp_receiver()
  {
    if (g_globals() == nullptr) return(nullptr);
    return(g_globals()->log_udp_receiver().get());
  }

  Logging::CLogProvider* g_log_provider()
  {
    if (g_globals() == nullptr) return(nullptr);
    return(g_globals()->log_provider().get());
  }

  Configuration& g_ecal_config()
  {
    return(g_ecal_configuration);
  }

#if ECAL_CORE_MONITORING
  CMonitoring* g_monitoring()
  {
    if (g_globals() == nullptr) return(nullptr);
    return(g_globals()->monitoring().get());
  }
#endif

#if ECAL_CORE_TIMEPLUGIN
  CTimeGate* g_timegate()
  {
    if (g_globals() == nullptr) return(nullptr);
    return(g_globals()->timegate().get());
  }
#endif

#if ECAL_CORE_REGISTRATION
  CRegistrationProvider* g_registration_provider()
  {
    if (g_globals() == nullptr) return(nullptr);
    return(g_globals()->registration_provider().get());
  }

  CRegistrationReceiver* g_registration_receiver()
  {
    if (g_globals() == nullptr) return(nullptr);
    return(g_globals()->registration_receiver().get());
  }
#endif

  CDescGate* g_descgate()
  {
    if (g_globals() == nullptr) return(nullptr);
    return(g_globals()->descgate().get());
  }

#if ECAL_CORE_SUBSCRIBER
  CSubGate* g_subgate()
  {
    if (g_globals() == nullptr) return(nullptr);
    return(g_globals()->subgate().get());
  }
#endif

#if ECAL_CORE_PUBLISHER
  CPubGate* g_pubgate()
  {
    if (g_globals() == nullptr) return(nullptr);
    return(g_globals()->pubgate().get());
  }
#endif

#if ECAL_CORE_SERVICE
  CServiceGate* g_servicegate()
  {
    if (g_globals() == nullptr) return(nullptr);
    return(g_globals()->servicegate().get());
  }

  CClientGate* g_clientgate()
  {
    if (g_globals() == nullptr) return(nullptr);
    return(g_globals()->clientgate().get());
  }
#endif

#if defined(ECAL_CORE_REGISTRATION_SHM) || defined(ECAL_CORE_TRANSPORT_SHM)
  CMemFileThreadPool* g_memfile_pool()
  {
    if (g_globals() == nullptr) return(nullptr);
    return(g_globals()->memfile_pool().get());
  }

  CMemFileMap* g_memfile_map()
  {
    if (g_globals() == nullptr) return(nullptr);
    return(g_globals()->memfile_map().get());
  }
#endif
}
