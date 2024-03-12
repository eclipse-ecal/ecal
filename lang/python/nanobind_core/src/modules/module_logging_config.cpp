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

enum NB_eCAL_Logging_eLogLevel
{
    nb_log_level_none = 0,
    nb_log_level_all = 255,
    nb_log_level_info = 1,
    nb_log_level_error = 4,
    nb_log_level_fatal = 8,
    nb_log_filter_default = 15,
    nb_log_level_debug1 = 16,
    nb_log_level_debug2 = 32,
    nb_log_level_warning = 2,
    nb_log_level_debug3 = 64,
    nb_log_level_debug4 = 128,
};


void AddLoggingConfigStructToModule(nanobind::module_& m)
{
    nanobind::enum_<NB_eCAL_Logging_eLogLevel>(m, "eCAL_Logging_Filter")
        .value("log_level_none", nb_log_level_none)
        .value("log_level_all", nb_log_level_all)
        .value("log_level_info", nb_log_level_info)
        .value("log_level_warning", nb_log_level_warning)
        .value("log_level_error", nb_log_level_error)
        .value("log_level_fatal", nb_log_level_fatal)
        .value("log_filter_default", nb_log_filter_default)                //Line has to be discussed
        .value("log_level_debug1", nb_log_level_debug1)
        .value("log_level_debug2", nb_log_level_debug2)
        .value("log_level_debug3", nb_log_level_debug3)
        .value("log_level_debug4", nb_log_level_debug4);

    // Bind the Console::Configuration structure
    nanobind::class_<eCAL::Logging::Sinks::Console::Configuration>(m, "ConsoleConfiguration")
        .def(nanobind::init<>())  // Constructor binding
        .def_rw("enable", &eCAL::Logging::Sinks::Console::Configuration::enable)
        .def_rw("filter_log_con", &eCAL::Logging::Sinks::Console::Configuration::filter_log_con);

    // Bind the File::Configuration structure
    nanobind::class_<eCAL::Logging::Sinks::File::Configuration>(m, "FileConfiguration")
        .def(nanobind::init<>())  // Constructor binding
        .def_rw("enable", &eCAL::Logging::Sinks::File::Configuration::enable)
        .def_rw("path", &eCAL::Logging::Sinks::File::Configuration::path)
        .def_rw("filter_log_file", &eCAL::Logging::Sinks::File::Configuration::filter_log_file);

    // Bind the UDP::Configuration structure
    nanobind::class_<eCAL::Logging::Sinks::UDP::Configuration>(m, "UDPConfiguration")
        .def(nanobind::init<>())  // Constructor binding
        .def_rw("enable", &eCAL::Logging::Sinks::UDP::Configuration::enable)
        .def_rw("port", &eCAL::Logging::Sinks::UDP::Configuration::port)
        .def_rw("filter_log_udp", &eCAL::Logging::Sinks::UDP::Configuration::filter_log_udp);

    // Bind the Sinks::Configuration structure
    nanobind::class_<eCAL::Logging::Sinks::Configuration>(m, "SinksConfiguration")
        .def(nanobind::init<>())  // Constructor binding
        .def_rw("console", &eCAL::Logging::Sinks::Configuration::console)
        .def_rw("file", &eCAL::Logging::Sinks::Configuration::file)
        .def_rw("udp", &eCAL::Logging::Sinks::Configuration::udp);

    // Bind the Logging::Configuration structure
    nanobind::class_<eCAL::Logging::Configuration>(m, "LoggingConfiguration")
        .def(nanobind::init<>())  // Constructor binding
        .def_rw("sinks", &eCAL::Logging::Configuration::sinks);

}

