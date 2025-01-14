/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2024 Continental Corporation
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

#include "ecal_globals.h"

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#if ECAL_CORE_SERVICE
#include "service/ecal_service_singleton_manager.h"
#endif

#include "ecal_config_internal.h"
#include "config/builder/registration_attribute_builder.h"
#include "config/builder/logging_attribute_builder.h"

namespace eCAL
{
  CGlobals::CGlobals() : initialized(false), components(0)
  {}

  CGlobals::~CGlobals()
  {
    Finalize();
  }

  bool CGlobals::Initialize(unsigned int components_)
  {
    // will be set if any new module was initialized
    bool new_initialization(false);

#if ECAL_CORE_REGISTRATION
    const Registration::SAttributes registration_attr = BuildRegistrationAttributes(GetRegistrationConfiguration(), GetTransportLayerConfiguration().udp, eCAL::Process::GetProcessID());
    /////////////////////
    // REGISTRATION PROVIDER
    /////////////////////
    if (registration_provider_instance == nullptr)
    {
      registration_provider_instance = std::make_unique<CRegistrationProvider>(registration_attr);
      new_initialization = true;
    }

    /////////////////////
    // REGISTRATION RECEIVER
    /////////////////////
    if(registration_receiver_instance == nullptr) 
    {
      registration_receiver_instance = std::make_unique<CRegistrationReceiver>(registration_attr);
      new_initialization = true;
    }
#endif // ECAL_CORE_REGISTRATION

    /////////////////////
    // DESCRIPTION GATE
    /////////////////////
    if (descgate_instance == nullptr)
    {
      // create description gate with configured expiration timeout
      descgate_instance = std::make_unique<CDescGate>();
      new_initialization = true;
    }

#if defined(ECAL_CORE_REGISTRATION_SHM) || defined(ECAL_CORE_TRANSPORT_SHM)
    /////////////////////
    // MEMFILE MAP
    /////////////////////
    if (memfile_map_instance == nullptr)
    {
      memfile_map_instance = std::make_unique<CMemFileMap>();
      new_initialization = true;
    }

    /////////////////////
    // MEMFILE POOL
    /////////////////////
    if (memfile_pool_instance == nullptr)
    {
      memfile_pool_instance = std::make_unique<CMemFileThreadPool>();
      new_initialization = true;
    }
#endif // defined(ECAL_CORE_REGISTRATION_SHM) || defined(ECAL_CORE_TRANSPORT_SHM)

#if ECAL_CORE_SUBSCRIBER
    /////////////////////
    // SUBSCRIBER GATE
    /////////////////////
    if ((components_ & Init::Subscriber) != 0u)
    {
      if (subgate_instance == nullptr)
      {
        subgate_instance = std::make_unique<CSubGate>();
        new_initialization = true;
      }
    }
#endif // ECAL_CORE_SUBSCRIBER

#if ECAL_CORE_PUBLISHER
    /////////////////////
    // PUBLISHER GATE
    /////////////////////
    if ((components_ & Init::Publisher) != 0u)
    {
      if (pubgate_instance == nullptr)
      {
        pubgate_instance = std::make_unique<CPubGate>();
        new_initialization = true;
      }
    }
#endif // ECAL_CORE_PUBLISHER

#if ECAL_CORE_SERVICE
    if ((components_ & Init::Service) != 0u)
    {
      // Reset the service manager, so it will be able to create new services, again
      eCAL::service::ServiceManager::instance()->reset();

      /////////////////////
      // SERVICE GATE
      /////////////////////
      if (servicegate_instance == nullptr)
      {
        servicegate_instance = std::make_unique<CServiceGate>();
        new_initialization = true;
      }

      /////////////////////
      // CLIENT GATE
      /////////////////////
      if (clientgate_instance == nullptr)
      {
        clientgate_instance = std::make_unique<CClientGate>();
        new_initialization = true;
      }
    }
#endif // ECAL_CORE_SERVICE

#if ECAL_CORE_TIMEPLUGIN
    /////////////////////
    // TIMEGATE
    /////////////////////
    if ((components_ & Init::TimeSync) != 0u)
    {
      if (timegate_instance == nullptr)
      {
        timegate_instance = std::make_unique<CTimeGate>();
        new_initialization = true;
      }
    }
#endif // ECAL_CORE_TIMEPLUGIN

#if ECAL_CORE_MONITORING
    /////////////////////
    // MONITORING
    /////////////////////
    if ((components_ & Init::Monitoring) != 0u)
    {
      if (monitoring_instance == nullptr)
      {
        monitoring_instance = std::make_unique<CMonitoring>();
        new_initialization = true;
      }
    }
#endif // ECAL_CORE_MONITORING

    /////////////////////
    // LOGGING
    /////////////////////
    if ((components_ & Init::Logging) != 0u)
    {
      if (log_provider_instance == nullptr)
      {
        log_provider_instance = std::make_unique<Logging::CLogProvider>(eCAL::Logging::BuildLoggingProviderAttributes(GetLoggingConfiguration(), GetRegistrationConfiguration(), GetTransportLayerConfiguration()));
        new_initialization = true;
      }

      if (log_udp_receiver_instance == nullptr)
      {
        log_udp_receiver_instance = std::make_unique<Logging::CLogReceiver>(eCAL::Logging::BuildLoggingReceiverAttributes(GetLoggingConfiguration(), GetRegistrationConfiguration(), GetTransportLayerConfiguration()));
        new_initialization = true;
      }
    }

    /////////////////////
    // START ALL
    /////////////////////
    if (log_provider_instance && ((components_ & Init::Logging) != 0u))
    {
      log_provider_instance->Start();
      log_udp_receiver_instance->Start();
    }
#if ECAL_CORE_REGISTRATION
    if (registration_provider_instance)                                           registration_provider_instance->Start();
    if (registration_receiver_instance)                                           registration_receiver_instance->Start();
#endif
    if (descgate_instance)
    {
#if ECAL_CORE_REGISTRATION
      // utilize registration receiver to get descriptions
      g_registration_receiver()->SetCustomApplySampleCallback("descgate", [](const auto& sample_) {g_descgate()->ApplySample(sample_, tl_none); });
#endif
    }
#if defined(ECAL_CORE_REGISTRATION_SHM) || defined(ECAL_CORE_TRANSPORT_SHM)
    if (memfile_pool_instance)                                              memfile_pool_instance->Start();
#endif
#if ECAL_CORE_SUBSCRIBER
    if (subgate_instance && ((components_ & Init::Subscriber) != 0u))       subgate_instance->Start();
#endif
#if ECAL_CORE_PUBLISHER
    if (pubgate_instance && ((components_ & Init::Publisher) != 0u))        pubgate_instance->Start();
#endif
#if ECAL_CORE_SERVICE
    if (servicegate_instance && ((components_ & Init::Service) != 0u))      servicegate_instance->Start();
    if (clientgate_instance && ((components_ & Init::Service) != 0u))       clientgate_instance->Start();
#endif
#if ECAL_CORE_TIMEPLUGIN
    if (timegate_instance && ((components_ & Init::TimeSync) != 0u))        timegate_instance->Start(CTimeGate::eTimeSyncMode::realtime);
#endif
#if ECAL_CORE_MONITORING
    if (monitoring_instance && ((components_ & Init::Monitoring) != 0u))    monitoring_instance->Start();
#endif
    initialized =  true;
    components  |= components_;

    return new_initialization;
  }

