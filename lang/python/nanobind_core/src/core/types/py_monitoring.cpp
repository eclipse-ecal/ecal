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

  // STopicMon
  nb::class_<STopicMon>(m, "TopicMon")
    .def(nb::init<>())
    .def_rw("registration_clock", &STopicMon::registration_clock)
    .def_rw("host_name", &STopicMon::host_name)
    .def_rw("shm_transport_domain", &STopicMon::shm_transport_domain)
    .def_rw("process_id", &STopicMon::process_id)
    .def_rw("process_name", &STopicMon::process_name)
    .def_rw("unit_name", &STopicMon::unit_name)
    .def_rw("topic_id", &STopicMon::topic_id)
    .def_rw("topic_name", &STopicMon::topic_name)
    .def_rw("direction", &STopicMon::direction)
    .def_rw("datatype_information", &STopicMon::datatype_information)
    .def_rw("transport_layer", &STopicMon::transport_layer)
    .def_rw("topic_size", &STopicMon::topic_size)
    .def_rw("connections_local", &STopicMon::connections_local)
    .def_rw("connections_external", &STopicMon::connections_external)
    .def_rw("message_drops", &STopicMon::message_drops)
    .def_rw("data_id", &STopicMon::data_id)
    .def_rw("data_clock", &STopicMon::data_clock)
    .def_rw("data_frequency", &STopicMon::data_frequency);

  // SProcessMon
  nb::class_<SProcessMon>(m, "ProcessMon")
    .def(nb::init<>())
    .def_rw("registration_clock", &SProcessMon::registration_clock)
    .def_rw("host_name", &SProcessMon::host_name)
    .def_rw("shm_transport_domain", &SProcessMon::shm_transport_domain)
    .def_rw("process_id", &SProcessMon::process_id)
    .def_rw("process_name", &SProcessMon::process_name)
    .def_rw("unit_name", &SProcessMon::unit_name)
    .def_rw("process_parameter", &SProcessMon::process_parameter)
    .def_rw("state_severity", &SProcessMon::state_severity)
    .def_rw("state_severity_level", &SProcessMon::state_severity_level)
    .def_rw("state_info", &SProcessMon::state_info)
    .def_rw("time_sync_state", &SProcessMon::time_sync_state)
    .def_rw("time_sync_module_name", &SProcessMon::time_sync_module_name)
    .def_rw("component_init_state", &SProcessMon::component_init_state)
    .def_rw("component_init_info", &SProcessMon::component_init_info)
    .def_rw("ecal_runtime_version", &SProcessMon::ecal_runtime_version)
    .def_rw("config_file_path", &SProcessMon::config_file_path);

  // SMethodMon
  nb::class_<SMethodMon>(m, "MethodMon")
    .def(nb::init<>())
    .def_rw("method_name", &SMethodMon::method_name)
    .def_rw("request_datatype_information", &SMethodMon::request_datatype_information)
    .def_rw("response_datatype_information", &SMethodMon::response_datatype_information)
    .def_rw("call_count", &SMethodMon::call_count);

  // SServerMon
  nb::class_<SServerMon>(m, "ServerMon")
    .def(nb::init<>())
    .def_rw("registration_clock", &SServerMon::registration_clock)
    .def_rw("host_name", &SServerMon::host_name)
    .def_rw("process_name", &SServerMon::process_name)
    .def_rw("unit_name", &SServerMon::unit_name)
    .def_rw("process_id", &SServerMon::process_id)
    .def_rw("service_name", &SServerMon::service_name)
    .def_rw("service_id", &SServerMon::service_id)
    .def_rw("version", &SServerMon::version)
    .def_rw("tcp_port_v0", &SServerMon::tcp_port_v0)
    .def_rw("tcp_port_v1", &SServerMon::tcp_port_v1)
    .def_rw("methods", &SServerMon::methods);

  // SClientMon
  nb::class_<SClientMon>(m, "ClientMon")
    .def(nb::init<>())
    .def_rw("registration_clock", &SClientMon::registration_clock)
    .def_rw("host_name", &SClientMon::host_name)
    .def_rw("process_name", &SClientMon::process_name)
    .def_rw("unit_name", &SClientMon::unit_name)
    .def_rw("process_id", &SClientMon::process_id)
    .def_rw("service_name", &SClientMon::service_name)
    .def_rw("service_id", &SClientMon::service_id)
    .def_rw("methods", &SClientMon::methods)
    .def_rw("version", &SClientMon::version);

  // Root struct (anonymous originally)
  nb::class_<SMonitoring>(m, "Monitoring")
    .def(nb::init<>())
    .def_rw("processes", &SMonitoring::processes)
    .def_rw("publishers", &SMonitoring::publisher)
    .def_rw("subscribers", &SMonitoring::subscriber)
    .def_rw("servers", &SMonitoring::server)
    .def_rw("clients", &SMonitoring::clients);
}
