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

#include <core/config/py_publisher.h>
#include <ecal/config/publisher.h>
#include <nanobind/stl/vector.h>

namespace nb = nanobind;
using namespace eCAL::Publisher;

void AddConfigPublisher(nanobind::module_& module)
{
  // Bind Publisher::Layer::SHM::Configuration struct
  nb::class_<Layer::SHM::Configuration>(module, "PublisherLayerSHMConfiguration")
    .def(nb::init<>()) // Default constructor
    .def_rw("enable", &Layer::SHM::Configuration::enable, "Enable SHM layer")
    .def_rw("zero_copy_mode", &Layer::SHM::Configuration::zero_copy_mode, "Enable zero copy shared memory mode")
    .def_rw("acknowledge_timeout_ms", &Layer::SHM::Configuration::acknowledge_timeout_ms,
      "Timeout for subscriber acknowledge event (0 = no handshake)")
    .def_rw("memfile_buffer_count", &Layer::SHM::Configuration::memfile_buffer_count,
      "Maximum number of memory buffers (must be >1)")
    .def_rw("memfile_min_size_bytes", &Layer::SHM::Configuration::memfile_min_size_bytes,
      "Default memory file size for new publishers")
    .def_rw("memfile_reserve_percent", &Layer::SHM::Configuration::memfile_reserve_percent,
      "Dynamic memory file size reserve before recreation");

  // Bind Publisher::Layer::UDP::Configuration struct
  nb::class_<Layer::UDP::Configuration>(module, "PublisherLayerUDPConfiguration")
    .def(nb::init<>()) // Default constructor
    .def_rw("enable", &Layer::UDP::Configuration::enable, "Enable UDP layer");

  // Bind Publisher::Layer::TCP::Configuration struct
  nb::class_<Layer::TCP::Configuration>(module, "PublisherLayerTCPConfiguration")
    .def(nb::init<>()) // Default constructor
    .def_rw("enable", &Layer::TCP::Configuration::enable, "Enable TCP layer");

  // Bind Publisher::Layer::Configuration struct
  nb::class_<Layer::Configuration>(module, "PublisherLayerConfiguration")
    .def(nb::init<>()) // Default constructor
    .def_rw("shm", &Layer::Configuration::shm, "Shared Memory (SHM) layer configuration")
    .def_rw("udp", &Layer::Configuration::udp, "UDP layer configuration")
    .def_rw("tcp", &Layer::Configuration::tcp, "TCP layer configuration");

  // Bind Publisher::Configuration struct
  nb::class_<Configuration>(module, "PublisherConfiguration")
    .def(nb::init<>()) // Default constructor
    .def_rw("layer", &Configuration::layer, "Layer configuration")
    .def_rw("layer_priority_local", &Configuration::layer_priority_local,
      "Transport layer priority for local communication")
    .def_rw("layer_priority_remote", &Configuration::layer_priority_remote,
      "Transport layer priority for remote communication");
}
