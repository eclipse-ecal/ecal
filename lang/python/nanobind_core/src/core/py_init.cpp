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

#include <core/py_init.h>
#include <ecal/init.h>

void AddInit(nanobind::module_& module)
{
  // Bind constants from eCAL::Init
  module.attr("PUBLISHER") = eCAL::Init::Publisher;
  module.attr("SUBSCRIBER") = eCAL::Init::Subscriber;
  module.attr("SERVICE") = eCAL::Init::Service;
  module.attr("MONITORING") = eCAL::Init::Monitoring;
  module.attr("LOGGING") = eCAL::Init::Logging;
  module.attr("TIMESYNC") = eCAL::Init::TimeSync;
  module.attr("ALL") = eCAL::Init::All;
  module.attr("DEFAULT") = eCAL::Init::Default;
  module.attr("NONE") = eCAL::Init::None;

  // Bind the inline function that creates a default configuration.
  // This function initializes a configuration via Init::Configuration().
  module.def("get_configuration", &eCAL::Init::Configuration,
    "Create a default eCAL configuration by initializing from the current configuration settings");
}
