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

/**
 * @brief  Add Publisher config structs to nanobind module
**/


#include <modules/module_logging_config.h>
#include <wrappers/wrapper_logging_config.h>

void AddLoggingConfigStructToModule(const nanobind::module_& module)
{
    // Bind the Console::Configuration structure
    nanobind::class_<eCAL::Logging::Sinks::Console::CNBConsoleConfiguration>(m, "ConsoleConfiguration")
        .def(nanobind::init<>())  // Constructor binding
        .def_rw("enable", &eCAL::Logging::Sinks::Console::CNBConsoleConfiguration::enable)
        .def_rw("filter_log_con", &eCAL::Logging::Sinks::Console::CNBConsoleConfiguration::filter_log_con);

    // Bind the File::Configuration structure
    nanobind::class_<eCAL::Logging::Sinks::File::CNBFileConfiguration>(m, "FileConfiguration")
        .def(nanobind::init<>())  // Constructor binding
        .def_rw("enable", &eCAL::Logging::Sinks::File::CNBFileConfiguration::enable)
        .def_rw("path", &eCAL::Logging::Sinks::File::CNBFileConfiguration::path)
        .def_rw("filter_log_file", &eCAL::Logging::Sinks::File::CNBFileConfiguration::filter_log_file);

    // Bind the UDP::Configuration structure
    nanobind::class_<eCAL::Logging::Sinks::UDP::CNBUDPConfiguration>(m, "UDPConfiguration")
        .def(nanobind::init<>())  // Constructor binding
        .def_rw("enable", &eCAL::Logging::Sinks::UDP::CNBUDPConfiguration::enable)
        .def_rw("port", &eCAL::Logging::Sinks::UDP::CNBUDPConfiguration::port)
        .def_rw("filter_log_udp", &eCAL::Logging::Sinks::UDP::CNBUDPConfiguration::filter_log_udp);

    // Bind the Sinks::Configuration structure
    nanobind::class_<eCAL::Logging::Sinks::NBSinksConfiguration>(m, "SinksConfiguration")
        .def(nanobind::init<>())  // Constructor binding
        .def_rw("console", &eCAL::Logging::Sinks::NBSinksConfiguration::console)
        .def_rw("file", &eCAL::Logging::Sinks::NBSinksConfiguration::file)
        .def_rw("udp", &eCAL::Logging::Sinks::NBSinksConfiguration::udp);

    // Bind the Logging::Configuration structure
    nanobind::class_<eCAL::Logging::CNBLoggingConfiguration>(m, "LoggingConfiguration")
        .def(nanobind::init<>())  // Constructor binding
        .def_rw("sinks", &eCAL::Logging::CNBLoggingConfiguration::sinks);

}

