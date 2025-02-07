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

#pragma once

#include <core/config/py_application.h>
#include <ecal/config/application.h>

namespace nb = nanobind;

void AddConfigApplication(nanobind::module_& module)
{
  // Bind Sys::Configuration struct
  nb::class_<eCAL::Application::Sys::Configuration>(module, "SysConfiguration")
    .def(nb::init<>()) // Default constructor
    .def_rw("filter_excl", &eCAL::Application::Sys::Configuration::filter_excl,
      "Apps blacklist to be excluded when importing tasks from cloud");

  // Bind Startup::Configuration struct
  nb::class_<eCAL::Application::Startup::Configuration>(module, "StartupConfiguration")
    .def(nb::init<>()) // Default constructor
    .def_rw("terminal_emulator", &eCAL::Application::Startup::Configuration::terminal_emulator,
      "Linux-only command for starting applications with an external terminal emulator");

  // Bind Application::Configuration struct
  nb::class_<eCAL::Application::Configuration>(module, "ApplicationConfiguration")
    .def(nb::init<>()) // Default constructor
    .def_rw("sys", &eCAL::Application::Configuration::sys)
    .def_rw("startup", &eCAL::Application::Configuration::startup);
}
