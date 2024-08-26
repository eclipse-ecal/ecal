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

#include "logging/attributes/logging_attributes.h"
#include "ecal/config/logging.h"

namespace eCAL
{
  namespace Logging
  {
    SAttributes BuildLoggingAttributes(const Logging::Configuration& config_)
    {
      SAttributes attributes;

      attributes.udp.enabled        = config_.sinks.udp.enable;
      attributes.udp.port           = config_.sinks.udp.port;
      attributes.udp.filter_log     = config_.sinks.udp.filter_log_udp;

      attributes.file.enabled       = config_.sinks.file.enable;
      attributes.file.path          = config_.sinks.file.path;
      attributes.file.filter_log    = config_.sinks.file.filter_log_file;

      attributes.console.enabled    = config_.sinks.console.enable;
      attributes.console.filter_log = config_.sinks.console.filter_log_con;

      return attributes;
    }
  }
}