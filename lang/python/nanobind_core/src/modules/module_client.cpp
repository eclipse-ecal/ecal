/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2024 Continental Corporation
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
#include <ecal/cimpl/ecal_callback_cimpl.h>

void AddClientClassToModule(const nanobind::module_& module) 
{
    /**
    * @brief eCAL service client event callback type.
    **/
    nanobind::enum_<eCAL_Client_Event>(module, "eCAL_Client_Event")
        .value("client_event_none", eCAL_Client_Event::client_event_none)
        .value("client_event_connected", eCAL_Client_Event::client_event_none)
        .value("client_event_disconnected", eCAL_Client_Event::client_event_none)
        .value("client_event_timeout", eCAL_Client_Event::client_event_none)
        .export_values();

    auto ServiceClient_cls = nanobind::class_<eCAL::CNBSrvClient>(module, "ServiceClient")
        .def(nanobind::init<>())
        .def(nanobind::init<const std::string&>())
         // TODO Ariff : we now need the overloads here (with also Information, which also needs to be wrapped)
        //.def("create", &eCAL::CNBSrvClient::Create)
        .def("set_hostname", &eCAL::CNBSrvClient::SetHostName)
        .def("destroy", &eCAL::CNBSrvClient::Destroy)
        .def("call", nanobind::overload_cast<const std::string&, const std::string&, int>(&eCAL::CNBSrvClient::Call))
        .def("call", nanobind::overload_cast<const std::string&, const std::string&, int, eCAL::ServiceResponseVecT*>(&eCAL::CNBSrvClient::Call))
        .def("call_async", &eCAL::CNBSrvClient::CallAsync)
        .def("add_response_callback", &eCAL::CNBSrvClient::WrapAddRespCB)
        .def("rem_response_callback", &eCAL::CNBSrvClient::RemResponseCallback)
        .def("rem_event_callback", &eCAL::CNBSrvClient::RemEventCallback)
        .def("add_event_callback", &eCAL::CNBSrvClient::WrapAddClientEventCB)
        .def("is_connected", &eCAL::CNBSrvClient::IsConnected)
        .def("get_service_name", &eCAL::CNBSrvClient::GetServiceName);
}
