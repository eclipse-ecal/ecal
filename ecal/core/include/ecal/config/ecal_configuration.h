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
 * @file   ecal_configuration.h
 * @brief  eCAL configuration interface
**/

#pragma once

#include <ecal/config/ecal_application_config.h>
#include <ecal/config/ecal_monitoring_config.h>
#include <ecal/config/ecal_registration_config.h>
#include <ecal/config/ecal_service_config.h>
#include <ecal/config/ecal_logging_config.h>
#include <ecal/config/ecal_transport_layer_config.h>
#include <ecal/config/user_arguments.h>
#include <ecal/config/ecal_publisher_config.h>
#include <ecal/config/ecal_subscriber_config.h>
#include <ecal/types/ecal_custom_data_types.h>


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
        Registration::Configuration      registration{};
        Monitoring::Configuration        monitoring{};
        Subscriber::Configuration        subscriber{};
        Publisher::Configuration         publisher{};
        Service::TimesyncOptions         timesync{};
        Service::Configuration           service{};
        Application::Configuration       application{};
        Logging::Configuration           logging{};
        Cli::Configuration               command_line_arguments{};        
        
        ECAL_API Configuration();
        ECAL_API Configuration(int argc_ , char **argv_);
        ECAL_API Configuration(std::vector<std::string> args_);

        ECAL_API void InitConfigWithDefaultIni();
        ECAL_API void InitConfig(std::string ini_path_ = std::string(""));

        ECAL_API std::string GetIniFilePath();

        friend class CmdParser;

        protected:
          std::string                    ecal_ini_file_path{};
          
        private:
          ECAL_API void Init(int argc_ , char **argv_);
    };
}