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

void AddConfigRegistration(nanobind::module_& module)
{
  // Enums
  nb::enum_<eCAL::Registration::Local::eTransportType>(module, "LocalTransportType")
    .value("SHM", eCAL::Registration::Local::eTransportType::shm)
    .value("UDP", eCAL::Registration::Local::eTransportType::udp);

  nb::enum_<eCAL::Registration::Network::eTransportType>(module, "NetworkTransportType")
    .value("UDP", eCAL::Registration::Network::eTransportType::udp);

  // Local::SHM::Configuration
  nb::class_<eCAL::Registration::Local::SHM::Configuration>(module, "LocalSHMConfig")
    .def(nb::init<>())
    .def_rw("domain", &eCAL::Registration::Local::SHM::Configuration::domain)
    .def_rw("queue_size", &eCAL::Registration::Local::SHM::Configuration::queue_size);

  // Local::UDP::Configuration
  nb::class_<eCAL::Registration::Local::UDP::Configuration>(module, "LocalUDPConfig")
    .def(nb::init<>())
    .def_rw("port", &eCAL::Registration::Local::UDP::Configuration::port);

  // Local::Configuration
  nb::class_<eCAL::Registration::Local::Configuration>(module, "LocalConfig")
    .def(nb::init<>())
    .def_rw("transport_type", &eCAL::Registration::Local::Configuration::transport_type)
    .def_rw("shm", &eCAL::Registration::Local::Configuration::shm)
    .def_rw("udp", &eCAL::Registration::Local::Configuration::udp);

  // Network::UDP::Configuration
  nb::class_<eCAL::Registration::Network::UDP::Configuration>(module, "NetworkUDPConfig")
    .def(nb::init<>())
    .def_rw("port", &eCAL::Registration::Network::UDP::Configuration::port);

  // Network::Configuration
  nb::class_<eCAL::Registration::Network::Configuration>(module, "NetworkConfig")
    .def(nb::init<>())
    .def_rw("transport_type", &eCAL::Registration::Network::Configuration::transport_type)
    .def_rw("udp", &eCAL::Registration::Network::Configuration::udp);

  // Root Configuration
  nb::class_<eCAL::Registration::Configuration>(module, "RegistrationConfig")
    .def(nb::init<>())
    .def_rw("registration_timeout", &eCAL::Registration::Configuration::registration_timeout)
    .def_rw("registration_refresh", &eCAL::Registration::Configuration::registration_refresh)
    .def_rw("loopback", &eCAL::Registration::Configuration::loopback)
    .def_rw("shm_transport_domain", &eCAL::Registration::Configuration::shm_transport_domain)
    .def_rw("local", &eCAL::Registration::Configuration::local)
    .def_rw("network", &eCAL::Registration::Configuration::network);
}