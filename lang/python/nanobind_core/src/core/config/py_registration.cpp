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

#include <core/config/py_registration.h>
#include <ecal/config/registration.h>

namespace nb = nanobind;
using namespace eCAL::Registration;

void AddConfigRegistration(nanobind::module_& module)
{
  // Bind Registration::Layer::SHM::Configuration struct
  nb::class_<Layer::SHM::Configuration>(module, "RegistrationLayerSHMConfiguration")
    .def(nb::init<>()) // Default constructor
    .def_rw("enable", &Layer::SHM::Configuration::enable, "Enable shared memory-based registration")
    .def_rw("domain", &Layer::SHM::Configuration::domain, "Shared memory registration domain name")
    .def_rw("queue_size", &Layer::SHM::Configuration::queue_size, "Queue size of registration events");

  // Bind Registration::Layer::UDP::Configuration struct
  nb::class_<Layer::UDP::Configuration>(module, "RegistrationLayerUDPConfiguration")
    .def(nb::init<>()) // Default constructor
    .def_rw("enable", &Layer::UDP::Configuration::enable, "Enable UDP-based registration")
    .def_rw("port", &Layer::UDP::Configuration::port, "UDP multicast port number");

  // Bind Registration::Layer::Configuration struct
  nb::class_<Layer::Configuration>(module, "RegistrationLayerConfiguration")
    .def(nb::init<>()) // Default constructor
    .def_rw("shm", &Layer::Configuration::shm, "Shared memory registration configuration")
    .def_rw("udp", &Layer::Configuration::udp, "UDP-based registration configuration");

  // Bind Registration::Configuration struct
  nb::class_<Configuration>(module, "RegistrationConfiguration")
    .def(nb::init<>()) // Default constructor
    .def_rw("registration_timeout_ms", &Configuration::registration_timeout,
      "Timeout for topic registration in milliseconds")
    .def_rw("registration_refresh_ms", &Configuration::registration_refresh,
      "Topic registration refresh cycle (must be smaller than timeout)")
    .def_rw("network_enabled", &Configuration::network_enabled,
      "Enable/disable network communication (false = local communication only)")
    .def_rw("loopback", &Configuration::loopback,
      "Enable receiving UDP messages on the same local machine")
    .def_rw("shm_transport_domain", &Configuration::shm_transport_domain,
      "Shared memory transport domain (enables interprocess mechanisms across virtual host borders)")
    .def_rw("layer", &Configuration::layer, "Layer configuration for registration mechanisms");
}