/* =========================== LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
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
 * @file   config/configuration.h
 * @brief  eCAL configuration interface
**/

#pragma once

#include <ecal/config/application.h>
#include <ecal/config/registration.h>
#include <ecal/config/logging.h>
#include <ecal/config/publisher.h>
#include <ecal/config/subscriber.h>
#include <ecal/config/time.h>
#include <ecal/types/custom_data_types.h>


#include "ecal/os.h"
#include "ecal/log_level.h"

#include <string>
#include <vector>
#include <iostream>
#include <chrono>

namespace eCAL
{
  struct Configuration
  {
    TransportLayer::Configuration transport_layer;
    Registration::Configuration   registration;
    Subscriber::Configuration     subscriber;
    Publisher::Configuration      publisher;
    Time::Configuration           timesync;
    Application::Configuration    application;
    Logging::Configuration        logging;

    ECAL_API Configuration();

    ECAL_API void InitFromConfig();
    ECAL_API void InitFromFile(const std::string& yaml_path_);

    ECAL_API std::string GetConfigurationFilePath() const;

    protected:
      std::string ecal_yaml_file_path;
  };
}
