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
 * @brief  Add Publisher config structs to nanobind module
**/


#include <modules/module_publisher_config.h>
#include <ecal/config/publisher.h>

namespace nb = nanobind;

void AddPublisherConfigStructToModule(nanobind::module_& module)
{
    nb::module_ m_Publisher = module.def_submodule("publisher");
    nb::module_ m_Layer = m_Publisher.def_submodule("layer");

    // SHM Configuration Binding
    nb::class_<eCAL::Publisher::Layer::SHM::Configuration>(m_Layer, "SHM_Configuration")
        .def(nb::init<>())
        .def_rw("enable", &eCAL::Publisher::Layer::SHM::Configuration::enable)
        .def_rw("zero_copy_mode", &eCAL::Publisher::Layer::SHM::Configuration::zero_copy_mode)
        .def_rw("acknowledge_timeout_ms", &eCAL::Publisher::Layer::SHM::Configuration::acknowledge_timeout_ms)
        .def_rw("memfile_buffer_count", &eCAL::Publisher::Layer::SHM::Configuration::memfile_buffer_count)
        .def_rw("memfile_min_size_bytes", &eCAL::Publisher::Layer::SHM::Configuration::memfile_min_size_bytes)
        .def_rw("memfile_reserve_percent", &eCAL::Publisher::Layer::SHM::Configuration::memfile_reserve_percent);

    // UDP Configuration Binding
    nb::class_<eCAL::Publisher::Layer::UDP::Configuration>(m_Layer, "UDP_Configuration")
        .def(nb::init<>())
        .def_rw("enable", &eCAL::Publisher::Layer::UDP::Configuration::enable);

    // TCP Configuration Binding
    nb::class_<eCAL::Publisher::Layer::TCP::Configuration>(m_Layer, "TCP_Configuration")
        .def(nb::init<>())
        .def_rw("enable", &eCAL::Publisher::Layer::TCP::Configuration::enable);

    // Layer Configuration Binding
    nb::class_<eCAL::Publisher::Layer::Configuration>(m_Layer, "LayerConfiguration")
        .def(nb::init<>())
        .def_rw("shm", &eCAL::Publisher::Layer::Configuration::shm)
        .def_rw("udp", &eCAL::Publisher::Layer::Configuration::udp)
        .def_rw("tcp", &eCAL::Publisher::Layer::Configuration::tcp);

    // Publisher Configuration Binding
    nb::class_<eCAL::Publisher::Configuration>(m_Publisher, "PublisherConfiguration")
        .def(nb::init<>())
        .def_rw("layer", &eCAL::Publisher::Configuration::layer)
        .def_rw("layer_priority_local", &eCAL::Publisher::Configuration::layer_priority_local)
        .def_rw("layer_priority_remote", &eCAL::Publisher::Configuration::layer_priority_remote)
        .def_rw("share_topic_type", &eCAL::Publisher::Configuration::share_topic_type)
        .def_rw("share_topic_description", &eCAL::Publisher::Configuration::share_topic_description);

}