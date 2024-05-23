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
 * @brief  Add ServiceServer class to nanobind module
**/


#include <modules/module_server.h>
#include <wrappers/server.h>

void AddServerClassToModule(const nanobind::module_& module) 
{
    // Class and Functions from ecal_server.h
    auto ServiceServer_cls = nanobind::class_<eCAL::CNBSrvServer>(module, "ServiceServer")
        .def(nanobind::init<>())
        .def(nanobind::init<const std::string&>())
        .def("create", &eCAL::CNBSrvServer::Create)
        .def("add_description", &eCAL::CNBSrvServer::AddDescription)
        .def("destroy", &eCAL::CNBSrvServer::Destroy)
        .def("add_method_callback", &eCAL::CNBSrvServer::WrapAddMethodCB)
        .def("rem_method_callback", &eCAL::CNBSrvServer::RemMethodCallback)
        .def("rem_event_callback", &eCAL::CNBSrvServer::RemEventCallback)
        .def("add_event_callback", &eCAL::CNBSrvServer::AddEventCallback)
        .def("is_connected", &eCAL::CNBSrvServer::IsConnected)
        .def("get_service_name", &eCAL::CNBSrvServer::GetServiceName);
}
