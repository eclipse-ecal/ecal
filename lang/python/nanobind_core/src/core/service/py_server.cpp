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

#include <core/service/py_server.h>
#include <ecal/service/server.h>

#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>
#include <nanobind/stl/set.h>
#include <nanobind/stl/tuple.h>

#include <helper/bytestring_property.h>

namespace nb = nanobind;
using namespace nb::literals;

void AddServiceServer(nanobind::module_& m)
{
  nb::class_<eCAL::CServiceServer>(m, "ServiceServer", "Service server for handling RPC calls in eCAL.")
    .def("__init__",
      [](eCAL::CServiceServer* self,
        nb::str service_name,
        const nb::object& event_callback_obj) {

          eCAL::ServerEventCallbackT event_callback_cpp = nullptr;
          if (!event_callback_obj.is_none()) {
            event_callback_cpp = [event_callback_obj](const eCAL::SServiceId& service_id,
              const eCAL::SServerEventCallbackData& data) {
                try {
                  nb::gil_scoped_acquire acquire;
                  event_callback_obj(service_id, data);
                }
                catch (const std::exception& e) {
                  std::cerr << "Error in server event callback: " << e.what() << std::endl;
                }
            };
          }

          new (self) eCAL::CServiceServer(service_name.c_str(), event_callback_cpp);
      },
      nb::arg("service_name"),
        nb::arg("event_callback") = nb::none(),
        "Create a service server with an optional event callback.")

    .def("set_method_callback",
      [](eCAL::CServiceServer& self,
        const eCAL::SServiceMethodInformation& method_info,
        const nb::callable& py_callback) {
          // Wrap the Python callback
          std::shared_ptr<nb::callable> python_callback_pointer = std::make_shared<nb::callable>(py_callback);

          auto wrapped_callback = [python_callback_pointer](
            const eCAL::SServiceMethodInformation& cpp_info,
            const std::string& cpp_request,
            std::string& cpp_response) -> int
          {
            try {
              // We aquire the gil, because now we need to call into Python code
              nb::gil_scoped_acquire acquire;
              nb::object result = (*python_callback_pointer)(cpp_info, std_string_to_nb_bytes(cpp_request));

              // Expect the Python callback to return a tuple (int (return_code), bytes (response))
              auto result_tuple = nb::cast<std::tuple<int, nb::bytes>>(result);

              int return_code = std::get<0>(result_tuple);
              const auto& python_response = std::get<1>(result_tuple);
              cpp_response = nb_bytes_to_std_string(python_response);
              return return_code;
            }
            catch (const std::exception& e) {
              std::cerr << "Error invoking method callback: " << e.what() << std::endl;
              return -1;
            }
          };

          nb::gil_scoped_release release;
          return self.SetMethodCallback(method_info, wrapped_callback);
      },
      nb::arg("method_info"),
      nb::arg("callback"),
        "Set a callback for a specific method.")

    .def("remove_method_callback",
      &eCAL::CServiceServer::RemoveMethodCallback,
      nb::arg("method"),
      "Remove a previously registered method callback.")

    .def("get_service_name",
      &eCAL::CServiceServer::GetServiceName,
      "Return the name of the service.")

    .def("get_service_id",
      &eCAL::CServiceServer::GetServiceId,
      "Return the service ID.")

    .def("is_connected",
      &eCAL::CServiceServer::IsConnected,
      "Check if the server is connected to any client.")

    .def("__repr__", [](const eCAL::CServiceServer& self) {
       return std::string("<ServiceServer name='" + self.GetServiceName() + "'>");
      });

}
