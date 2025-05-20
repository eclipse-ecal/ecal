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


#include <modules/module_monitoring_config.h>
#include <ecal/config/monitoring.h>

void AddMonitoringConfigStructToModule(nanobind::module_& m_Monitoring)
{
    nanobind::class_<eCAL::Monitoring::Configuration>(m_Monitoring, "MonitoringConfiguration")
        .def(nanobind::init<>()) 
        .def_rw("filter_excl", &eCAL::Monitoring::Configuration::filter_excl)
        .def_rw("filter_incl", &eCAL::Monitoring::Configuration::filter_incl);
}

