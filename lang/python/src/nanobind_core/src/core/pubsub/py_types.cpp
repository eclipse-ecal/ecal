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

#include <core/pubsub/py_types.h>
#include <ecal/pubsub/types.h>

#include <nanobind/stl/string.h>

#include <sstream>

namespace nb = nanobind;
using namespace eCAL;

void AddPubsubTypes(nanobind::module_& module)
{

  //-------------------------------------------------------------------------
  // Bind ePublisherEvent enum
  nb::enum_<eCAL::ePublisherEvent>(module, "PublisherEvent")
    .value("NONE", eCAL::ePublisherEvent::none)
    .value("CONNECTED", eCAL::ePublisherEvent::connected)
    .value("DISCONNECTED", eCAL::ePublisherEvent::disconnected)
    .value("DROPPED", eCAL::ePublisherEvent::dropped);

    //-------------------------------------------------------------------------
    // Bind eSubscriberEvent enum
    nb::enum_<eCAL::eSubscriberEvent>(module, "SubscriberEvent")
    .value("NONE", eCAL::eSubscriberEvent::none)
    .value("CONNECTED", eCAL::eSubscriberEvent::connected)
    .value("DISCONNECTED", eCAL::eSubscriberEvent::disconnected)
    .value("DROPPED", eCAL::eSubscriberEvent::dropped);

  //-------------------------------------------------------------------------
  // Bind STopicId struct
  nb::class_<eCAL::STopicId>(module, "TopicId")
    .def(nb::init<>())
    .def_rw("topic_id", &eCAL::STopicId::topic_id, "Unique topic identifier (SEntityId)")
    .def_rw("topic_name", &eCAL::STopicId::topic_name, "Topic name as a string")
    .def("__eq__", &eCAL::STopicId::operator==)
    .def("__lt__", &eCAL::STopicId::operator<)
    .def("__repr__",
      [](const eCAL::STopicId& self) {
        std::ostringstream oss;
        oss << self;  // uses the provided operator<< overload
        return oss.str();
      },
      "Return the string representation of TopicId")
    .def("__hash__", 
      [](const eCAL::STopicId& self) {
        return std::hash<eCAL::STopicId>{}(self);
      },
      "Hash calculation");

      //-------------------------------------------------------------------------
      // Bind SReceiveCallbackData struct
      // This data will always be set by C++, hence it's not changeable
      nb::class_<eCAL::SReceiveCallbackData>(module, "ReceiveCallbackData")
        .def(nb::init<>())
        .def_prop_ro("buffer", [](const SReceiveCallbackData& data) -> nb::bytes {
          // Cast the void* to const char* and create a nb::bytes from it.
          return nb::bytes(static_cast<const char*>(data.buffer), data.buffer_size);
          }, "Payload buffer as Python bytes")
        .def_ro("send_timestamp", &eCAL::SReceiveCallbackData::send_timestamp, "Publisher send timestamp (µs)")
        .def_ro("send_clock", &eCAL::SReceiveCallbackData::send_clock, "Publisher send clock counter");

      //-------------------------------------------------------------------------
      // Bind SPubEventCallbackData struct
      nb::class_<eCAL::SPubEventCallbackData>(module, "PubEventCallbackData")
        .def(nb::init<>())
        .def_ro("event_type", &eCAL::SPubEventCallbackData::event_type, "Publisher event type")
        .def_ro("event_time", &eCAL::SPubEventCallbackData::event_time, "Event time in µs (eCAL time)")
        .def_ro("subscriber_datatype", &eCAL::SPubEventCallbackData::subscriber_datatype,
          "Datatype description of the connected subscriber");

      //-------------------------------------------------------------------------
      // Bind SSubEventCallbackData struct
      nb::class_<eCAL::SSubEventCallbackData>(module, "SubEventCallbackData")
        .def(nb::init<>())
        .def_ro("event_type", &eCAL::SSubEventCallbackData::event_type, "Subscriber event type")
        .def_ro("event_time", &eCAL::SSubEventCallbackData::event_time, "Event time in µs (eCAL time)")
        .def_ro("publisher_datatype", &eCAL::SSubEventCallbackData::publisher_datatype,
          "Datatype description of the connected publisher");

      //-------------------------------------------------------------------------
      // Note on callback types:
      //
      // The callback types (e.g. ReceiveCallbackT, PubEventCallbackT, SubEventCallbackT)
      // are defined as std::function objects. Nanobind automatically converts Python callables
      // to std::function if a C++ function or method takes one as a parameter. Therefore,
      // explicit binding of these callback types is usually not necessary.
}