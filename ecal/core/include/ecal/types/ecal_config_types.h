/* =========================== LICENSE =================================
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
 * =========================== LICENSE =================================
 */

/**
 * @file   ecal_config_types.h
 * @brief  eCAL config interface using structs
**/

#pragma once

#include "ecal_application_options.h"
#include "ecal_custom_data_types.h"
#include "ecal_monitoring_options.h"
#include "ecal_publisher_options.h"
#include "ecal_receiving_options.h"
#include "ecal_registration_options.h"
#include "ecal_service_options.h"
#include "ecal_internal_options.h"
#include "ecal_transport_layer_options.h"

#include "ecal/ecal_os.h"
#include "ecal/ecal_log_level.h"

#include <string>
#include <vector>
#include <iostream>
#include <chrono>
#include <limits>

namespace eCAL
{
  namespace Config
  {        
    struct eCALConfig
    {
        TransportLayerOptions transport_layer_options;
        RegistrationOptions   registration_options;
        MonitoringOptions     monitoring_options;
        ReceivingOptions      receiving_options;
        PublisherOptions      publisher_options;
        TimesyncOptions       timesync_options;
        ServiceOptions        service_options;
        ApplicationOptions    application_options;
        std::string           loaded_ecal_ini_file;

        eCALConfig();
    };

    ECAL_API eCALConfig* GetCurrentConfig();
	}
}