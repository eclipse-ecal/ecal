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
 * @brief  Add ServiceClient class to nanobind module
**/


#include <modules/module_datatypeinfo.h>
#include <wrappers/wrapper_datatypeinfo.h>

void AddDataTypeInfoStructToModule(nanobind::module_& module)
{
    // Struct eCAL::SDataTypeInformation
    nanobind::class_<eCAL::CNBDataTypeInformation>(module, "DataTypeInformation")
        .def(nanobind::init<>())
        .def_rw("name", &eCAL::CNBDataTypeInformation::name)
        .def_rw("encoding", &eCAL::CNBDataTypeInformation::encoding)
        .def_rw("descriptor", &eCAL::CNBDataTypeInformation::descriptor);

    nanobind::class_<eCAL::Types::IpAddressV4>(module, "IpAddressV4")
        .def(nanobind::init<const std::string&>(), nanobind::arg("ip_address"))

        // Member function binding
        .def("Get", &eCAL::Types::IpAddressV4::Get, "Returns the IP address as a string.")

        // Assignment operator overloads
        .def("assign", nanobind::overload_cast<const std::string&>(&eCAL::Types::IpAddressV4::operator=), nanobind::arg("ip_string"))
        .def("assign", nanobind::overload_cast<const char*>(&eCAL::Types::IpAddressV4::operator=), nanobind::arg("ip_string"))

        // Conversion to string
        .def("__str__", [](const eCAL::Types::IpAddressV4& self) { return std::string(self); });
}
