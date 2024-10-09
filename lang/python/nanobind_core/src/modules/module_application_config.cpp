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


#include <modules/module_application_config.h>
#include <wrappers/wrapper_application_config.h>

// Import the required Nanobind namespace
namespace nb = nanobind;

void AddApplicationConfigStructToModule(nanobind::module_& m)
{   
    // Bind the Sys::Configuration structure
    nb::class_<eCAL::Application::Sys::CNBSysConfiguration>(m, "SysConfiguration")
        .def(nb::init<>())  // Constructor binding
        .def_rw("filter_excl", &eCAL::Application::Sys::CNBSysConfiguration::filter_excl);  // Field binding

    // Bind the Startup::Configuration structure
    nb::class_<eCAL::Application::Startup::CNBStartupConfiguration>(m, "StartupConfiguration")
        .def(nb::init<>())  // Constructor binding
        .def_rw("terminal_emulator", &eCAL::Application::Startup::CNBStartupConfiguration::terminal_emulator);  // Field binding

    // Bind the main Configuration structure
    nb::class_<eCAL::Application::CNBApplicationConfiguration>(m, "Configuration")
        .def(nb::init<>())  // Constructor binding
        .def_rw("sys", &eCAL::Application::CNBApplicationConfiguration::sys)  // Field binding
        .def_rw("startup", &eCAL::Application::CNBApplicationConfiguration::startup);  // Field binding
}

