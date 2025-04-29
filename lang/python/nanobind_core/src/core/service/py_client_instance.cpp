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

#include <core/service/py_client_instance.h>
#include <ecal/service/client_instance.h>

#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>
#include <nanobind/stl/set.h>
#include <nanobind/stl/tuple.h>
#include <nanobind/stl/pair.h>

#include <helper/bytestring_property.h>
#include <helper/make_gil_safe_shared.h>

namespace nb = nanobind;
using namespace nb::literals;

void AddServiceClientInstance(nanobind::module_& m)
{
  // Bind ClientInstance but disable Python construction
  nb::class_<eCAL::CClientInstance>(m, "ClientInstance", 
    "Handle to a specific eCAL service client instance.  \n"
    "Cannot be constructed directly from Python; you get it from the ServiceClient API."
    )
    // No .def(nb::init<…>()) here! -  Class shall not be constructible from Python

    .def("call_with_response",
      [](eCAL::CClientInstance& self,
        const std::string& method_name,
        nb::bytes python_request,
        int timeout_ms) -> std::pair<bool, eCAL::SServiceResponse>
      {
        // Convert Python bytes to std::string
        std::string cpp_request{ nb_bytes_to_std_string(python_request) };
        std::pair<bool, eCAL::SServiceResponse> result;
        {
          // Release the GIL while we do the blocking C++ call
          nb::gil_scoped_release release;
          result = self.CallWithResponse(method_name, cpp_request, timeout_ms);
        }
        // GIL is re-acquired here
        return result;
      },
      nb::arg("method_name"),
      nb::arg("request"),
      nb::arg("timeout_ms") = eCAL::CClientInstance::DEFAULT_TIME_ARGUMENT,
      "Blocking call on a single server instance; request is bytes, releases GIL, "
      "returns (success: bool, ServiceResponse).")

    .def("call_with_callback",
      [](eCAL::CClientInstance& self,
        const std::string& method_name,
        const nb::bytes& python_request,
        const nb::callable& python_callback,
        int timeout_ms) {
          auto python_callback_pointer = make_gil_safe_shared<nb::callable>(python_callback);
          eCAL::ResponseCallbackT cpp_callback = [python_callback_pointer](const eCAL::SServiceResponse& resp) {
            nb::gil_scoped_acquire acquire;
            try { (*python_callback_pointer)(resp); }
            catch (const std::exception& e) {
              std::cerr << "Response callback error: " << e.what() << std::endl;
            }
          };
          // Do the conversion before releasing the GIL
          std::string cpp_request{ nb_bytes_to_std_string(python_request) };

          nb::gil_scoped_release release;
          return self.CallWithCallback(method_name, cpp_request, cpp_callback, timeout_ms);
      },
      nb::arg("method_name"),
      nb::arg("request"),
      nb::arg("callback"),
      nb::arg("timeout_ms") = eCAL::CClientInstance::DEFAULT_TIME_ARGUMENT,
      "Blocking call with callback.")

    .def("call_with_callback_async",
      [](eCAL::CClientInstance& self,
        const std::string& method_name,
        const nb::bytes& python_request,
        const nb::callable& python_callback) {
          auto python_callback_pointer = make_gil_safe_shared<nb::callable>(python_callback);
          eCAL::ResponseCallbackT cpp_callback = [python_callback_pointer](const eCAL::SServiceResponse& resp) {
            nb::gil_scoped_acquire acquire;
            try { (*python_callback_pointer)(resp); }
            catch (const std::exception& e) {
              std::cerr << "Async callback error: " << e.what() << std::endl;
            }
          };
          // Do the conversion before releasing the GIL
          std::string cpp_request{ nb_bytes_to_std_string(python_request) };

          nb::gil_scoped_release release;
          return self.CallWithCallbackAsync(method_name, cpp_request, cpp_callback);
      },
      nb::arg("method_name"),
      nb::arg("request"),
      nb::arg("callback"),
      "Asynchronous call with callback.")

    .def("is_connected",
      &eCAL::CClientInstance::IsConnected,
      "Check whether this client instance is connected.")

    .def("get_client_id",
      &eCAL::CClientInstance::GetClientID,
      "Return the underlying SEntityId of this client instance.")

    .def("__repr__",
      [](const eCAL::CClientInstance& self) {
        auto id = self.GetClientID();
        return "<ClientInstance entity_id=" + std::to_string(id.entity_id) + ">";
      });
}
