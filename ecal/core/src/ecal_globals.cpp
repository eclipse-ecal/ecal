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

#include "ecal_globals.h"
#include "io/udp_init.h"
#include "ecal_config.h"

#include <stdexcept>

namespace eCAL
{
  CGlobals::CGlobals() : initialized(false), components(0)
  {}

  CGlobals::~CGlobals()
  {
    Finalize(Init::All);
  };

  int CGlobals::Initialize(unsigned int components_, std::vector<std::string>* config_keys_ /*= nullptr*/)
  {
    // will be set if any new module was initialized
    bool new_initialization(false);

    // this is needed here for functions like "GetHostName" on windows
    Net::Initialize();

    /////////////////////
    // CONFIG
    /////////////////////
    if (config_instance == nullptr)
    {
      config_instance = std::make_unique<CConfig>();
      if (config_keys_)
      {
        config_instance->OverwriteKeys(*config_keys_);
      }
      config_instance->AddFile(g_default_ini_file);

      if (!config_instance->Validate())
      {
        std::string emsg("Core initialization failed cause by a configuration error.");

        std::cerr                                                                 << std::endl;
        std::cerr << "----------------------------------------------------------" << std::endl;
        std::cerr << "eCAL CORE PANIC :-("                                        << std::endl;
        std::cerr                                                                 << std::endl;
        std::cerr << emsg                                                         << std::endl;
        std::cerr << "----------------------------------------------------------" << std::endl;
        std::cerr                                                                 << std::endl;
        
        throw std::runtime_error(emsg.c_str());
      }

      new_initialization = true;
    }

    /////////////////////
    // ENTITY REGISTER
    /////////////////////
    if (entity_register_instance == nullptr)
    {
      entity_register_instance = std::make_unique<CEntityRegister>();
      new_initialization = true;
    }

    /////////////////////
    // DESCRIPTION GATE
    /////////////////////
    if(descgate_instance == nullptr) 
    {
      descgate_instance = std::make_unique<CDescGate>();
      new_initialization = true;
    }

    /////////////////////
    // REGISTRATION GATE
    /////////////////////
    if(reggate_instance == nullptr) 
    {
      reggate_instance = std::make_unique<CRegGate>();
      new_initialization = true;
    }

#ifndef ECAL_LAYER_ICEORYX
    /////////////////////
    // MEMFILE MAP
    /////////////////////
    if(memfile_map_instance == nullptr) 
    {
      memfile_map_instance = std::make_unique<SMemFileMap>();
      new_initialization = true;
    }

    /////////////////////
    // MEMFILE POOL
    /////////////////////
    if(memfile_pool_instance == nullptr) 
    {
      memfile_pool_instance = std::make_unique<CMemFileThreadPool>();
      new_initialization = true;
    }
#endif /* !ECAL_LAYER_ICEORYX */

    /////////////////////
    // SUBSCRIBER GATE
    /////////////////////
    if (components_ & Init::Subscriber)
    {
      if (subgate_instance == nullptr)
      {
        subgate_instance = std::make_unique<CSubGate>();
        new_initialization = true;
      }
    }

    /////////////////////
    // PUBLISHER GATE
    /////////////////////
    if (components_ & Init::Publisher)
    {
      if (pubgate_instance == nullptr)
      {
        pubgate_instance = std::make_unique<CPubGate>();
        new_initialization = true;
      }
    }

    if (components_ & Init::Service)
    {
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

    /////////////////////
    // TIMEGATE
    /////////////////////
    if (components_ & Init::TimeSync)
    {
      if (timegate_instance == nullptr)
      {
        timegate_instance = std::make_unique<CTimeGate>();
        new_initialization = true;
      }
    }

    /////////////////////
    // MONITORING
    /////////////////////
    if (components_ & Init::Monitoring)
    {
      if (monitoring_instance == nullptr)
      {
        monitoring_instance = std::make_unique<CMonitoring>();
        new_initialization = true;
      }
    }

    /////////////////////
    // LOGGING
    /////////////////////
    if (components_ & Init::Logging)
    {
      if (log_instance == nullptr)
      {
        log_instance = std::make_unique<CLog>();
        new_initialization = true;
      }
    }

    /////////////////////
    // CREATE ALL
    /////////////////////
    //if (config_instance)                                          config_instance->Create();
    if (log_instance && (components_ & Init::Logging))            log_instance->Create();
    if (entity_register_instance)                                 entity_register_instance->Create(true, true, (components_ & Init::ProcessReg) != 0x0);
    if (descgate_instance)                                        descgate_instance->Create();
    if (reggate_instance)                                         reggate_instance->Create();
#ifndef ECAL_LAYER_ICEORYX
    if (memfile_pool_instance)                                    memfile_pool_instance->Create();
#endif /* !ECAL_LAYER_ICEORYX */
    if (subgate_instance && (components_ & Init::Subscriber))     subgate_instance->Create();
    if (pubgate_instance && (components_ & Init::Publisher))      pubgate_instance->Create();
    if (servicegate_instance && (components_ & Init::Service))     servicegate_instance->Create();
    if (clientgate_instance && (components_ & Init::Service))     clientgate_instance->Create();
    if (timegate_instance && (components_ & Init::TimeSync))      timegate_instance->Create(CTimeGate::eTimeSyncMode::realtime);
    if (monitoring_instance && (components_ & Init::Monitoring))  monitoring_instance->Create();

    initialized =  true;
    components  |= components_;

    if (new_initialization) return 0;
    else                    return 1;
  }

  int CGlobals::IsInitialized(unsigned int component_)
  {
    // check common initialization
    if (component_ == 0)
    {
      return(initialized);
    }

    // check single component initialization
    switch (component_)
    {
    case Init::Publisher:
      return(pubgate_instance != nullptr);
    case Init::Subscriber:
      return(subgate_instance != nullptr);
    case Init::Service:
      return(servicegate_instance != nullptr);
    case Init::Monitoring:
      return(monitoring_instance != nullptr);
    case Init::Logging:
      return(log_instance != nullptr);
    case Init::TimeSync:
      return(timegate_instance != nullptr);
    default:
      return(0);
    }
  }

  int CGlobals::Finalize(unsigned int /*components_*/)
  {
    if (!initialized) return(1);

    // start destruction
    if (monitoring_instance)       monitoring_instance->Destroy();
    if (timegate_instance)         timegate_instance->Destroy();
    if (clientgate_instance)       clientgate_instance->Destroy();
    if (servicegate_instance)       servicegate_instance->Destroy();
    if (pubgate_instance)          pubgate_instance->Destroy();
    if (subgate_instance)          subgate_instance->Destroy();
    if (reggate_instance)          reggate_instance->Destroy();
    if (descgate_instance)         descgate_instance->Destroy();
    if (entity_register_instance)  entity_register_instance->Destroy();
#ifndef ECAL_LAYER_ICEORYX
    if (memfile_pool_instance)     memfile_pool_instance->Destroy();
#endif /* !ECAL_LAYER_ICEORYX */
    if (log_instance)              log_instance->Destroy();
    //if (config_instance)           config_instance->Destroy();

    monitoring_instance       = nullptr;
    timegate_instance         = nullptr;
    servicegate_instance         = nullptr;
    pubgate_instance          = nullptr;
    subgate_instance          = nullptr;
    reggate_instance          = nullptr;
    descgate_instance         = nullptr;
    entity_register_instance  = nullptr;
#ifndef ECAL_LAYER_ICEORYX
    memfile_pool_instance     = nullptr;
    memfile_map_instance      = nullptr;
#endif /* !ECAL_LAYER_ICEORYX */
    log_instance              = nullptr;
    config_instance           = nullptr;

    // last not least we close all
    Net::Finalize();

    initialized = false;

    return(0);
  }
}
