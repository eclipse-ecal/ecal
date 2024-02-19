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

#include "ecal_global_accessors.h"
#include <string>
#include <vector>
#if ECAL_CORE_REGISTRATION
#include "registration/ecal_registration_provider.h"
#include "registration/ecal_registration_receiver.h"
#endif 
#include "time/ecal_timegate.h"
#include "logging/ecal_log_impl.h"
#if ECAL_CORE_MONITORING
#include "monitoring/ecal_monitoring_def.h"
#endif
#if ECAL_CORE_PUBLISHER
#include "pubsub/ecal_pubgate.h"
#endif
#if ECAL_CORE_SUBSCRIBER
#include "pubsub/ecal_subgate.h"
#endif
#if defined(ECAL_CORE_REGISTRATION_SHM) || defined(ECAL_CORE_TRANSPORT_SHM)
#include "io/shm/ecal_memfile_pool.h"
#include "io/shm/ecal_memfile_db.h"
#endif
#if ECAL_CORE_SERVICE
#include "service/ecal_servicegate.h"
#include "service/ecal_clientgate.h"
#endif
#include "ecal_descgate.h"

#include <memory>

namespace eCAL
{
  class CGlobals
  {
  public:
    CGlobals();
    ~CGlobals();

    int Initialize     ( unsigned int components_, std::vector<std::string>* config_keys_ = nullptr);
    bool IsInitialized ( unsigned int component_  );

    unsigned int GetComponents() const { return(components); };

    int Finalize(unsigned int components_);

    const std::unique_ptr<CConfig>&                                       config()                 { return config_instance; };
    const std::unique_ptr<CLog>&                                          log()                    { return log_instance; };
#if ECAL_CORE_MONITORING
    const std::unique_ptr<CMonitoring>&                                   monitoring()             { return monitoring_instance; };
#endif
#if ECAL_CORE_TIMEPLUGIN
    const std::unique_ptr<CTimeGate>&                                     timegate()               { return timegate_instance; };
#endif
#if ECAL_CORE_SUBSCRIBER
    const std::unique_ptr<CSubGate>&                                      subgate()                { return subgate_instance; };
#endif
#if ECAL_CORE_PUBLISHER
    const std::unique_ptr<CPubGate>&                                      pubgate()                { return pubgate_instance; };
#endif
#if ECAL_CORE_SERVICE
    const std::unique_ptr<CServiceGate>&                                  servicegate()            { return servicegate_instance; };
    const std::unique_ptr<CClientGate>&                                   clientgate()             { return clientgate_instance; };
#endif
#if ECAL_CORE_REGISTRATION
    const std::unique_ptr<CRegistrationProvider>&                         registration_provider()  { return registration_provider_instance; };
    const std::unique_ptr<CRegistrationReceiver>&                         registration_receiver()  { return registration_receiver_instance; };
#endif
#if defined(ECAL_CORE_REGISTRATION_SHM) || defined(ECAL_CORE_TRANSPORT_SHM)
    const std::unique_ptr<CMemFileThreadPool>&                            memfile_pool()           { return memfile_pool_instance; };
    const std::unique_ptr<CMemFileMap>&                                   memfile_map()            { return memfile_map_instance; };
#endif
    const std::unique_ptr<CDescGate>&                                     descgate()               { return descgate_instance; };

  private:
    bool                                                                  initialized;
    unsigned int                                                          components;
    std::unique_ptr<CConfig>                                              config_instance;
    std::unique_ptr<CLog>                                                 log_instance;
#if ECAL_CORE_MONITORING
    std::unique_ptr<CMonitoring>                                          monitoring_instance;
#endif
#if ECAL_CORE_TIMEPLUGIN
    std::unique_ptr<CTimeGate>                                            timegate_instance;
#endif
#if ECAL_CORE_SUBSCRIBER
    std::unique_ptr<CSubGate>                                             subgate_instance;
#endif
#if ECAL_CORE_PUBLISHER
    std::unique_ptr<CPubGate>                                             pubgate_instance;
#endif
#if ECAL_CORE_SERVICE
    std::unique_ptr<CServiceGate>                                         servicegate_instance;
    std::unique_ptr<CClientGate>                                          clientgate_instance;
#endif
#if ECAL_CORE_REGISTRATION
    std::unique_ptr<CRegistrationProvider>                                registration_provider_instance;
    std::unique_ptr<CRegistrationReceiver>                                registration_receiver_instance;
#endif
#if defined(ECAL_CORE_REGISTRATION_SHM) || defined(ECAL_CORE_TRANSPORT_SHM)
    std::unique_ptr<CMemFileThreadPool>                                   memfile_pool_instance;
    std::unique_ptr<CMemFileMap>                                          memfile_map_instance;
#endif
    std::unique_ptr<CDescGate>                                            descgate_instance;
  };
}
