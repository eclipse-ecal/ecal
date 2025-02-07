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

namespace nb = nanobind;
using namespace eCAL;

void AddPubsubPublisher(nanobind::module_& module)
{
  // Define static default values for correct nb::arg_v() usage
  static SDataTypeInformation default_data_type_info;
  static Publisher::Configuration default_publisher_config;

  // Define CPublisher class
  nb::class_<CPublisher>(module, "Publisher")
    .def(nb::init<const std::string&, const SDataTypeInformation&, const Publisher::Configuration&>(),
      nb::arg("topic_name"),
      // need to figure out how arg_v works
      //nb::arg_v("data_type_info", nb::cast(SDataTypeInformation())),
      //nb::arg_v("config", default_publisher_config, "Default publisher configuration"),
      nb::arg("data_type_info"),
      nb::arg("config"),
      "Initialize a Publisher with a topic name, data type, and optional configuration.")

    // Send function for Python bytes
    .def("send", [](CPublisher& pub, nb::bytes payload, long long time) {
    const char* data = payload.c_str();
    size_t length = payload.size();
    return pub.Send(data, length, time);
      }, nb::arg("payload"), nb::arg("time") = CPublisher::DEFAULT_TIME_ARGUMENT,
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
