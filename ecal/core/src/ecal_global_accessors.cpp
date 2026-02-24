/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
 * Copyright 2026 AUMOVIO and subsidiaries. All rights reserved.
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

#include "config/builder/logging_attribute_builder.h"
#include "util/unique_single_instance.h"
#include "logging/ecal_log_provider.h"
#include "logging/ecal_log_receiver.h"

#include <atomic>
#include <string>

namespace eCAL
{
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

  Logging::CLogProvider::CLogProviderUniquePtrT g_log_provider_instance;
  Logging::CLogReceiver::CLogReceiverUniquePtrT g_log_receiver_instance;

  void SetGlobalUnitName(const char *unit_name_)
  {
    if(unit_name_ != nullptr) g_unit_name = unit_name_;
    
    if (g_unit_name.empty())
    {
      g_unit_name = EcalUtils::Filesystem::BaseName(Process::GetProcessName());
    }
  }

  std::shared_ptr<CGlobals> CreateGlobalsInstance()
  {
    return CGlobals::instance();
  }

  bool FinalizeGlobals() 
  { 
    return CGlobals::instance()->Finalize(); 
  }

  void ResetGlobalEcalConfiguration() 
  { 
    g_ecal_configuration = Configuration(); 
  }

  void SetGlobalEcalConfiguration(const Configuration& config_)
  {
    g_ecal_configuration = config_;
  }

  void InitializeLogging(const eCAL::Configuration& config_)
  {
    g_log_provider_instance = Logging::CLogProvider::Create(eCAL::Logging::BuildLoggingProviderAttributes(config_));
    g_log_receiver_instance = Logging::CLogReceiver::Create(eCAL::Logging::BuildLoggingReceiverAttributes(config_));
  }

  void ResetLogging()
  {
    g_log_provider_instance.reset();
    g_log_receiver_instance.reset();
  }

  std::shared_ptr<CGlobals> g_globals()
  {
    if (auto globals_instance = CGlobals::instance(); globals_instance) 
    {
      return globals_instance->IsInitialized() ? std::move(globals_instance) : nullptr;
    }
    
    return nullptr;
  }

#if ECAL_CORE_MONITORING
  std::shared_ptr<CMonitoring> g_monitoring()
  {
    if (auto globals = g_globals(); globals) return globals->monitoring();
    return nullptr;
  }
#endif

#if ECAL_CORE_TIMEPLUGIN
  std::shared_ptr<CTimeGate> g_timegate()
  {
    if (auto globals = g_globals(); globals) return globals->timegate();
    return nullptr;
  }
#endif

#if ECAL_CORE_REGISTRATION
  std::shared_ptr<CRegistrationProvider> g_registration_provider()
  {
    if (auto globals = g_globals(); globals) return globals->registration_provider();
    return nullptr;
  }

  std::shared_ptr<CRegistrationReceiver> g_registration_receiver()
  {
    if (auto globals = g_globals(); globals) return globals->registration_receiver();
    return nullptr;
  }
#endif

  std::shared_ptr<CDescGate> g_descgate()
  {
    if (auto globals = g_globals(); globals) return globals->descgate();
    return nullptr;
  }

#if ECAL_CORE_SUBSCRIBER
  std::shared_ptr<CSubGate> g_subgate()
  {
    if (auto globals = g_globals(); globals) return globals->subgate();
    return nullptr;
  }
#endif

#if ECAL_CORE_PUBLISHER
  std::shared_ptr<CPubGate> g_pubgate()
  {
    if (auto globals = g_globals(); globals) return globals->pubgate();
    return nullptr;
  }
#endif

#if ECAL_CORE_SERVICE
  std::shared_ptr<CServiceGate> g_servicegate()
  {
    if (auto globals = g_globals(); globals) return globals->servicegate();
    return nullptr;
  }

  std::shared_ptr<CClientGate> g_clientgate()
  {
    if (auto globals = g_globals(); globals) return globals->clientgate();
    return nullptr;
  }
#endif

#if defined(ECAL_CORE_REGISTRATION_SHM) || defined(ECAL_CORE_TRANSPORT_SHM)
  std::shared_ptr<CMemFileThreadPool> g_memfile_pool()
  {
    if (auto globals = g_globals(); globals) return globals->memfile_pool();
    return nullptr;
  }

  std::shared_ptr<CMemFileMap> g_memfile_map()
  {
    if (auto globals = g_globals(); globals) return globals->memfile_map();
    return nullptr;
  }
#endif
}
