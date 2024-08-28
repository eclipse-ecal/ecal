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
 * @brief  Add ServiceClient class to nanobind module
**/


#include <modules/module_subscriber_config.h>
#include <wrappers/wrapper_subscriber_config.h>

void AddSubscriberConfigStructToModule(nanobind::module_& module)
{
    nanobind::class_<eCAL::Subscriber::SHM::CNBSHMConfiguration>(module, "SHMConfiguration")
        .def(nanobind::init<>())
        .def_rw("enable", &eCAL::Subscriber::SHM::CNBSHMConfiguration::enable);

    nanobind::class_<eCAL::Subscriber::UDP::CNBUDPConfiguration>(module, "UDPConfiguration")
        .def(nanobind::init<>())
        .def_rw("enable", &eCAL::Subscriber::UDP::CNBUDPConfiguration::enable);

    nanobind::class_<eCAL::Subscriber::TCP::CNBTCPConfiguration>(module, "TCPConfiguration")
        .def(nanobind::init<>())
        .def_rw("enable", &eCAL::Subscriber::TCP::CNBTCPConfiguration::enable);

    nanobind::class_<eCAL::Subscriber::CNBSubscriberConfiguration>(module, "SubscriberConfiguration")
        .def(nanobind::init<>())
        .def_rw("shm", &eCAL::Subscriber::CNBSubscriberConfiguration::shm)
        .def_rw("udp", &eCAL::Subscriber::CNBSubscriberConfiguration::udp)
        .def_rw("tcp", &eCAL::Subscriber::CNBSubscriberConfiguration::tcp);
}
