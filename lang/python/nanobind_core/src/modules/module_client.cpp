/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2019 Continental Corporation
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
#include <wrappers/client.h>

void AddClientClassToModule(const nanobind::module_& module) 
{
    auto ServiceClient_cls = nanobind::class_<eCAL::CNBSrvClient>(module, "ServiceClient")
        .def(nanobind::init<>())
        .def(nanobind::init<const std::string&>())
        .def("create", &eCAL::CNBSrvClient::Create)
        .def("set_hostname", &eCAL::CNBSrvClient::SetHostName)
        .def("destroy", &eCAL::CNBSrvClient::Destroy)
        .def("call", nanobind::overload_cast<const std::string&, const std::string&, int>(&eCAL::CNBSrvClient::Call))
        .def("call", nanobind::overload_cast<const std::string&, const std::string&, int, eCAL::ServiceResponseVecT*>(&eCAL::CNBSrvClient::Call))
        .def("call_async", &eCAL::CNBSrvClient::CallAsync)
        .def("add_response_callback", &eCAL::CNBSrvClient::WrapAddRespCB)
        .def("rem_response_callback", &eCAL::CNBSrvClient::RemResponseCallback)
        .def("rem_event_callback", &eCAL::CNBSrvClient::RemEventCallback)
        .def("add_event_callback", &eCAL::CNBSrvClient::AddEventCallback)
        .def("is_connected", &eCAL::CNBSrvClient::IsConnected)
        .def("get_service_name", &eCAL::CNBSrvClient::GetServiceName);
}
