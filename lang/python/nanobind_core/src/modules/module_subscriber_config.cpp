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
 * @brief  Add Subscriber class to nanobind module
**/


#include <modules/module_subscriber_config.h>
#include <ecal/config/subscriber.h>

namespace nb = nanobind;

void AddSubscriberConfigStructToModule(nanobind::module_& module)
{
    nb::module_ m_Subscriber = module.def_submodule("subscriber");
    nb::module_ m_Layer = m_Subscriber.def_submodule("layer");

    // Binding SHM Layer Configuration
    nb::class_<eCAL::Subscriber::Layer::SHM::Configuration>(m_Layer, "SHM_Configuration")
        .def(nb::init<>())
        .def_rw("enable", &eCAL::Subscriber::Layer::SHM::Configuration::enable);

    // Binding UDP Layer Configuration
    nb::class_<eCAL::Subscriber::Layer::UDP::Configuration>(m_Layer, "UDP_Configuration")
        .def(nb::init<>())
        .def_rw("enable", &eCAL::Subscriber::Layer::UDP::Configuration::enable);

    // Binding TCP Layer Configuration
    nb::class_<eCAL::Subscriber::Layer::TCP::Configuration>(m_Layer, "TCP_Configuration")
        .def(nb::init<>())
        .def_rw("enable", &eCAL::Subscriber::Layer::TCP::Configuration::enable);

    // Binding Layer Configuration
    nb::class_<eCAL::Subscriber::Layer::Configuration>(m_Layer, "LayerConfiguration")
        .def(nb::init<>())
        .def_rw("shm", &eCAL::Subscriber::Layer::Configuration::shm)
        .def_rw("udp", &eCAL::Subscriber::Layer::Configuration::udp)
        .def_rw("tcp", &eCAL::Subscriber::Layer::Configuration::tcp);

    // Binding Subscriber Configuration
    nb::class_<eCAL::Subscriber::Configuration>(m_Subscriber, "SubscriberConfiguration")
        .def(nb::init<>())
        .def_rw("layer", &eCAL::Subscriber::Configuration::layer)
        .def_rw("drop_out_of_order_messages", &eCAL::Subscriber::Configuration::drop_out_of_order_messages);

}
