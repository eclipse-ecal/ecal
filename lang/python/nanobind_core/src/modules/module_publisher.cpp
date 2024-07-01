/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2024 Continental Corporation
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
 * @brief  Add publisher class to nanobind module
**/


#include <modules/module_publisher.h>
#include <wrappers/wrapper_publisher.h>

void AddPublisherClassToModule(const nanobind::module_& module) 
{
    // Class and Functions from ecal_publisher.h
    auto Publisher_cls = nanobind::class_<eCAL::CNBPublisher>(module, "Publisher")
        .def(nanobind::init<>())
        .def(nanobind::init<const std::string&>())
        // TODO: we need the constructor with the eCAL::Publisher::Configuration, too
        .def(nanobind::init<const std::string&, const eCAL::CNBDataTypeInformation&>())
        //       .def(nanobind::self = nanobind::self)
        .def("create", nanobind::overload_cast<const std::string&>(&eCAL::CNBPublisher::Create))
        .def("create", nanobind::overload_cast<const std::string&, const eCAL::SDataTypeInformation&, const eCAL::Publisher::Configuration&>(&eCAL::CNBPublisher::Create))
        .def("send", nanobind::overload_cast<const void*, size_t, long long>(&eCAL::CNBPublisher::Send))
        .def("send", nanobind::overload_cast<eCAL::CPayloadWriter&, long long>(&eCAL::CNBPublisher::Send))
        .def("send", nanobind::overload_cast<const std::string&, long long>(&eCAL::CNBPublisher::Send))
        .def("destroy", &eCAL::CNBPublisher::Destroy)
        .def("set_attribute", &eCAL::CNBPublisher::SetAttribute)
        .def("clear_attribute", &eCAL::CNBPublisher::ClearAttribute)
        .def("set_id", &eCAL::CNBPublisher::SetID)
        .def("rem_event_callback", &eCAL::CNBPublisher::RemEventCallback)
        .def("add_event_callback", &eCAL::CNBPublisher::AddEventCallback)
        .def("is_created", &eCAL::CNBPublisher::IsCreated)
        .def("is_subscribed", &eCAL::CNBPublisher::IsSubscribed)
        .def("get_subscriber_count", &eCAL::CNBPublisher::GetSubscriberCount)
        .def("get_topic_name", &eCAL::CNBPublisher::GetTopicName)
        .def("get_datatype_information", &eCAL::CNBPublisher::GetDataTypeInformation)
        .def("dump", &eCAL::CNBPublisher::Dump);
}
