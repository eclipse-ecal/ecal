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

#include <core/py_monitoring.h>

#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>
#include <nanobind/stl/map.h>
#include <nanobind/stl/optional.h>

#include <ecal/monitoring.h>

namespace nb = nanobind;
using namespace nb::literals;

void AddMonitoring(nanobind::module_& m)
{
  // Enum values from Entity namespace
  nb::module_ entity_mod = m.def_submodule("Entity");
  entity_mod.attr("Publisher") = eCAL::Monitoring::Entity::Publisher;
  entity_mod.attr("Subscriber") = eCAL::Monitoring::Entity::Subscriber;
  entity_mod.attr("Server") = eCAL::Monitoring::Entity::Server;
  entity_mod.attr("Client") = eCAL::Monitoring::Entity::Client;
  entity_mod.attr("Process") = eCAL::Monitoring::Entity::Process;
  entity_mod.attr("Host") = eCAL::Monitoring::Entity::Host;
  entity_mod.attr("All") = eCAL::Monitoring::Entity::All;
  entity_mod.attr("None") = eCAL::Monitoring::Entity::None;

  // GetMonitoring: serialized string version
  m.def("get_serialized_monitoring", [](unsigned int entities = eCAL::Monitoring::Entity::All) {
    //nb::gil_scoped_release release_gil;
    std::string result;
    if (eCAL::Monitoring::GetMonitoring(result, entities)) {
      return nb::bytes(result.c_str(), result.size());
    }
    else {
      throw std::runtime_error("Failed to get monitoring string.");
    }
    }, "entities"_a = eCAL::Monitoring::Entity::All,
    "Returns monitoring info as a serialized string. Can be deserialized using monitoring.proto");

  // GetMonitoring: struct version
  m.def("get_monitoring", [](unsigned int entities = eCAL::Monitoring::Entity::All) {
    //nb::gil_scoped_release release_gil;
    eCAL::Monitoring::SMonitoring mon;
    if (eCAL::Monitoring::GetMonitoring(mon, entities)) {
      return mon;
    }
    else {
      throw std::runtime_error("Failed to get monitoring struct.");
    }
    }, "entities"_a = eCAL::Monitoring::Entity::All,
    "Returns monitoring info as an object");
}
