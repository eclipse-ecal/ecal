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
 * @brief  Add subscriber class to nanobind module
**/


#include <modules/module_subscriber.h>
#include <wrappers/wrapper_subscriber.h>

void AddSubscriberClassToModule(const nanobind::module_& module) 
{
	auto Subscriber_cls = nanobind::class_<eCAL::CNBSubscriber>(module, "Subscriber")
		.def(nanobind::init<>())
		.def(nanobind::init<const std::string&>())
		.def(nanobind::init<const std::string&, const eCAL::CNBDataTypeInformation&>())
		.def("receive", &eCAL::CNBSubscriber::Receive)
		.def("create", nanobind::overload_cast<const std::string&>(&eCAL::CNBSubscriber::Create))
		.def("create", nanobind::overload_cast<const std::string&, const eCAL::SDataTypeInformation&, const eCAL::Subscriber::Configuration&>(&eCAL::CNBSubscriber::Create))
		.def("destroy", &eCAL::CNBSubscriber::Destroy)
		.def("set_attribute", &eCAL::CNBSubscriber::SetAttribute)
		.def("clear_attribute", &eCAL::CNBSubscriber::ClearAttribute)
		.def("set_id", &eCAL::CNBSubscriber::SetID)
		.def("receive_buffer", &eCAL::CNBSubscriber::ReceiveBuffer)
		.def("add_receive_callback", &eCAL::CNBSubscriber::WrapAddRecCB)
		.def("rem_receive_callback", &eCAL::CNBSubscriber::RemReceiveCallback)
		.def("rem_event_callback", &eCAL::CNBSubscriber::RemEventCallback)
		.def("add_event_callback", &eCAL::CNBSubscriber::AddEventCallback)
		.def("is_created", &eCAL::CNBSubscriber::IsCreated)
		.def("get_publisher_count", &eCAL::CNBSubscriber::GetPublisherCount)
		.def("get_topic_name", &eCAL::CNBSubscriber::GetTopicName)
		.def("get_datatype_information", &eCAL::CNBSubscriber::GetDataTypeInformation)
		.def("dump", &eCAL::CNBSubscriber::Dump);
}
