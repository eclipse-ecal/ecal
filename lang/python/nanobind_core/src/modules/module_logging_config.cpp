/* ========================= eCAL LICENSE =================================
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
 * ========================= eCAL LICENSE =================================
*/

/**
 * @brief  Add Logging config structs to nanobind module
**/


#include <modules/module_logging_config.h>
#include <ecal/config/logging.h>

namespace nb = nanobind;

void AddLoggingConfigStructToModule(nanobind::module_& m)
{
    // If available, export logging level constants.
    m.attr("log_level_warning") = eCAL::Logging::log_level_warning;
    m.attr("log_level_error") = eCAL::Logging::log_level_error;
    m.attr("log_level_fatal") = eCAL::Logging::log_level_fatal;
    m.attr("log_level_none") = eCAL::Logging::log_level_none;
    m.attr("log_level_info") = eCAL::Logging::log_level_info;

    // ---------------------------------------------------------------------------
    // Wrap eCAL::Logging::Provider::Sink
    // ---------------------------------------------------------------------------
    nb::class_<eCAL::Logging::Provider::Sink>(m, "ProviderSink")
      .def(nb::init<>())  // default constructor
      .def_rw("enable", &eCAL::Logging::Provider::Sink::enable)
      .def_rw("filter_log", &eCAL::Logging::Provider::Sink::filter_log);

    // ---------------------------------------------------------------------------
    // Wrap eCAL::Logging::Provider::File::Configuration
    // ---------------------------------------------------------------------------
    nb::class_<eCAL::Logging::Provider::File::Configuration>(m, "ProviderFileConfiguration")
      .def(nb::init<>())
      .def_rw("path", &eCAL::Logging::Provider::File::Configuration::path);

    // ---------------------------------------------------------------------------
    // Wrap eCAL::Logging::Provider::UDP::Configuration
    // ---------------------------------------------------------------------------
    nb::class_<eCAL::Logging::Provider::UDP::Configuration>(m, "ProviderUDPConfiguration")
      .def(nb::init<>())
      .def_rw("port", &eCAL::Logging::Provider::UDP::Configuration::port);

    // ---------------------------------------------------------------------------
    // Wrap eCAL::Logging::Provider::Configuration
    // ---------------------------------------------------------------------------
    nb::class_<eCAL::Logging::Provider::Configuration>(m, "ProviderConfiguration")
      .def(nb::init<>())
      .def_rw("console", &eCAL::Logging::Provider::Configuration::console)
      .def_rw("file", &eCAL::Logging::Provider::Configuration::file)
      .def_rw("udp", &eCAL::Logging::Provider::Configuration::udp)
      .def_rw("file_config", &eCAL::Logging::Provider::Configuration::file_config)
      .def_rw("udp_config", &eCAL::Logging::Provider::Configuration::udp_config);

    // ---------------------------------------------------------------------------
    // Wrap eCAL::Logging::Receiver::UDP::Configuration
    // ---------------------------------------------------------------------------
    nb::class_<eCAL::Logging::Receiver::UDP::Configuration>(m, "ReceiverUDPConfiguration")
      .def(nb::init<>())
      .def_rw("port", &eCAL::Logging::Receiver::UDP::Configuration::port);

    // ---------------------------------------------------------------------------
    // Wrap eCAL::Logging::Receiver::Configuration
    // ---------------------------------------------------------------------------
    nb::class_<eCAL::Logging::Receiver::Configuration>(m, "ReceiverConfiguration")
      .def(nb::init<>())
      .def_rw("enable", &eCAL::Logging::Receiver::Configuration::enable)
      .def_rw("udp_config", &eCAL::Logging::Receiver::Configuration::udp_config);

    // ---------------------------------------------------------------------------
    // Wrap eCAL::Logging::Configuration (top–level)
    // ---------------------------------------------------------------------------
    nb::class_<eCAL::Logging::Configuration>(m, "LoggingConfiguration")
      .def(nb::init<>())
      .def_rw("provider", &eCAL::Logging::Configuration::provider)
      .def_rw("receiver", &eCAL::Logging::Configuration::receiver);
}

