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
#include <sstream>         // for constructing __repr__ strings

// Include the header file that contains the eCAL types.
// Adjust this include to match your actual header file.
#include <ecal/types.h>
#include <helper/bytestring_property.h>

namespace nb = nanobind;
using namespace nb::literals;  // Bring in the _a literal for named arguments

void AddTypes(nb::module_& m) {
  // Wrap SVersion
  // Note: Since SVersion members are const, we provide a constructor that takes
  // three ints for initialization.
  nb::class_<eCAL::SVersion>(m, "Version")
    .def(nb::init<int, int, int>(), "major"_a, "minor"_a, "patch"_a)
    .def_ro("major", &eCAL::SVersion::major)
    .def_ro("minor", &eCAL::SVersion::minor)
    .def_ro("patch", &eCAL::SVersion::patch)
    .def("__repr__", [](const eCAL::SVersion& v) {
    return "<Version major=" + std::to_string(v.major) +
      ", minor=" + std::to_string(v.minor) +
      ", patch=" + std::to_string(v.patch) + ">";
      });

  // Wrap SDataTypeInformation
  nb::class_<eCAL::SDataTypeInformation>(m, "DataTypeInformation")
    .def(nb::init<>())
    .def_rw("name", &eCAL::SDataTypeInformation::name)
    .def_rw("encoding", &eCAL::SDataTypeInformation::encoding)
    // descriptors are bytes, e.g. the std::string needs to be mapped to python bytes
    .def(bytestring_property("descriptor", &eCAL::SDataTypeInformation::descriptor))
    //.def("clear", &eCAL::SDataTypeInformation::clear,
    //  "Clears the name, encoding, and descriptor strings.")
    .def("__eq__", [](const eCAL::SDataTypeInformation& a, const eCAL::SDataTypeInformation& b) {
    return a == b;
      })
    .def("__lt__", [](const eCAL::SDataTypeInformation& a, const eCAL::SDataTypeInformation& b) {
        return a < b;
      })
    .def("__repr__", [](const eCAL::SDataTypeInformation& info) {
      return "<DataTypeInformation name='" + info.name +
        "', encoding='" + info.encoding +
        "', descriptor='" + info.descriptor + "'>";
      });

    // Wrap SEntityId
    nb::class_<eCAL::SEntityId>(m, "EntityId")
        .def(nb::init<>())
        .def_rw("entity_id", &eCAL::SEntityId::entity_id)
        .def_rw("process_id", &eCAL::SEntityId::process_id)
        .def_rw("host_name", &eCAL::SEntityId::host_name)
        .def("__eq__", [](const eCAL::SEntityId& a, const eCAL::SEntityId& b) {
        return a == b;
          })
        .def("__lt__", [](const eCAL::SEntityId& a, const eCAL::SEntityId& b) {
            return a < b;
          })
            .def("__repr__", [](const eCAL::SEntityId& id) {
            std::ostringstream oss;
            oss << "SEntityId(entity_id: " << id.entity_id
              << ", process_id: " << id.process_id
              << ", host_name: " << id.host_name << ")";
            return oss.str();
              });
}
