/* ========================= eCAL LICENSE =================================
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
 * ========================= eCAL LICENSE =================================
*/

#pragma once

#include "logging/config/attributes/ecal_log_provider_attributes.h"
#include "logging/config/attributes/ecal_log_receiver_attributes.h"
#include "ecal/config/logging.h"
#include "ecal/config/registration.h"
#include "ecal/config/transport_layer.h"

namespace eCAL
{
  namespace Logging
  {
    SProviderAttributes BuildLoggingProviderAttributes(const Logging::Configuration& log_config_, const Registration::Configuration& reg_config_, const TransportLayer::Configuration& tl_config_);
    SReceiverAttributes BuildLoggingReceiverAttributes(const Logging::Configuration& log_config_, const Registration::Configuration& reg_config_, const TransportLayer::Configuration& tl_config_);
  }
}