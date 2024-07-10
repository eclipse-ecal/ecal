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
 * @file   configuration.h
 * @brief  eCAL configuration interface
**/

#pragma once

#include <ecal/config_optional/application.h>
#include <ecal/config_optional/monitoring.h>
#include <ecal/config_optional/registration.h>
#include <ecal/config_optional/service.h>
#include <ecal/config_optional/logging.h>
#include <ecal/config_optional/transport_layer.h>
#include <ecal/config_optional/publisher.h>
#include <ecal/config_optional/subscriber.h>
#include <ecal/config_optional/time.h>
#include <ecal/config/user_arguments.h>
#include <ecal/types/ecal_custom_data_types.h>

#include "ecal/ecal_os.h"
#include "ecal/ecal_log_level.h"

#include <string>
#include <vector>
#include <iostream>
#include <chrono>
#include <optional>


namespace eCAL
{
    struct ConfigurationSegment
    {
        std::optional<TransportLayerOptional::Configuration>    transport_layer{};
        std::optional<RegistrationOptional::Configuration>      registration{};
        std::optional<MonitoringOptional::Configuration>        monitoring{};
        std::optional<SubscriberOptional::Configuration>        subscriber{};
        std::optional<PublisherOptional::Configuration>         publisher{};
        std::optional<TimeOptional::Configuration>              timesync{};
        std::optional<ServiceOptional::Configuration>           service{};
        std::optional<ApplicationOptional::Configuration>       application{};
        std::optional<LoggingOptional::Configuration>           logging{};
        
        ECAL_API ConfigurationSegment();

        ECAL_API void InitConfigWithDefaultIni();
        ECAL_API void InitConfig(std::string ini_path_ = std::string(""));

        ECAL_API std::string GetIniFilePath();

        protected:
          std::string                    ecal_ini_file_path{};
          
        private:
          ECAL_API void Init();
    };
}