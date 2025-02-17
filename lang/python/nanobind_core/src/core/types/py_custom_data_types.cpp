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

#include <core/types/py_custom_data_types.h>
#include <ecal/types/custom_data_types.h>

#include <nanobind/stl/string.h>

namespace nb = nanobind;
using namespace eCAL;

void AddTypesCustomDataTypes(nanobind::module_& module)
{
  // Bind UdpConfigVersion enum
  nb::enum_<Types::UdpConfigVersion>(module, "UdpConfigVersion")
    .value("V1", Types::UdpConfigVersion::V1)
    .value("V2", Types::UdpConfigVersion::V2)
    .def("__repr__", [](Types::UdpConfigVersion version) {
    return version == Types::UdpConfigVersion::V1 ? "UdpConfigVersion.V1" : "UdpConfigVersion.V2";
      });

  // Bind UDPMode enum
  nb::enum_<Types::UDPMode>(module, "UDPMode")
    .value("NETWORK", Types::UDPMode::NETWORK)
    .value("LOCAL",   Types::UDPMode::LOCAL)
    .def("__repr__", [](Types::UDPMode mode) {
        return mode == Types::UDPMode::NETWORK ? "UDPMode.NETWORK" : "UDPMode.LOCAL";
      });

  // Bind IpAddressV4 class
  nb::class_<Types::IpAddressV4>(module, "IpAddressV4")
    .def(nb::init<const std::string&>(), nb::arg("ip_address"), "Initialize from a string IP address")
    .def("__str__", &Types::IpAddressV4::Get)
    .def("__repr__", [](const Types::IpAddressV4& ip) {
    return "<IpAddressV4 '" + ip.Get() + "'>";
      });
}