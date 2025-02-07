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
 * @file   py_init.h
 * @brief  eCAL core function interface
**/

#pragma once

#include <core/config/py_configuration.h>
#include <ecal/config/configuration.h>

namespace nb = nanobind;

void AddConfigConfiguration(nanobind::module_& module)
{
  // Bind Configuration struct
  nb::class_<eCAL::Configuration>(module, "Configuration")
    .def(nb::init<>()) // Default constructor
    // Not sure if we want to expose the following functions to python
    //.def("init_from_config", &eCAL::Configuration::InitFromConfig, "Initialize configuration from eCAL settings")
    //.def("init_from_file", &eCAL::Configuration::InitFromFile, nb::arg("yaml_path"), "Initialize configuration from YAML file")
    .def("get_configuration_file_path", &eCAL::Configuration::GetConfigurationFilePath, "Get the configuration file path")
    .def_rw("transport_layer", &eCAL::Configuration::transport_layer)
    .def_rw("registration", &eCAL::Configuration::registration)
    .def_rw("subscriber", &eCAL::Configuration::subscriber)
    .def_rw("publisher", &eCAL::Configuration::publisher)
    .def_rw("timesync", &eCAL::Configuration::timesync)
    .def_rw("application", &eCAL::Configuration::application)
    .def_rw("logging", &eCAL::Configuration::logging);
}