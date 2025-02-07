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

#include <core/config/py_logging.h>
#include <ecal/config/logging.h>

namespace nb = nanobind;

void AddConfigLogging(nanobind::module_& module)
{
  // Bind eCAL::Logging::Provider::Sink struct
  nb::class_<eCAL::Logging::Provider::Sink>(module, "LoggingProviderSink")
    .def(nb::init<>()) // Default constructor
    .def_rw("enable", &eCAL::Logging::Provider::Sink::enable, "Enable sink")
    .def_rw("filter_log", &eCAL::Logging::Provider::Sink::filter_log, "Log message filter level");

  // Bind eCAL::Logging::Provider::File::Configuration struct
  nb::class_<eCAL::Logging::Provider::File::Configuration>(module, "LoggingProviderFileConfiguration")
    .def(nb::init<>()) // Default constructor
    .def_rw("path", &eCAL::Logging::Provider::File::Configuration::path, "Path to log file");

  // Bind eCAL::Logging::Provider::UDP::Configuration struct
  nb::class_<eCAL::Logging::Provider::UDP::Configuration>(module, "LoggingProviderUDPConfiguration")
    .def(nb::init<>()) // Default constructor
    .def_rw("port", &eCAL::Logging::Provider::UDP::Configuration::port, "UDP port number");

  // Bind eCAL::Logging::Provider::Configuration struct
  nb::class_<eCAL::Logging::Provider::Configuration>(module, "LoggingProviderConfiguration")
    .def(nb::init<>()) // Default constructor
    .def_rw("console", &eCAL::Logging::Provider::Configuration::console, "Console sink settings")
    .def_rw("file", &eCAL::Logging::Provider::Configuration::file, "File sink settings")
    .def_rw("udp", &eCAL::Logging::Provider::Configuration::udp, "UDP sink settings")
    .def_rw("file_config", &eCAL::Logging::Provider::Configuration::file_config, "File sink configuration")
    .def_rw("udp_config", &eCAL::Logging::Provider::Configuration::udp_config, "UDP sink configuration");

  // Bind eCAL::Logging::Receiver::UDP::Configuration struct
  nb::class_<eCAL::Logging::Receiver::UDP::Configuration>(module, "LoggingReceiverUDPConfiguration")
    .def(nb::init<>()) // Default constructor
    .def_rw("port", &eCAL::Logging::Receiver::UDP::Configuration::port, "UDP port number");

  // Bind Logging::Receiver::Configuration struct
  nb::class_<eCAL::Logging::Receiver::Configuration>(module, "LoggingReceiverConfiguration")
    .def(nb::init<>()) // Default constructor
    .def_rw("enable", &eCAL::Logging::Receiver::Configuration::enable, "Enable UDP receiver")
    .def_rw("udp_config", &eCAL::Logging::Receiver::Configuration::udp_config, "UDP receiver configuration");

  // Bind eCAL::Logging::Configuration struct
  nb::class_<eCAL::Logging::Configuration>(module, "LoggingConfiguration")
    .def(nb::init<>()) // Default constructor
    .def_rw("provider", &eCAL::Logging::Configuration::provider, "Logging provider configuration")
    .def_rw("receiver", &eCAL::Logging::Configuration::receiver, "Logging receiver configuration");
}
