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


#include <modules/module_client.h>
#include <wrappers/wrapper_client.h>
#include <ecal/service/types.h>

void AddClientClassToModule(const nanobind::module_& module) 
{
    /**
    * @brief eCAL service client event callback type.
    **/
    nanobind::enum_<eCAL::eClientEvent>(module, "ClientEvent")
        .value("none", eCAL::eClientEvent::none)
        .value("connected", eCAL::eClientEvent::connected)
        .value("disconnected", eCAL::eClientEvent::disconnected)
        .value("timeout", eCAL::eClientEvent::timeout)
        .export_values();

    auto ServiceClient_cls = nanobind::class_<eCAL::CNBSrvClient>(module, "ServiceClient")
        .def(nanobind::init<>())
        .def(nanobind::init<const std::string&>())
        .def("call", nanobind::overload_cast<const std::string&, const std::string&, int>(&eCAL::CNBSrvClient::Call))
        .def("call", nanobind::overload_cast<const std::string&, const std::string&, int, eCAL::ServiceResponseVecT*>(&eCAL::CNBSrvClient::Call))
        .def("call_async", &eCAL::CNBSrvClient::CallAsync)
        .def("set_response_callback", &eCAL::CNBSrvClient::WrapAddRespCB)
        .def("remove_response_callback", &eCAL::CNBSrvClient::RemoveResponseCallback)
        .def("add_event_callback", &eCAL::CNBSrvClient::WrapAddClientEventCB)
        .def("is_connected", &eCAL::CNBSrvClient::IsConnected)
        .def("get_service_name", &eCAL::CNBSrvClient::GetServiceName);
}
