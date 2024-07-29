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


#include <modules/module_publisher_config.h>
#include <wrappers/wrapper_publisher_config.h>

void AddPublisherConfigStructToModule(nanobind::module_& module)
{
    nanobind::class_<eCAL::Publisher::SHM::CNBSHMConfiguration>(module, "SHMConfiguration")
        .def(nanobind::init<>())
        .def_rw("enable", &eCAL::Publisher::SHM::CNBSHMConfiguration::enable)
        .def_rw("zero_copy_mode", &eCAL::Publisher::SHM::CNBSHMConfiguration::zero_copy_mode)
        .def_rw("acknowledge_timeout_ms", &eCAL::Publisher::SHM::CNBSHMConfiguration::acknowledge_timeout_ms)
        .def_rw("memfile_min_size_bytes", &eCAL::Publisher::SHM::CNBSHMConfiguration::memfile_min_size_bytes)
        .def_rw("memfile_reserve_percent", &eCAL::Publisher::SHM::CNBSHMConfiguration::memfile_reserve_percent)
        .def_rw("memfile_buffer_count", &eCAL::Publisher::SHM::CNBSHMConfiguration::memfile_buffer_count);

    nanobind::class_<eCAL::Publisher::UDP::CNBUDPConfiguration>(module, "UDPConfiguration")
        .def(nanobind::init<>())
        .def_rw("enable", &eCAL::Publisher::UDP::CNBUDPConfiguration::enable)
        .def_rw("loopback", &eCAL::Publisher::UDP::CNBUDPConfiguration::loopback)
        .def_rw("sndbuf_size_bytes", &eCAL::Publisher::UDP::CNBUDPConfiguration::sndbuf_size_bytes);

    nanobind::class_<eCAL::Publisher::TCP::CNBTCPConfiguration>(module, "TCPConfiguration")
        .def(nanobind::init<>())
        .def_rw("enable", &eCAL::Publisher::TCP::CNBTCPConfiguration::enable);

    nanobind::class_<eCAL::Publisher::CNBPublisherConfiguration>(module, "PublisherConfiguration")
        .def(nanobind::init<>())
        .def_rw("shm", &eCAL::Publisher::CNBPublisherConfiguration::shm)
        .def_rw("udp", &eCAL::Publisher::CNBPublisherConfiguration::udp)
        .def_rw("tcp", &eCAL::Publisher::CNBPublisherConfiguration::tcp)
        .def_rw("share_topic_type", &eCAL::Publisher::CNBPublisherConfiguration::share_topic_type)
        .def_rw("share_topic_description", &eCAL::Publisher::CNBPublisherConfiguration::share_topic_description);
}