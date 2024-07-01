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
 * @brief  Add ServiceServer class to nanobind module
**/


#include <modules/module_server.h>
#include <wrappers/wrapper_server.h>

void AddServerClassToModule(const nanobind::module_& module) 
{
    /**
    * @brief eCAL service server event callback type.
    **/
    nanobind::enum_<eCAL_Server_Event>(module, "eCAL_Server_Event")
        .value("server_event_none", eCAL_Server_Event::server_event_none)
        .value("server_event_connected", eCAL_Server_Event::server_event_connected)
        .value("server_event_disconnected", eCAL_Server_Event::server_event_disconnected)
        .export_values();

    // Struct eCAL::SServiceResponse
    nanobind::class_<eCAL::SServiceResponse>(module, "ServiceResponse")
        .def(nanobind::init<>())
        .def_rw("host_name", &eCAL::SServiceResponse::host_name)
        .def_rw("service_name", &eCAL::SServiceResponse::service_name)
        .def_rw("service_id", &eCAL::SServiceResponse::service_id)
        .def_rw("method_name", &eCAL::SServiceResponse::method_name)
        .def_rw("error_msg", &eCAL::SServiceResponse::error_msg)
        .def_rw("ret_state", &eCAL::SServiceResponse::ret_state)
        .def_rw("call_state", &eCAL::SServiceResponse::call_state)
        .def_rw("response", &eCAL::SServiceResponse::response);

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
        .def("add_event_callback", &eCAL::CNBSrvServer::WrapAddSrvEventCB)
        .def("is_connected", &eCAL::CNBSrvServer::IsConnected)
        .def("get_service_name", &eCAL::CNBSrvServer::GetServiceName);
}
