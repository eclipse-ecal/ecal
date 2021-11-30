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
#include "ecal_reggate.h"
#include "ecal_descgate.h"
#include "ecal_timegate.h"
#include "ecal_register.h"
#include "ecal_log_impl.h"
#include "mon/ecal_monitoring_def.h"
#include "pubsub/ecal_pubgate.h"
#include "pubsub/ecal_subgate.h"
#include "service/ecal_servicegate.h"
#include "service/ecal_clientgate.h"

#ifndef ECAL_LAYER_ICEORYX
#include "io/ecal_memfile_pool.h"
#endif /* !ECAL_LAYER_ICEORYX */

#include <memory>

namespace eCAL
{
  class CGlobals
  {
  public:
    CGlobals();
    ~CGlobals();

    int Initialize    ( unsigned int components_, std::vector<std::string>* config_keys_ = nullptr);
    int IsInitialized ( unsigned int component_  );

    unsigned int GetComponents() { return(components); };

    int Finalize(unsigned int components_);

    const std::unique_ptr<CConfig>&                                       config()           { return config_instance; };
    const std::unique_ptr<CLog>&                                          log()              { return log_instance; };
    const std::unique_ptr<CMonitoring>&                                   monitoring()       { return monitoring_instance; };
    const std::unique_ptr<CTimeGate>&                                     timegate()         { return timegate_instance; };
    const std::unique_ptr<CSubGate>&                                      subgate()          { return subgate_instance; };
    const std::unique_ptr<CPubGate>&                                      pubgate()          { return pubgate_instance; };
    const std::unique_ptr<CServiceGate>&                                  servicegate()      { return servicegate_instance; };
    const std::unique_ptr<CClientGate>&                                   clientgate()       { return clientgate_instance; };
    const std::unique_ptr<CEntityRegister>&                               entity_register()  { return entity_register_instance; };
    const std::unique_ptr<CDescGate>&                                     descgate()         { return descgate_instance; };
    const std::unique_ptr<CRegGate>&                                      reggate()          { return reggate_instance; };
#ifndef ECAL_LAYER_ICEORYX
    const std::unique_ptr<CMemFileThreadPool>&                            memfile_pool()     { return memfile_pool_instance; };
    const std::unique_ptr<SMemFileMap>&                                   memfile_map()      { return memfile_map_instance; };
#endif /* !ECAL_LAYER_ICEORYX */

  private:
    bool                                                                  initialized;
    unsigned int                                                          components;
    std::unique_ptr<CConfig>                                              config_instance;
    std::unique_ptr<CLog>                                                 log_instance;
    std::unique_ptr<CMonitoring>                                          monitoring_instance;
    std::unique_ptr<CTimeGate>                                            timegate_instance;
    std::unique_ptr<CSubGate>                                             subgate_instance;
    std::unique_ptr<CPubGate>                                             pubgate_instance;
    std::unique_ptr<CServiceGate>                                         servicegate_instance;
    std::unique_ptr<CClientGate>                                          clientgate_instance;
    std::unique_ptr<CEntityRegister>                                      entity_register_instance;
    std::unique_ptr<CDescGate>                                            descgate_instance;
    std::unique_ptr<CRegGate>                                             reggate_instance;
#ifndef ECAL_LAYER_ICEORYX
    std::unique_ptr<CMemFileThreadPool>                                   memfile_pool_instance;
    std::unique_ptr<SMemFileMap>                                          memfile_map_instance;
#endif /* !ECAL_LAYER_ICEORYX */
  };
}
