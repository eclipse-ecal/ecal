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
  std::shared_ptr<CGlobals> CGlobals::instance()
  {
    auto instance = m_instance;
    if (instance) return instance;

    std::lock_guard<std::mutex> lock(m_instance_mutex);
    if (!m_instance)
    {
      m_instance = std::shared_ptr<CGlobals>(new CGlobals());
    }

    return m_instance;
  }

  CGlobals::~CGlobals()
  {
    Finalize();
  }

  bool CGlobals::Initialize(unsigned int components_)
  {
    // will be set if any new module was initialized
    bool new_initialization(false);

    /////////////////////
    // DESCRIPTION GATE
    /////////////////////
    if (!descgate_instance)
    {
      // create description gate with configured expiration timeout
      descgate_instance = std::make_shared<CDescGate>();
      new_initialization = true;
    }

#if defined(ECAL_CORE_REGISTRATION_SHM) || defined(ECAL_CORE_TRANSPORT_SHM)
    /////////////////////
    // MEMFILE MAP
    /////////////////////
    if (!memfile_map_instance)
    {
      memfile_map_instance = std::make_shared<CMemFileMap>();
      new_initialization = true;
    }

    /////////////////////
    // MEMFILE POOL
    /////////////////////
    if (!memfile_pool_instance)
    {
      memfile_pool_instance = std::make_shared<CMemFileThreadPool>(memfile_map_instance);
      new_initialization = true;
    }
#endif // defined(ECAL_CORE_REGISTRATION_SHM) || defined(ECAL_CORE_TRANSPORT_SHM)

#if ECAL_CORE_SUBSCRIBER
    /////////////////////
    // SUBSCRIBER GATE
    /////////////////////
    if ((components_ & Init::Subscriber) != 0u)
    {
      if (!subgate_instance)
      {
        subgate_instance = std::make_shared<CSubGate>();
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
      if (!pubgate_instance)
      {
        pubgate_instance = std::make_shared<CPubGate>();
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
      if (!servicegate_instance)
      {
        servicegate_instance = std::make_shared<CServiceGate>();
        new_initialization = true;
      }

      /////////////////////
      // CLIENT GATE
      /////////////////////
      if (!clientgate_instance)
      {
        clientgate_instance = std::make_shared<CClientGate>();
        new_initialization = true;
      }
    }
#endif // ECAL_CORE_SERVICE

#if ECAL_CORE_REGISTRATION
    const Registration::SAttributes registration_attr = BuildRegistrationAttributes(eCAL::GetConfiguration(), eCAL::Process::GetProcessID());
    /////////////////////
    // REGISTRATION PROVIDER
    /////////////////////
    if (!registration_provider_instance)
    {
      SRegistrationProviderContext registration_provider_context;
      registration_provider_context.attributes   = registration_attr;
      registration_provider_context.memfile_map  = memfile_map_instance;
      registration_provider_context.subgate      = subgate_instance;
      registration_provider_context.pubgate      = pubgate_instance;
      registration_provider_context.servicegate  = servicegate_instance;
      registration_provider_context.clientgate   = clientgate_instance;
      registration_provider_instance = std::make_shared<CRegistrationProvider>(registration_provider_context);
      new_initialization = true;
    }

    /////////////////////
    // REGISTRATION RECEIVER
    /////////////////////
    if(!registration_receiver_instance) 
    {
      SRegistrationReceiverContext registration_receiver_context;
      registration_receiver_context.attributes    = registration_attr;
      registration_receiver_context.memfile_map   = memfile_map_instance;
      registration_receiver_instance = std::make_shared<CRegistrationReceiver>(registration_receiver_context);
      new_initialization = true;
    }
#endif // ECAL_CORE_REGISTRATION

#if ECAL_CORE_MONITORING
    /////////////////////
    // MONITORING
    /////////////////////
    if ((components_ & Init::Monitoring) != 0u)
    {
      if (!monitoring_instance)
      {
        monitoring_instance = std::make_shared<CMonitoring>(registration_receiver_instance);
        new_initialization = true;
      }
    }
#endif // ECAL_CORE_MONITORING


    m_shm_reader_layer_instance = std::make_shared<eCAL::CSHMReaderLayer>(subgate_instance, memfile_pool_instance);
    m_udp_reader_layer_instance = std::make_shared<eCAL::CUDPReaderLayer>(subgate_instance);
    m_tcp_reader_layer_instance = std::make_shared<eCAL::CTCPReaderLayer>(subgate_instance);

    /////////////////////
    // START ALL
    /////////////////////
#if ECAL_CORE_REGISTRATION
    if (registration_provider_instance) registration_provider_instance->Start();
    if (registration_receiver_instance) registration_receiver_instance->Start();
#endif
    if (descgate_instance)
    {
#if ECAL_CORE_REGISTRATION
      // utilize registration receiver to get descriptions
      if (registration_receiver_instance)
        registration_receiver_instance->SetCustomApplySampleCallback("descgate", [this](const auto& sample_) {
          if (descgate_instance) descgate_instance->ApplySample(sample_, tl_none);
        });
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
#if ECAL_CORE_MONITORING
    if (monitoring_instance && ((components_ & Init::Monitoring) != 0u))    monitoring_instance->Start();
#endif
    
    components  |= components_;
    initialized.store(true);

    // eCAL needs to be marked as initialized, so that the timegate can register 
    // possible subscribers.
    // This is very fragile, and we should think about how it can be reworked
#if ECAL_CORE_TIMEPLUGIN
    /////////////////////
    // TIMEGATE
    /////////////////////
    if ((components_ & Init::TimeSync) != 0u)
    {
      if (timegate_instance == nullptr)
      {
        timegate_instance = CTimeGate::CreateTimegate();
        new_initialization = true;
      }
    }
#endif // ECAL_CORE_TIMEPLUGIN

    return new_initialization;
  }

  bool CGlobals::IsInitialized()
  {
    return initialized.load();
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
    // The timegate needs to be stopped first, 
    // but eCAL still treated as running
#if ECAL_CORE_TIMEPLUGIN
    timegate_instance.reset();
#endif

    // We expect true to return, otherwise intialization was not done or
    // another call to finalize is already in progress.
    if (!initialized.exchange(false)) return false;

    // start destruction
#if ECAL_CORE_MONITORING
    if (monitoring_instance)             monitoring_instance->Stop();
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
      if (registration_receiver_instance) registration_receiver_instance->RemCustomApplySampleCallback("descgate");
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

#if ECAL_CORE_MONITORING
    monitoring_instance.reset();
#endif
#if ECAL_CORE_SERVICE
    servicegate_instance.reset();
    clientgate_instance.reset();
#endif
#if ECAL_CORE_PUBLISHER
    pubgate_instance.reset();
#endif
#if ECAL_CORE_SUBSCRIBER
    subgate_instance.reset();
#endif
#if ECAL_CORE_REGISTRATION
    registration_receiver_instance.reset();
    registration_provider_instance.reset();
#endif
    descgate_instance.reset();
#if defined(ECAL_CORE_REGISTRATION_SHM) || defined(ECAL_CORE_TRANSPORT_SHM)
    memfile_pool_instance.reset();
    memfile_map_instance.reset();
#endif
    m_udp_reader_layer_instance.reset();
    m_tcp_reader_layer_instance.reset();
    m_shm_reader_layer_instance.reset();

    {
      std::lock_guard<std::mutex> lock(m_instance_mutex);
      m_instance.reset();
    }
    
    return true;
  }
}
