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

#include <core/pubsub/py_subscriber.h>
#include <ecal/pubsub/subscriber.h>

#include <nanobind/stl/function.h>
#include <nanobind/stl/string.h>

#include <helper/make_gil_safe_shared.h>

#include <exception>

namespace nb = nanobind;
using namespace eCAL;

void AddPubsubSubscriber(nanobind::module_& module)
{
    // Define Subscriber class
  nb::class_<CSubscriber>(module, "Subscriber")
    .def("__init__", [](CSubscriber* t, nb::str topic_name, const SDataTypeInformation& datatype_info, const Subscriber::Configuration& config_, const nb::object& event_callback_)
      { 
        SubEventCallbackT event_callback_cpp = nullptr;
        if (!event_callback_.is_none())
        {
          event_callback_cpp = [event_callback_](auto&&... args) {
            try {
              nb::gil_scoped_acquire acquire;
              // Call the Python callback, forwarding the arguments.
              event_callback_(std::forward<decltype(args)>(args)...);
            }
            catch (const std::exception& e)
            {
              std::cout << "Error invoking event callback: " << e.what() << std::endl;
            }
          };
        }
        new (t) CSubscriber(topic_name.c_str(), datatype_info, event_callback_cpp, config_);
      },
      nb::arg("topic_name"),
      nb::arg("data_type_info") = SDataTypeInformation(),
      nb::arg("config") = GetSubscriberConfiguration(),
      nb::arg("event_callback") = nb::none()
    )
    .def("set_receive_callback",
      [](CSubscriber& self, const nb::callable& py_callback) {
        // Wrap the Python callback with a lambda that acquires the GIL.
        // Adjust the callback parameters according to the actual signature of ReceiveCallbackT.
        // We use a shared pointer of the callback, because this way the reference count does not have to be
        // increased when we set the callback. Because we cannot hold the GIL when we set the callback, du to potential deadlocks
        // also we need to make sure that the GIL is held whenever the callback is destroyed
        auto python_callback_pointer = make_gil_safe_shared<nb::callable>(py_callback);
        auto wrapped_callback = [python_callback_pointer](auto&&... args) {
          try {
            nb::gil_scoped_acquire acquire;
            // Call the Python callback, forwarding the arguments.
            (*python_callback_pointer)(std::forward<decltype(args)>(args)...);
          }
          catch (const std::exception& e)
          {
            std::cout << "Error invoking callback: " << e.what() << std::endl;
          }
        };

        nb::gil_scoped_release release;
        self.SetReceiveCallback(wrapped_callback);
      },
      nb::arg("callback"))
    .def("remove_receive_callback", &CSubscriber::RemoveReceiveCallback,
      "Remove a previously set receive callback.")
    .def("get_publisher_count", &CSubscriber::GetPublisherCount,
      "Get the number of connected publishers.")
    .def("get_topic_name", &CSubscriber::GetTopicName,
      "Retrieve the topic name.")
    .def("get_topic_id", &CSubscriber::GetTopicId,
      "Retrieve the topic ID.")
    .def("get_data_type_information", &CSubscriber::GetDataTypeInformation,
      "Get the topic's data type information.")
    .def("__repr__", [](const CSubscriber& pub) {
    return "<Subscriber topic='" + pub.GetTopicName() + "' publishers=" +
      std::to_string(pub.GetPublisherCount()) + ">";
      });
}