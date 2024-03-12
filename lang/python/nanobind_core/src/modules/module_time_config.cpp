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


#include <modules/module_time_config.h>
#include <ecal/config/time.h>

void AddTimeConfigStructToModule(nanobind::module_& module)
{   
    nanobind::class_<eCAL::Time::Configuration>(module, "TimeConfiguration")
        .def(nanobind::init<>())  // Default constructor
        .def_rw("timesync_module_rt", &eCAL::Time::Configuration::timesync_module_rt)
        .def_rw("timesync_module_replay", &eCAL::Time::Configuration::timesync_module_replay);
}

