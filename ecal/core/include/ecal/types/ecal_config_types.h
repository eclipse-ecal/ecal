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
#include "ecal_registration_options.h"
#include "ecal_service_options.h"
#include "ecal_logging_options.h"
#include "ecal_transport_layer_options.h"
#include "user_arg_options.h"
#include "ecal/config/ecal_publisher_config.h"
#include "ecal/config/ecal_subscriber_config.h"

#include "ecal/ecal_os.h"
#include "ecal/ecal_log_level.h"

#include <string>
#include <vector>
#include <iostream>
#include <chrono>

namespace eCAL
{
    struct Configuration
    {
        TransportLayer::Configuration    transport_layer{};
        Config::RegistrationOptions      registration{};
        Config::MonitoringOptions        monitoring{};
        Subscriber::Configuration        subscriber{};
        Publisher::Configuration         publisher{};
        Config::TimesyncOptions          timesync{};
        Config::ServiceOptions           service{};
        Config::ApplicationOptions       application{};
        Config::LoggingOptions           logging{};
        Cli::Configuration               command_line_arguments{};
        std::string                      ecal_ini_file_path{};        
        
        ECAL_API Configuration(int argc_ , char **argv_);
        ECAL_API Configuration(std::vector<std::string> args_);

        ECAL_API void InitConfigWithDefaultIni();
        ECAL_API void InitConfig(std::string ini_path_);

        private:
          ECAL_API void Init(int argc_ , char **argv_);
    };
}