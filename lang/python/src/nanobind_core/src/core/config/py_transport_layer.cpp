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

#include <core/config/py_transport_layer.h>
#include <ecal/config/transport_layer.h>

namespace nb = nanobind;
using namespace eCAL::TransportLayer;


void AddConfigTransportLayer(nanobind::module_& module)
{
  // Bind TransportLayer::eType enum
  nb::enum_<eType>(module, "TransportType")
    .value("NONE", eType::none)
    .value("UDP_MC", eType::udp_mc)
    .value("SHM", eType::shm)
    .value("TCP", eType::tcp);

  // Bind TransportLayer::UDP::MulticastConfiguration struct
  nb::class_<UDP::MulticastConfiguration>(module, "MulticastConfiguration")
    .def(nb::init<>()) // Default constructor
    .def_rw("group", &UDP::MulticastConfiguration::group, "UDP multicast group base")
    .def_rw("ttl", &UDP::MulticastConfiguration::ttl, "Time-to-live (TTL) for UDP packets");

  // Bind TransportLayer::UDP::Configuration struct
  nb::class_<UDP::Configuration>(module, "UDPConfiguration")
    .def(nb::init<>()) // Default constructor
    .def_rw("config_version", &UDP::Configuration::config_version, "UDP configuration version (Default: v2)")
    .def_rw("port", &UDP::Configuration::port, "UDP multicast port number (Default: 14002)")
    .def_rw("mask", &UDP::Configuration::mask, "Multicast mask for group routing")
    .def_rw("send_buffer", &UDP::Configuration::send_buffer, "UDP send buffer size in bytes")
    .def_rw("receive_buffer", &UDP::Configuration::receive_buffer, "UDP receive buffer size in bytes")
    .def_rw("join_all_interfaces", &UDP::Configuration::join_all_interfaces,
      "Enable joining multicast groups on all network interfaces (Linux-specific)")
    .def_rw("npcap_enabled", &UDP::Configuration::npcap_enabled,
      "Enable UDP traffic reception with Npcap-based receiver")
    .def_rw("network", &UDP::Configuration::network, "Network multicast configuration")
    .def_rw("local", &UDP::Configuration::local, "Local multicast configuration");

  // Bind TransportLayer::TCP::Configuration struct
  nb::class_<TCP::Configuration>(module, "TCPConfiguration")
    .def(nb::init<>()) // Default constructor
    .def_rw("number_executor_reader", &TCP::Configuration::number_executor_reader,
      "Number of reader threads for TCP execution")
    .def_rw("number_executor_writer", &TCP::Configuration::number_executor_writer,
      "Number of writer threads for TCP execution")
    .def_rw("max_reconnections", &TCP::Configuration::max_reconnections,
      "Maximum number of reconnection attempts (Default: 5)");

  // Bind TransportLayer::Configuration struct
  nb::class_<Configuration>(module, "TransportLayerConfiguration")
    .def(nb::init<>()) // Default constructor
    .def_rw("udp", &Configuration::udp, "UDP transport layer configuration")
    .def_rw("tcp", &Configuration::tcp, "TCP transport layer configuration");
}