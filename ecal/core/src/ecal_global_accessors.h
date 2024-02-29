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
 * @brief  eCAL core functions
**/

#pragma once
#include <atomic>
#include <string>
#include <vector>

#include <ecal/ecal_process_severity.h>

// Forward declaration of global accessible classes
namespace eCAL
{
  class  CGlobals;
  class  CConfig;
  class  CLog;
#if ECAL_CORE_MONITORING
  class  CMonitoring;
#endif
#if ECAL_CORE_TIMEPLUGIN
  class  CTimeGate;
#endif
#if ECAL_CORE_REGISTRATION
  class  CRegistrationProvider;
  class  CRegistrationReceiver;
#endif
  class  CDescGate;
#if ECAL_CORE_SUBSCRIBER
  class  CSubGate;
#endif
#if ECAL_CORE_PUBLISHER
  class  CPubGate;
#endif
#if ECAL_CORE_SERVICE
  class  CServiceGate;
  class  CClientGate;
#endif
#if defined(ECAL_CORE_REGISTRATION_SHM) || defined(ECAL_CORE_TRANSPORT_SHM)
  class  CMemFileThreadPool;
  class  CMemFileMap;
#endif

  // Declaration of getter functions for globally accessible variable instances
  CGlobals*               g_globals();
  CConfig*                g_config();
  CLog*                   g_log();
#if ECAL_CORE_MONITORING
  CMonitoring*            g_monitoring();
#endif
#if ECAL_CORE_TIMEPLUGIN
  CTimeGate*              g_timegate();
#endif
#if ECAL_CORE_REGISTRATION
  CRegistrationProvider*  g_registration_provider();
  CRegistrationReceiver*  g_registration_receiver();
#endif
  CDescGate*              g_descgate();
#if ECAL_CORE_SUBSCRIBER
  CSubGate*               g_subgate();
#endif
#if ECAL_CORE_PUBLISHER
  CPubGate*               g_pubgate();
#endif
#if ECAL_CORE_SERVICE
  CServiceGate*           g_servicegate();
  CClientGate*            g_clientgate();
#endif
#if defined(ECAL_CORE_REGISTRATION_SHM) || defined(ECAL_CORE_TRANSPORT_SHM)
  CMemFileThreadPool*     g_memfile_pool();
  CMemFileMap*            g_memfile_map();
#endif

  // declaration of globally accessible variables
  extern CGlobals*                     g_globals_ctx;
  extern std::atomic<int>              g_globals_ctx_ref_cnt;

  extern std::string                   g_default_ini_file;

  extern std::string                   g_host_name;
  extern std::string                   g_unit_name;
  extern std::vector<std::string>      g_task_parameter;

  extern std::string                   g_process_name;
  extern std::string                   g_process_par;
  extern int                           g_process_id;
  extern std::string                   g_process_id_s;
  extern std::string                   g_process_info;

  extern eCAL_Process_eSeverity        g_process_severity;
  extern eCAL_Process_eSeverity_Level  g_process_severity_level;
}
