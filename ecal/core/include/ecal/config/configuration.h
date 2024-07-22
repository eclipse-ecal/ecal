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

#include <ecal/config/application.h>
#include <ecal/config/monitoring.h>
#include <ecal/config/registration.h>
#include <ecal/config/service.h>
#include <ecal/config/logging.h>
#include <ecal/config/user_arguments.h>
#include <ecal/config/publisher.h>
#include <ecal/config/subscriber.h>
#include <ecal/config/time.h>
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
    Time::Configuration              timesync{};
    Service::Configuration           service{};
    Application::Configuration       application{};
    Logging::Configuration           logging{};
    Cli::Configuration               command_line_arguments{};

    ECAL_API Configuration();
    ECAL_API Configuration(int argc_, char** argv_);
    ECAL_API Configuration(std::vector<std::string>& args_);

    ECAL_API void InitConfigWithDefaultYaml();
    ECAL_API void InitConfigWithDefaults();
    ECAL_API void InitConfigFromFile(std::string yaml_path_);

    ECAL_API std::string GetYamlFilePath();

    friend class CmdParser;

  protected:
    std::string                      ecal_yaml_file_path{};

  private:
    ECAL_API void Init(std::vector<std::string>& args_);
  };
}
