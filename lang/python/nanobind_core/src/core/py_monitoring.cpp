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
  nb::module_ m_monitoring = m.def_submodule("monitoring", "eCAL Monitoring related functionality");

  // Enum values from Entity namespace
  m_monitoring.attr("Publisher") = eCAL::Monitoring::Entity::Publisher;
  m_monitoring.attr("Subscriber") = eCAL::Monitoring::Entity::Subscriber;
  m_monitoring.attr("Server") = eCAL::Monitoring::Entity::Server;
  m_monitoring.attr("Client") = eCAL::Monitoring::Entity::Client;
  m_monitoring.attr("Process") = eCAL::Monitoring::Entity::Process;
  m_monitoring.attr("Host") = eCAL::Monitoring::Entity::Host;
  m_monitoring.attr("All") = eCAL::Monitoring::Entity::All;
  m_monitoring.attr("None") = eCAL::Monitoring::Entity::None;

  // GetMonitoring: serialized string version
  m_monitoring.def("get_serialized_monitoring", [](unsigned int entities = eCAL::Monitoring::Entity::All) {
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
  m_monitoring.def("get_monitoring", [](unsigned int entities = eCAL::Monitoring::Entity::All) {
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
