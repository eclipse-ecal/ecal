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
 * @brief  Add transport_layer config structs to nanobind module
**/


#include <modules/module_transport_layer_config.h>
#include <ecal/config/transport_layer.h>

namespace nb = nanobind;

void AddTransportLayerConfigStructToModule(nanobind::module_& m)
{
    nb::module_ m_eCAL = m.def_submodule("eCAL");
    nb::module_ m_TransportLayer = m_eCAL.def_submodule("TransportLayer");
    nb::module_ m_UDP = m_TransportLayer.def_submodule("UDP");
    nb::module_ m_TCP = m_TransportLayer.def_submodule("TCP");

    // UDP Network Configuration Binding
    nb::class_<eCAL::TransportLayer::UDP::Network::Configuration>(m_UDP, "NetworkConfiguration")
        .def(nb::init<>())
        .def_rw("group", &eCAL::TransportLayer::UDP::Network::Configuration::group)
        .def_rw("ttl", &eCAL::TransportLayer::UDP::Network::Configuration::ttl);

    // UDP Local Configuration Binding
    nb::class_<eCAL::TransportLayer::UDP::Local::Configuration>(m_UDP, "LocalConfiguration")
        .def(nb::init<>())
#if 0
        // Property for 'group' with automatic string conversion
        .def_prop_rw(
            "group",
            [](const eCAL::TransportLayer::UDP::Local::Configuration& self) -> std::string {
                return self.group.Get();  // Get the IP as a string
            },
            [](eCAL::TransportLayer::UDP::Local::Configuration& self, const std::string& value) {
                self.group = value;  // Directly assign a string to 'group' using overloaded assignment
            },
            "UDP multicast group base as a string"
        )
#endif
        .def_rw("group", &eCAL::TransportLayer::UDP::Local::Configuration::group)
        .def_rw("ttl", &eCAL::TransportLayer::UDP::Local::Configuration::ttl);

    // UDP Configuration Binding
    nb::class_<eCAL::TransportLayer::UDP::Configuration>(m_UDP, "UDPConfiguration")
        .def(nb::init<>())
        .def_rw("config_version", &eCAL::TransportLayer::UDP::Configuration::config_version)
        .def_rw("port", &eCAL::TransportLayer::UDP::Configuration::port)
        .def_rw("mode", &eCAL::TransportLayer::UDP::Configuration::mode)
        .def_rw("mask", &eCAL::TransportLayer::UDP::Configuration::mask)
        .def_rw("send_buffer", &eCAL::TransportLayer::UDP::Configuration::send_buffer)
        .def_rw("receive_buffer", &eCAL::TransportLayer::UDP::Configuration::receive_buffer)
        .def_rw("join_all_interfaces", &eCAL::TransportLayer::UDP::Configuration::join_all_interfaces)
        .def_rw("npcap_enabled", &eCAL::TransportLayer::UDP::Configuration::npcap_enabled)
        .def_rw("network", &eCAL::TransportLayer::UDP::Configuration::network)
        .def_ro("local", &eCAL::TransportLayer::UDP::Configuration::local);

    // TCP Configuration Binding
    nb::class_<eCAL::TransportLayer::TCP::Configuration>(m_TCP, "TCPConfiguration")
        .def(nb::init<>())
        .def_rw("number_executor_reader", &eCAL::TransportLayer::TCP::Configuration::number_executor_reader)
        .def_rw("number_executor_writer", &eCAL::TransportLayer::TCP::Configuration::number_executor_writer)
        .def_rw("max_reconnections", &eCAL::TransportLayer::TCP::Configuration::max_reconnections);

    // Overall Transport Layer Configuration Binding
    nb::class_<eCAL::TransportLayer::Configuration>(m_TransportLayer, "TransportLayerConfiguration")
        .def(nb::init<>())
        .def_rw("udp", &eCAL::TransportLayer::Configuration::udp)
        .def_rw("tcp", &eCAL::TransportLayer::Configuration::tcp);

}

