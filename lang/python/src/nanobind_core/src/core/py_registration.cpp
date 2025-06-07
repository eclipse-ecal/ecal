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

#include <core/py_registration.h>
#include <ecal/registration.h>

#include <nanobind/stl/string.h>
#include <nanobind/stl/set.h>
#include <nanobind/stl/pair.h>
#include <nanobind/stl/optional.h>

#include <helper/make_gil_safe_shared.h>

namespace nb = nanobind;

struct PyCallbackToken {
  explicit PyCallbackToken(std::size_t val) : value(val) {}
  std::size_t value;

  bool operator==(const PyCallbackToken& other) const {
    return value == other.value;
  }

  std::string repr() const {
    return "<CallbackToken " + std::to_string(value) + ">";
  }
};

void AddRegistration(nanobind::module_& m)
{
  using namespace eCAL::Registration;

  auto m_registration = m.def_submodule("registration", "eCAL Registration API.");

  nb::class_<PyCallbackToken>(m_registration, "CallbackToken", "Opaque handle used to unregister a callback.")
    .def("__eq__", &PyCallbackToken::operator==)
    .def("__repr__", &PyCallbackToken::repr);

  nb::enum_<RegistrationEventType>(m_registration, "RegistrationEventType", "Type of registration event (e.g., entity created or deleted).")
    .value("NEW_ENTITY", RegistrationEventType::new_entity, "A new entity was registered.")
    .value("DELETED_ENTITY", RegistrationEventType::deleted_entity, "An existing entity was unregistered.");

  nb::class_<SServiceMethod>(m_registration, "ServiceMethod", "Combination of service and method name.")
    .def(nb::init<>())
    .def_rw("service_name", &SServiceMethod::service_name, "Name of the service.")
    .def_rw("method_name", &SServiceMethod::method_name, "Name of the method.")
    .def("__lt__", &SServiceMethod::operator<)
    .def("__repr__", [](const SServiceMethod& sm) {
    return "<ServiceMethod '" + sm.service_name + "::" + sm.method_name + "'>";
      });

  m_registration.def("get_publisher_ids", []() -> nb::object {
    std::set<eCAL::STopicId> ids;
    return GetPublisherIDs(ids) ? nb::cast(ids) : nb::none();
    }, "Get all known publisher topic IDs.");

  m_registration.def("get_subscriber_ids", []() -> nb::object {
    std::set<eCAL::STopicId> ids;
    return GetSubscriberIDs(ids) ? nb::cast(ids) : nb::none();
    }, "Get all known subscriber topic IDs.");

  m_registration.def("get_server_ids", []() -> nb::object {
    std::set<eCAL::SServiceId> ids;
    return GetServerIDs(ids) ? nb::cast(ids) : nb::none();
    }, "Get all known server service IDs.");

  m_registration.def("get_client_ids", []() -> nb::object {
    std::set<eCAL::SServiceId> ids;
    return GetClientIDs(ids) ? nb::cast(ids) : nb::none();
    }, "Get all known client service IDs.");

  m_registration.def("get_published_topic_names", []() -> nb::object {
    std::set<std::string> names;
    return GetPublishedTopicNames(names) ? nb::cast(names) : nb::none();
    }, "Convenience: Get all topic names currently being published.");

  m_registration.def("get_subscribed_topic_names", []() -> nb::object {
    std::set<std::string> names;
    return GetSubscribedTopicNames(names) ? nb::cast(names) : nb::none();
    }, "Convenience: Get all topic names currently being subscribed to.");

  // TODO: Should we wrap this function or should we deprecate it on C++ side
  m_registration.def("get_server_method_names", []() -> nb::object {
    std::set<SServiceMethod> methods;
    return GetServerMethodNames(methods) ? nb::cast(methods) : nb::none();
    }, "Get all service/method name pairs of all eCAL servers.");

  // TODO: Should we wrap this function or should we deprecate it on C++ side
  m_registration.def("get_client_method_names", []() -> nb::object {
    std::set<SServiceMethod> methods;
    return GetClientMethodNames(methods) ? nb::cast(methods) : nb::none();
    }, "Get all service/method name pairs of all eCAL clients.");

  m_registration.def("get_publisher_info", [](const eCAL::STopicId& id) -> nb::object {
    eCAL::SDataTypeInformation info;
    return GetPublisherInfo(id, info) ? nb::cast(info) : nb::none();
    }, nb::arg("topic_id"), "Get datatype info for a specific publisher topic ID.");

  m_registration.def("get_subscriber_info", [](const eCAL::STopicId& id) -> nb::object {
    eCAL::SDataTypeInformation info;
    return GetSubscriberInfo(id, info) ? nb::cast(info) : nb::none();
    }, nb::arg("topic_id"), "Get datatype info for a specific subscriber topic ID.");

  m_registration.def("get_server_info", [](const eCAL::SServiceId& id) -> nb::object {
    eCAL::ServiceMethodInformationSetT methods;
    return GetServerInfo(id, methods) ? nb::cast(methods) : nb::none();
    }, nb::arg("server_id"), "Get method info for a specific server by ID.");

  m_registration.def("get_client_info", [](const eCAL::SServiceId& id) -> nb::object {
    eCAL:: ServiceMethodInformationSetT methods;
    return GetClientInfo(id, methods) ? nb::cast(methods) : nb::none();
    }, nb::arg("client_id"), "Get method info for a specific client by ID.");

  m_registration.def("add_publisher_event_callback", [](const nb::callable& py_cb) -> PyCallbackToken {
    auto cb_ptr = make_gil_safe_shared<nb::callable>(py_cb);
    TopicEventCallbackT cb = [cb_ptr](const eCAL::STopicId& id, RegistrationEventType type) {
      try {
        nb::gil_scoped_acquire acquire;
        (*cb_ptr)(id, type);
      }
      catch (const std::exception& e) {
        std::cerr << "Error in publisher callback: " << e.what() << std::endl;
      }
    };
    nb::gil_scoped_release release;
    return PyCallbackToken(AddPublisherEventCallback(cb));
    }, nb::arg("callback"), "Register callback for new or removed publishers. Returns a token.");

  m_registration.def("remove_publisher_event_callback", [](const PyCallbackToken& token) {
    RemPublisherEventCallback(token.value);
    }, nb::arg("token"), "Unregister a publisher event callback using its token.");

  m_registration.def("add_subscriber_event_callback", [](const nb::callable& py_cb) -> PyCallbackToken {
    auto cb_ptr = make_gil_safe_shared<nb::callable>(py_cb);
    TopicEventCallbackT cb = [cb_ptr](const eCAL::STopicId& id, RegistrationEventType type) {
      try {
        nb::gil_scoped_acquire acquire;
        (*cb_ptr)(id, type);
      }
      catch (const std::exception& e) {
        std::cerr << "Error in subscriber callback: " << e.what() << std::endl;
      }
    };
    nb::gil_scoped_release release;
    return PyCallbackToken(AddSubscriberEventCallback(cb));
    }, nb::arg("callback"), "Register callback for new or removed subscribers. Returns a token.");

  m_registration.def("remove_subscriber_event_callback", [](const PyCallbackToken& token) {
    RemSubscriberEventCallback(token.value);
    }, nb::arg("token"), "Unregister a subscriber event callback using its token.");
}