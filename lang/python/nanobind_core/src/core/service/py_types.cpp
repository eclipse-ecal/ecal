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

#include <core/service/py_types.h>
#include <ecal/service/types.h>

#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>
#include <nanobind/stl/set.h>
#include <nanobind/stl/tuple.h>

#include <helper/bytestring_property.h>

namespace nb = nanobind;
using namespace nb::literals;

void AddServiceTypes(nanobind::module_& m)
{
  nb::enum_<eCAL::eCallState>(m, "CallState")
    .value("NONE", eCAL::eCallState::none)
    .value("EXECUTED", eCAL::eCallState::executed)
    .value("TIMEOUTED", eCAL::eCallState::timeouted)
    .value("FAILED", eCAL::eCallState::failed);

  nb::enum_<eCAL::eClientEvent>(m, "ClientEvent")
    .value("NONE", eCAL::eClientEvent::none)
    .value("CONNECTED", eCAL::eClientEvent::connected)
    .value("DISCONNECTED", eCAL::eClientEvent::disconnected)
    .value("TIMEOUT", eCAL::eClientEvent::timeout);

  nb::enum_<eCAL::eServerEvent>(m, "ServerEvent")
    .value("NONE", eCAL::eServerEvent::none)
    .value("CONNECTED", eCAL::eServerEvent::connected)
    .value("DISCONNECTED", eCAL::eServerEvent::disconnected);

  nb::class_<eCAL::SServiceId>(m, "ServiceId")
    .def(nb::init<>())
    .def_rw("service_id", &eCAL::SServiceId::service_id)
    .def_rw("service_name", &eCAL::SServiceId::service_name)
    .def("__eq__", &eCAL::SServiceId::operator==)
    .def("__lt__", &eCAL::SServiceId::operator<);

  nb::class_<eCAL::SServiceMethodInformation>(m, "ServiceMethodInformation")
    .def(nb::init<>())
    .def_rw("method_name", &eCAL::SServiceMethodInformation::method_name)
    .def_rw("request_type", &eCAL::SServiceMethodInformation::request_type)
    .def_rw("response_type", &eCAL::SServiceMethodInformation::response_type)
    .def("__eq__", &eCAL::SServiceMethodInformation::operator==)
    .def("__lt__", &eCAL::SServiceMethodInformation::operator<);

  nb::class_<eCAL::SServiceResponse>(m, "ServiceResponse")
    .def(nb::init<>())
    .def_rw("call_state", &eCAL::SServiceResponse::call_state)
    .def_rw("server_id", &eCAL::SServiceResponse::server_id)
    .def_rw("service_method_information", &eCAL::SServiceResponse::service_method_information)
    .def_rw("ret_state", &eCAL::SServiceResponse::ret_state)
    .def(bytestring_property("response", &eCAL::SServiceResponse::response))
    .def_rw("error_msg", &eCAL::SServiceResponse::error_msg);

  nb::class_<eCAL::SClientEventCallbackData>(m, "ClientEventCallbackData")
    .def(nb::init<>())
    .def_rw("type", &eCAL::SClientEventCallbackData::type)
    .def_rw("time", &eCAL::SClientEventCallbackData::time);

  nb::class_<eCAL::SServerEventCallbackData>(m, "ServerEventCallbackData")
    .def(nb::init<>())
    .def_rw("type", &eCAL::SServerEventCallbackData::type)
    .def_rw("time", &eCAL::SServerEventCallbackData::time);
}
