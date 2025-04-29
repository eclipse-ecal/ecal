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

#include <core/service/py_client.h>
#include <ecal/service/client.h>

#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>
#include <nanobind/stl/set.h>
#include <nanobind/stl/tuple.h>

#include <helper/bytestring_property.h>
#include <helper/make_gil_safe_shared.h>

namespace nb = nanobind;
using namespace nb::literals;

void AddServiceClient(nanobind::module_& m)
{
  nb::class_<eCAL::CServiceClient>(m, "ServiceClient", "Wrapper for eCAL service client.")
    // Expose constructor with optional method set and event callback
    .def("__init__",
      [](eCAL::CServiceClient* self,
        nb::str service_name,
        const eCAL::ServiceMethodInformationSetT& methods,
        const nb::object& callback_python)
      {
        // Wrap optional Python callback
        eCAL::ClientEventCallbackT event_callback_cpp = nullptr;
        if (!callback_python.is_none()) {
          event_callback_cpp = [callback_python](const eCAL::SServiceId& sid,
            const eCAL::SClientEventCallbackData& data) {
              try {
                nb::gil_scoped_acquire acquire;
                callback_python(sid, data);
              }
              catch (const std::exception& e) {
                std::cerr << "Client event callback error: " << e.what() << std::endl;
              }
          };
        }
        // Placement-new
        new (self) eCAL::CServiceClient(service_name.c_str(), methods, event_callback_cpp);
      },
      nb::arg("service_name"),
      nb::arg("methods") = eCAL::ServiceMethodInformationSetT(),
      nb::arg("event_callback") = nb::none(),
        "Create a ServiceClient for `service_name`, optionally specifying methods and an event callback.")


    .def_ro_static("DEFAULT_TIME_ARGUMENT",
      &eCAL::CServiceClient::DEFAULT_TIME_ARGUMENT,
      "Use for infinite timeout in call methods.")

    .def("get_client_instances",
      &eCAL::CServiceClient::GetClientInstances,
      "Return a list of ClientInstance objects for all matching service servers.")

    .def("call_with_response",
      [](const eCAL::CServiceClient& self,
        const std::string& method_name,
        const nb::bytes& request,
        int timeout_ms) -> nb::object
      {
        eCAL::ServiceResponseVecT resp_vec;
        bool ok = false;
        {
          // Release the GIL during the potentially long C++ call
          nb::gil_scoped_release release;
          ok = self.CallWithResponse(method_name, nb_bytes_to_std_string(request), resp_vec, timeout_ms);
        }
        if (ok) return nb::cast(resp_vec);
        return nb::none();
      },
      nb::arg("method_name"),
      nb::arg("request"),
      nb::arg("timeout_ms") = eCAL::CServiceClient::DEFAULT_TIME_ARGUMENT,
      "Blocking call on all servers; returns list of ServiceResponse if successful, else None.")

    .def("call_with_callback",
      [](const eCAL::CServiceClient& self,
        const std::string& method_name,
        const nb::bytes& request,
        const nb::callable& python_callback,
        int timeout_ms) -> bool
      {
        auto python_callback_pointer = make_gil_safe_shared<nb::callable>(python_callback);
        eCAL::ResponseCallbackT cb = [python_callback_pointer](const eCAL::SServiceResponse& resp) {
          try {
            nb::gil_scoped_acquire acquire;
            (*python_callback_pointer)(resp);
          }
          catch (const std::exception& e) {
            std::cerr << "Response callback error: " << e.what() << std::endl;
          }
        };
        nb::gil_scoped_release release;
        return self.CallWithCallback(method_name, nb_bytes_to_std_string(request), cb, timeout_ms);
      },
      nb::arg("method_name"),
      nb::arg("request"),
      nb::arg("callback"),
      nb::arg("timeout_ms") = eCAL::CServiceClient::DEFAULT_TIME_ARGUMENT,
      "Blocking call with callback on each response; returns True if the call was issued.")

    .def("call_with_callback_async",
      [](const eCAL::CServiceClient& self,
          const std::string& method_name,
          const nb::bytes& request,
          const nb::callable& python_callback) -> bool
        {
          auto python_callback_pointer = make_gil_safe_shared<nb::callable>(python_callback);
          eCAL::ResponseCallbackT cb = [python_callback_pointer](const eCAL::SServiceResponse& resp) {
          try {
            nb::gil_scoped_acquire acquire;
            (*python_callback_pointer)(resp);
          }
          catch (const std::exception& e) {
            std::cerr << "Async response callback error: " << e.what() << std::endl;
          }
        };
        nb::gil_scoped_release release;
        return self.CallWithCallbackAsync(method_name, nb_bytes_to_std_string(request), cb);
      },
      nb::arg("method_name"),
      nb::arg("request"),
      nb::arg("callback"),
      "Asynchronous call with callback on each response; returns True if the call was issued.")

    .def("get_service_name",
      &eCAL::CServiceClient::GetServiceName,
      "Get the name of the service this client is bound to.")

    .def("get_service_id",
      &eCAL::CServiceClient::GetServiceId,
      "Get the unique service ID of this client.")

    .def("is_connected",
      &eCAL::CServiceClient::IsConnected,
      "Return True if at least one server instance is connected.");
}