  bool CGlobals::IsInitialized()
  {
    return initialized;
  }

  bool CGlobals::IsInitialized(unsigned int component_)
  {
    // check single component initialization
    switch (component_)
    {
#if ECAL_CORE_PUBLISHER
    case Init::Publisher:
      return(pubgate_instance != nullptr);
#endif
#if ECAL_CORE_SUBSCRIBER
    case Init::Subscriber:
      return(subgate_instance != nullptr);
#endif
#if ECAL_CORE_SERVICE
    case Init::Service:
      return(servicegate_instance != nullptr);
#endif
#if ECAL_CORE_MONITORING
    case Init::Monitoring:
      return(monitoring_instance != nullptr);
#endif
    case Init::Logging:
      return(log_provider_instance != nullptr);
#if ECAL_CORE_TIMEPLUGIN
    case Init::TimeSync:
      return(timegate_instance != nullptr);
#endif
    default:
      return(false);
    }
  }

  bool CGlobals::Finalize()
  {
    if (!initialized) return false;

    // start destruction
#if ECAL_CORE_MONITORING
    if (monitoring_instance)             monitoring_instance->Stop();
#endif
#if ECAL_CORE_TIMEPLUGIN
    if (timegate_instance)               timegate_instance->Stop();
#endif
#if ECAL_CORE_SERVICE
    // The order here is EXTREMELY important! First, the actual service
    // implementation must be stopped (->Service Manager), then the
    // clientgate/servicegate. The callbacks in the service implementation carry
    // raw pointers to the gate's functions, so we must make sure that everything
    // has been executed, before we delete the gates.
    eCAL::service::ServiceManager::instance()->stop();
    if (clientgate_instance)             clientgate_instance->Stop();
    if (servicegate_instance)            servicegate_instance->Stop();
#endif
#if ECAL_CORE_PUBLISHER
    if (pubgate_instance)                pubgate_instance->Stop();
#endif
#if ECAL_CORE_SUBSCRIBER
    if (subgate_instance)                subgate_instance->Stop();
#endif
    if (descgate_instance)
    {
#if ECAL_CORE_REGISTRATION
      // stop registration receiver utilization to get descriptions
      g_registration_receiver()->RemCustomApplySampleCallback("descgate");
#endif
    }
#if ECAL_CORE_REGISTRATION
    if (registration_receiver_instance)  registration_receiver_instance->Stop();
    if (registration_provider_instance)  registration_provider_instance->Stop();
#endif
#if defined(ECAL_CORE_REGISTRATION_SHM) || defined(ECAL_CORE_TRANSPORT_SHM)
    if (memfile_pool_instance)           memfile_pool_instance->Stop();
    if (memfile_map_instance)            memfile_map_instance->Stop();
#endif
    if (log_udp_receiver_instance)       log_udp_receiver_instance->Stop();
    if (log_provider_instance)           log_provider_instance->Stop();

#if ECAL_CORE_MONITORING
    monitoring_instance             = nullptr;
#endif
#if ECAL_CORE_TIMEPLUGIN
    timegate_instance               = nullptr;
#endif
#if ECAL_CORE_SERVICE
    servicegate_instance            = nullptr;
    clientgate_instance             = nullptr;
#endif
#if ECAL_CORE_PUBLISHER
    pubgate_instance                = nullptr;
#endif
#if ECAL_CORE_SUBSCRIBER
    subgate_instance                = nullptr;
#endif
#if ECAL_CORE_REGISTRATION
    registration_receiver_instance  = nullptr;
    registration_provider_instance  = nullptr;
#endif
    descgate_instance               = nullptr;
#if defined(ECAL_CORE_REGISTRATION_SHM) || defined(ECAL_CORE_TRANSPORT_SHM)
    memfile_pool_instance           = nullptr;
    memfile_map_instance            = nullptr;
#endif
    log_provider_instance           = nullptr;
    log_udp_receiver_instance       = nullptr;
    
    initialized = false;

    // reset configuration to default values
    g_ecal_configuration = Configuration();

    return true;
  }
}
