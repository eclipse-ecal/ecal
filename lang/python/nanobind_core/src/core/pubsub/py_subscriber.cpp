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
 * @file   pubsub/subscriber.h
 * @brief  eCAL subscriber interface
**/

#pragma once

#include <core/pubsub/py_subscriber.h>
#include <ecal/pubsub/subscriber.h>

#include <exception>

namespace nb = nanobind;
using namespace eCAL;

void AddPubsubSubscriber(nanobind::module_& module)
{
  // --- Wrap the CSubscriber class ------------------------------------------------
  nb::class_<CSubscriber>(module, "Subscriber")
    // Define CPublisher class
    .def("__init__", [](CSubscriber* t, nb::str topic_name, const SDataTypeInformation& datatype_info/*, const PubEventCallbackT& event_callback_*/, const Subscriber::Configuration& config_) { new (t) CSubscriber(topic_name.c_str(), datatype_info/*, event_callback_*/, config_); },
      nb::arg("topic_name"),
      nb::arg("data_type_info") = SDataTypeInformation(),
      nb::arg("config") = GetSubscriberConfiguration()
      //nb::arg("event_callback") = nullptr
    )

    .def("set_receive_callback",
      [](CSubscriber& self, nb::object py_callback) {
        // Wrap the Python callback with a lambda that acquires the GIL.
        // Adjust the callback parameters according to the actual signature of ReceiveCallbackT.
        auto wrapped_callback = [py_callback](auto&&... args) {
          try {
            nb::gil_scoped_acquire acquire;
            // Call the Python callback, forwarding the arguments.
            py_callback(std::forward<decltype(args)>(args)...);
          }
          catch (std::exception e)
          {
            std::cout << "Error invoking callback: " << e.what() << std::endl;
          }
        };

        nb::gil_scoped_release release;
        return self.SetReceiveCallback(wrapped_callback);
      },
      nb::arg("callback"))

    .def("remove_receive_callback", &CSubscriber::RemoveReceiveCallback)
    .def("get_publisher_count", &CSubscriber::GetPublisherCount)
    .def("get_topic_name", &CSubscriber::GetTopicName)
    .def("get_topic_id", &CSubscriber::GetTopicId)
    .def("get_data_type_information", &CSubscriber::GetDataTypeInformation)

    .def("__repr__", [](const CSubscriber& pub) {
    return "<Publisher topic='" + pub.GetTopicName() + "' subscribers=" +
      std::to_string(pub.GetPublisherCount()) + ">";
      });
    ;
}