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
 * @brief  Add ServiceServer class to nanobind module
**/


#include <modules/module_server.h>
#include <wrappers/wrapper_server.h>
#include <ecal/service/types.h>

void AddServerClassToModule(const nanobind::module_& module) 
{
    /**
    * @brief eCAL service server event callback type.
    **/
    nanobind::enum_<eCAL::eServerEvent>(module, "ServerEvent")
        .value("server_event_none", eCAL::eServerEvent::none)
        .value("server_event_connected", eCAL::eServerEvent::connected)
        .value("server_event_disconnected", eCAL::eServerEvent::disconnected)
        .export_values();

    // Struct eCAL::SServiceResponse
    nanobind::class_<eCAL::SServiceResponse>(module, "ServiceResponse")
      .def(nanobind::init<>())
      .def_rw("call_state", &eCAL::SServiceResponse::call_state)
      .def_rw("server_id", &eCAL::SServiceResponse::server_id)
      .def_rw("service_method_information", &eCAL::SServiceResponse::service_method_information)
      .def_rw("ret_state", &eCAL::SServiceResponse::ret_state)
      .def_rw("response", &eCAL::SServiceResponse::response)
      .def_rw("error_msg", &eCAL::SServiceResponse::error_msg);

    // Class and Functions from ecal_server.h
    auto ServiceServer_cls = nanobind::class_<eCAL::CServiceServer>(module, "ServiceServer")
        .def(nanobind::init<>())
        .def(nanobind::init<const std::string&>())
        //.def("set_method_callback", &eCAL::CNBSrvServer::WrapAddMethodCB)
        .def("set_method_callback",
          [](eCAL::CServiceServer& self,
          const eCAL::SServiceMethodInformation& method_info,
            nanobind::object callback) -> bool {
            eCAL::ServiceMethodCallbackT cb;  // default callback (empty)

            if (callback && nanobind::isinstance<nanobind::function>(callback)) {
              // Create a lambda that wraps the Python callback.
              //
              // Note: Replace RequestType and ResponseType with the actual types expected.
              cb = [py_cb = callback](const eCAL::SServiceMethodInformation& method_info_, const std::string& request_, std::string& response_) -> bool {
                nanobind::gil_scoped_acquire gil;  // ensure GIL is held

                // Convert the C++ types to Python objects.
                // (Assumes nanobind can cast these types. If not, you must write conversion code.)
                nanobind::object py_req = nanobind::cast(request_);
                nanobind::object py_res = nanobind::cast(res);

                // Call the Python callback. (The signature should match your C++ callback signature.)
                nanobind::object result = py_cb(py_req, py_res);

                // Convert the result back to bool.
                return nanobind::cast<bool>(result);
              };
            }
            return self.SetMethodCallback(method_info, cb);
          },
          "method_info"_a, "callback"_a)
            .def("remove_method_callback", &eCAL::CNBSrvServer::RemoveMethodCallback)
            .def("get_service_name", &eCAL::CNBSrvServer::GetServiceName)
            .def("get_service_id", &eCAL::CNBSrvServer::GetServiceId)
            .def("is_connected", &eCAL::CNBSrvServer::IsConnected);
}
