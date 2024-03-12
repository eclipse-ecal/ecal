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
 * @brief  Add Registration config structs to nanobind module
**/


#include <modules/module_registration_config.h>
#include <ecal/config/registration.h>

namespace nb = nanobind;

void AddRegistrationConfigStructToModule(nanobind::module_& m)
{
    nb::module_ m_Registration = m.def_submodule("Registration");
    nb::module_ m_Layer = m_Registration.def_submodule("Layer");
    nb::module_ m_SHM = m_Layer.def_submodule("SHM");
    nb::module_ m_UDP = m_Layer.def_submodule("UDP");

    // Binding SHM Configuration
    nb::class_<eCAL::Registration::Layer::SHM::Configuration>(m_SHM, "SHMConfiguration")
        .def(nb::init<>())  // Default constructor
        .def_rw("enable", &eCAL::Registration::Layer::SHM::Configuration::enable)
        .def_rw("domain", &eCAL::Registration::Layer::SHM::Configuration::domain)
        .def_rw("queue_size", &eCAL::Registration::Layer::SHM::Configuration::queue_size);

    // Binding UDP Configuration
    nb::class_<eCAL::Registration::Layer::UDP::Configuration>(m_UDP, "UDPConfiguration")
        .def(nb::init<>())  // Default constructor
        .def_rw("enable", &eCAL::Registration::Layer::UDP::Configuration::enable)
        .def_rw("port", &eCAL::Registration::Layer::UDP::Configuration::port);

    // Binding Layer Configuration
    nb::class_<eCAL::Registration::Layer::Configuration>(m_Layer, "LayerConfiguration")
        .def(nb::init<>())  // Default constructor
        .def_rw("shm", &eCAL::Registration::Layer::Configuration::shm)
        .def_rw("udp", &eCAL::Registration::Layer::Configuration::udp);

    // Binding Registration Configuration
    nb::class_<eCAL::Registration::Configuration>(m_Registration, "RegistrationConfiguration")
        .def(nb::init<>())  // Default constructor
        .def_rw("registration_timeout", &eCAL::Registration::Configuration::registration_timeout)
        .def_rw("registration_refresh", &eCAL::Registration::Configuration::registration_refresh)
        .def_rw("network_enabled", &eCAL::Registration::Configuration::network_enabled)
        .def_rw("loopback", &eCAL::Registration::Configuration::loopback)
        .def_rw("host_group_name", &eCAL::Registration::Configuration::host_group_name)
        .def_rw("layer", &eCAL::Registration::Configuration::layer);
}

