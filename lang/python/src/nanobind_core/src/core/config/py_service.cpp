/* =========================== LICENSE =================================
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
 * =========================== LICENSE =================================
 */

#include <core/config/py_service.h>
#include <ecal/config/service.h>

namespace nb = nanobind;
using namespace eCAL::Service;

void AddConfigService(nanobind::module_& module)
{
  // Bind Service::Configuration struct
  nb::class_<Configuration>(module, "ServiceConfiguration")
    .def(nb::init<>()) // Default constructor
    .def_rw("server_client_id_timeout_ms", &Configuration::server_client_id_timeout_ms,
      "Timeout in ms to wait for the client to send its entity ID after connecting. "
      "Set to 0 to disable waiting (old behavior). Default: 1000");
}
