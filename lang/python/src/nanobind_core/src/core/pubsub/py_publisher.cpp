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

#include <core/pubsub/py_publisher.h>
#include <ecal/pubsub/publisher.h>

// Nanobind includes to map stl types to python types
#include <nanobind/stl/function.h>
#include <nanobind/stl/string.h>

#include <exception>

namespace nb = nanobind;
using namespace eCAL;

void AddPubsubPublisher(nanobind::module_& module)
{
  // Define CPublisher class
  nb::class_<CPublisher>(module, "Publisher")
    .def("__init__", [](CPublisher* t, nb::str topic_name, const SDataTypeInformation& datatype_info, const Publisher::Configuration& config_, const nb::object& event_callback_) { 
        PubEventCallbackT event_callback_cpp = nullptr;
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
        new (t) CPublisher(topic_name.c_str(), datatype_info, event_callback_cpp, config_);
      },
      nb::arg("topic_name"),
      nb::arg("data_type_info") = SDataTypeInformation(),
      nb::arg("config") = GetPublisherConfiguration(),
      nb::arg("event_callback") = nb::none()
    )
    // Send function for Python bytes
    .def("send", [](CPublisher& pub, nb::bytes payload, long long time) {
        const char* data = payload.c_str();
        size_t length = payload.size();
        // we need to release the GIL, so that we don't potentially deadlock ourselves.
        nb::gil_scoped_release release_gil;
        return pub.Send(data, length, time);
      },
      nb::arg("payload"), 
      nb::arg("time") = CPublisher::DEFAULT_TIME_ARGUMENT,
      "Send a message as raw bytes.")
    .def("get_subscriber_count", &CPublisher::GetSubscriberCount,
      "Get the number of connected subscribers.")
    .def("get_topic_name", &CPublisher::GetTopicName,
      "Retrieve the topic name.")
    .def("get_topic_id", &CPublisher::GetTopicId,
      "Retrieve the topic ID.")
    .def("get_data_type_information", &CPublisher::GetDataTypeInformation,
      "Get the topic's data type information.")
    .def("__repr__", [](const CPublisher& pub) {
    return "<Publisher topic='" + pub.GetTopicName() + "' subscribers=" +
      std::to_string(pub.GetSubscriberCount()) + ">";
      });
}
