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
  std::shared_ptr<CGlobals>     g_globals_ctx(nullptr);

  std::string                   g_default_ini_file(ECAL_DEFAULT_CFG);
  Configuration                 g_ecal_configuration{};

  std::string                   g_host_name;
  std::string                   g_unit_name;
  
  std::string                   g_process_name;
  std::string                   g_process_par;
  int                           g_process_id(0);
  std::string                   g_process_id_s;

  Types::Process::SProcessState g_process_state;
  std::mutex                    g_process_state_mutex;

  void SetGlobalUnitName(const char *unit_name_)
  {
    if(unit_name_ != nullptr) g_unit_name = unit_name_;
    
    if (g_unit_name.empty())
    {
      g_unit_name = EcalUtils::Filesystem::BaseName(Process::GetProcessName());
    }
  }

  std::shared_ptr<CGlobals> g_globals()
  {
    return g_globals_ctx;
  }

  std::shared_ptr<Logging::CLogReceiver> g_log_udp_receiver()
  {
    auto globals = g_globals();
    if (globals == nullptr) return nullptr;
    return globals->log_udp_receiver();
  }

  std::shared_ptr<Logging::CLogProvider> g_log_provider()
  {
    auto globals = g_globals();
    if (globals == nullptr) return nullptr;
    return globals->log_provider();
  }

  Configuration& g_ecal_config()
  {
    return g_ecal_configuration;
  }

#if ECAL_CORE_MONITORING
  std::shared_ptr<CMonitoring> g_monitoring()
  {
    auto globals = g_globals();
    if (globals == nullptr) return nullptr;
    return globals->monitoring();
  }
#endif

#if ECAL_CORE_TIMEPLUGIN
  std::shared_ptr<CTimeGate> g_timegate()
  {
    auto globals = g_globals();
    if (globals == nullptr) return nullptr;
    return globals->timegate();
  }
#endif

#if ECAL_CORE_REGISTRATION
  std::shared_ptr<CRegistrationProvider> g_registration_provider()
  {
    auto globals = g_globals();
    if (globals == nullptr) return nullptr;
    return globals->registration_provider();
  }

  std::shared_ptr<CRegistrationReceiver> g_registration_receiver()
  {
    auto globals = g_globals();
    if (globals == nullptr) return nullptr;
    return globals->registration_receiver();
  }
#endif

  std::shared_ptr<CDescGate> g_descgate()
  {
    auto globals = g_globals();
    if (globals == nullptr) return nullptr;
    return globals->descgate();
  }

#if ECAL_CORE_SUBSCRIBER
  std::shared_ptr<CSubGate> g_subgate()
  {
    auto globals = g_globals();
    if (globals == nullptr) return nullptr;
    return globals->subgate();
  }
#endif

#if ECAL_CORE_PUBLISHER
  std::shared_ptr<CPubGate> g_pubgate()
  {
    auto globals = g_globals();
    if (globals == nullptr) return nullptr;
    return globals->pubgate();
  }
#endif

#if ECAL_CORE_SERVICE
  std::shared_ptr<CServiceGate> g_servicegate()
  {
    auto globals = g_globals();
    if (globals == nullptr) return nullptr;
    return globals->servicegate();
  }

  std::shared_ptr<CClientGate> g_clientgate()
  {
    auto globals = g_globals();
    if (globals == nullptr) return nullptr;
    return globals->clientgate();
  }
#endif

#if defined(ECAL_CORE_REGISTRATION_SHM) || defined(ECAL_CORE_TRANSPORT_SHM)
  std::shared_ptr<CMemFileThreadPool> g_memfile_pool()
  {
    auto globals = g_globals();
    if (globals == nullptr) return nullptr;
    return globals->memfile_pool();
  }

  std::shared_ptr<CMemFileMap> g_memfile_map()
  {
    auto globals = g_globals();
    if (globals == nullptr) return nullptr;
    return globals->memfile_map();
  }
#endif
}
