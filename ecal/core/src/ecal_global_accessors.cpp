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

#include "ecal_global_accessors.h"
#include "ecal_globals.h"

namespace eCAL
{
  CGlobals*                     g_globals_ctx(nullptr);
  std::atomic<int>              g_globals_ctx_ref_cnt;
  std::atomic<int>              g_shutdown;

  std::string                   g_default_ini_file(ECAL_DEFAULT_CFG);

  std::string                   g_host_name;
  int                           g_host_id(0);
  std::string                   g_unit_name;
  std::vector<std::string>      g_task_parameter;

  std::string                   g_process_name;
  std::string                   g_process_par;
  int                           g_process_id(0);
  std::string                   g_process_id_s;
  std::string                   g_process_info;

  eCAL_Process_eSeverity        g_process_severity(proc_sev_unknown);
  eCAL_Process_eSeverity_Level  g_process_severity_level(proc_sev_level1);

  std::atomic<long long>        g_process_wclock;
  std::atomic<long long>        g_process_wbytes;
  std::atomic<long long>        g_process_wbytes_sum;

  std::atomic<long long>        g_process_rclock;
  std::atomic<long long>        g_process_rbytes;
  std::atomic<long long>        g_process_rbytes_sum;


  CGlobals* g_globals()
  {
    return g_globals_ctx;
  }

  CConfig* g_config()
  {
    if (!g_globals()) return(nullptr);
    return(g_globals()->config().get());
  }

  CLog* g_log()
  {
    if (!g_globals()) return(nullptr);
    return(g_globals()->log().get());
  }

  CMonitoring* g_monitoring()
  {
    if (!g_globals()) return(nullptr);
    return(g_globals()->monitoring().get());
  }

  CTimeGate* g_timegate()
  {
    if (!g_globals()) return(nullptr);
    return(g_globals()->timegate().get());
  }

  CEntityRegister* g_entity_register()
  {
    if (!g_globals()) return(nullptr);
    return(g_globals()->entity_register().get());
  }

  CDescGate* g_descgate()
  {
    if (!g_globals()) return(nullptr);
    return(g_globals()->descgate().get());
  }

  CSubGate* g_subgate()
  {
    if (!g_globals()) return(nullptr);
    return(g_globals()->subgate().get());
  }

  CPubGate* g_pubgate()
  {
    if (!g_globals()) return(nullptr);
    return(g_globals()->pubgate().get());
  }

  CServiceGate* g_servicegate()
  {
    if (!g_globals()) return(nullptr);
    return(g_globals()->servicegate().get());
  }

  CClientGate* g_clientgate()
  {
    if (!g_globals()) return(nullptr);
    return(g_globals()->clientgate().get());
  }

  CRegGate* g_reggate()
  {
    if (!g_globals()) return(nullptr);
    return(g_globals()->reggate().get());
  }

#ifndef ECAL_LAYER_ICEORYX
  CMemFileThreadPool* g_memfile_pool()
  {
    if (!g_globals()) return(nullptr);
    return(g_globals()->memfile_pool().get());
  }

  SMemFileMap* g_memfile_map()
  {
    if (!g_globals()) return(nullptr);
    return(g_globals()->memfile_map().get());
  }
#endif /* !ECAL_LAYER_ICEORYX */
}
