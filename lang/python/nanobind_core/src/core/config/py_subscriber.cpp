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

#include <core/config/py_subscriber.h>
#include <ecal/config/subscriber.h>

namespace nb = nanobind;
using namespace eCAL::Subscriber;

void AddConfigSubscriber(nanobind::module_& module)
{
  // Bind Subscriber::Layer::SHM::Configuration struct
  nb::class_<Layer::SHM::Configuration>(module, "SubscriberLayerSHMConfiguration")
    .def(nb::init<>()) // Default constructor
    .def_rw("enable", &Layer::SHM::Configuration::enable, "Enable SHM layer (Default: true)");

  // Bind Subscriber::Layer::UDP::Configuration struct
  nb::class_<Layer::UDP::Configuration>(module, "SubscriberLayerUDPConfiguration")
    .def(nb::init<>()) // Default constructor
    .def_rw("enable", &Layer::UDP::Configuration::enable, "Enable UDP layer (Default: true)");

  // Bind Subscriber::Layer::TCP::Configuration struct
  nb::class_<Layer::TCP::Configuration>(module, "SubscriberLayerTCPConfiguration")
    .def(nb::init<>()) // Default constructor
    .def_rw("enable", &Layer::TCP::Configuration::enable, "Enable TCP layer (Default: false)");

  // Bind Subscriber::Layer::Configuration struct
  nb::class_<Layer::Configuration>(module, "SubscriberLayerConfiguration")
    .def(nb::init<>()) // Default constructor
    .def_rw("shm", &Layer::Configuration::shm, "Shared Memory (SHM) layer configuration")
    .def_rw("udp", &Layer::Configuration::udp, "UDP layer configuration")
    .def_rw("tcp", &Layer::Configuration::tcp, "TCP layer configuration");

  // Bind Subscriber::Configuration struct
  nb::class_<Configuration>(module, "SubscriberConfiguration")
    .def(nb::init<>()) // Default constructor
    .def_rw("layer", &Configuration::layer, "Layer configuration for subscriber")
    .def_rw("drop_out_of_order_messages", &Configuration::drop_out_of_order_messages,
      "Enable dropping of out-of-order messages (Default: true)");
}
