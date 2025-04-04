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

#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>
#include <nanobind/stl/map.h>
#include <nanobind/stl/optional.h>

#include <ecal/types/monitoring.h>

namespace nb = nanobind;
using namespace nb::literals;

using namespace eCAL::Monitoring;

void AddTypesMonitoring(nanobind::module_& m)
{


  // Entity mask constants
  nb::module_ entity_mod = m.def_submodule("Entity");
  entity_mod.attr("Publisher") = Entity::Publisher;
  entity_mod.attr("Subscriber") = Entity::Subscriber;
  entity_mod.attr("Server") = Entity::Server;
  entity_mod.attr("Client") = Entity::Client;
  entity_mod.attr("Process") = Entity::Process;
  entity_mod.attr("Host") = Entity::Host;
  entity_mod.attr("All") = Entity::All;
  entity_mod.attr("None") = Entity::None;

  // Enums
  nb::enum_<eTransportLayerType>(m, "TransportLayerType")
    .value("none", eTransportLayerType::none)
    .value("udp_mc", eTransportLayerType::udp_mc)
    .value("shm", eTransportLayerType::shm)
    .value("tcp", eTransportLayerType::tcp)
    .export_values();

  // Transport Layer
  nb::class_<STransportLayer>(m, "TransportLayer")
    .def(nb::init<>())
    .def_rw("type", &STransportLayer::type)
    .def_rw("version", &STransportLayer::version)
    .def_rw("active", &STransportLayer::active);

  // STopic
  nb::class_<STopic>(m, "Topic")
    .def(nb::init<>())
    .def_rw("registration_clock", &STopic::registration_clock)
    .def_rw("host_name", &STopic::host_name)
    .def_rw("shm_transport_domain", &STopic::shm_transport_domain)
    .def_rw("process_id", &STopic::process_id)
    .def_rw("process_name", &STopic::process_name)
    .def_rw("unit_name", &STopic::unit_name)
    .def_rw("topic_id", &STopic::topic_id)
    .def_rw("topic_name", &STopic::topic_name)
    .def_rw("direction", &STopic::direction)
    .def_rw("datatype_information", &STopic::datatype_information)
    .def_rw("transport_layer", &STopic::transport_layer)
    .def_rw("topic_size", &STopic::topic_size)
    .def_rw("connections_local", &STopic::connections_local)
    .def_rw("connections_external", &STopic::connections_external)
    .def_rw("message_drops", &STopic::message_drops)
    .def_rw("data_id", &STopic::data_id)
    .def_rw("data_clock", &STopic::data_clock)
    .def_rw("data_frequency", &STopic::data_frequency);

  // SProcess
  nb::class_<SProcess>(m, "Process")
    .def(nb::init<>())
    .def_rw("registration_clock", &SProcess::registration_clock)
    .def_rw("host_name", &SProcess::host_name)
    .def_rw("shm_transport_domain", &SProcess::shm_transport_domain)
    .def_rw("process_id", &SProcess::process_id)
    .def_rw("process_name", &SProcess::process_name)
    .def_rw("unit_name", &SProcess::unit_name)
    .def_rw("process_parameter", &SProcess::process_parameter)
    .def_rw("state_severity", &SProcess::state_severity)
    .def_rw("state_severity_level", &SProcess::state_severity_level)
    .def_rw("state_info", &SProcess::state_info)
    .def_rw("time_sync_state", &SProcess::time_sync_state)
    .def_rw("time_sync_module_name", &SProcess::time_sync_module_name)
    .def_rw("component_init_state", &SProcess::component_init_state)
    .def_rw("component_init_info", &SProcess::component_init_info)
    .def_rw("ecal_runtime_version", &SProcess::ecal_runtime_version)
    .def_rw("config_file_path", &SProcess::config_file_path);

  // SMethod
  nb::class_<SMethod>(m, "Method")
    .def(nb::init<>())
    .def_rw("method_name", &SMethod::method_name)
    .def_rw("request_datatype_information", &SMethod::request_datatype_information)
    .def_rw("response_datatype_information", &SMethod::response_datatype_information)
    .def_rw("call_count", &SMethod::call_count);

  // SServer
  nb::class_<SServer>(m, "Server")
    .def(nb::init<>())
    .def_rw("registration_clock", &SServer::registration_clock)
    .def_rw("host_name", &SServer::host_name)
    .def_rw("process_name", &SServer::process_name)
    .def_rw("unit_name", &SServer::unit_name)
    .def_rw("process_id", &SServer::process_id)
    .def_rw("service_name", &SServer::service_name)
    .def_rw("service_id", &SServer::service_id)
    .def_rw("version", &SServer::version)
    .def_rw("tcp_port_v0", &SServer::tcp_port_v0)
    .def_rw("tcp_port_v1", &SServer::tcp_port_v1)
    .def_rw("methods", &SServer::methods);

  // SClient
  nb::class_<SClient>(m, "Client")
    .def(nb::init<>())
    .def_rw("registration_clock", &SClient::registration_clock)
    .def_rw("host_name", &SClient::host_name)
    .def_rw("process_name", &SClient::process_name)
    .def_rw("unit_name", &SClient::unit_name)
    .def_rw("process_id", &SClient::process_id)
    .def_rw("service_name", &SClient::service_name)
    .def_rw("service_id", &SClient::service_id)
    .def_rw("methods", &SClient::methods)
    .def_rw("version", &SClient::version);

  // Root struct (anonymous originally)
  nb::class_<SMonitoring>(m, "Monitoring")
    .def(nb::init<>())
    .def_rw("processes", &SMonitoring::processes)
    .def_rw("publishers", &SMonitoring::publishers)
    .def_rw("subscribers", &SMonitoring::subscribers)
    .def_rw("servers", &SMonitoring::servers)
    .def_rw("clients", &SMonitoring::clients);
}
