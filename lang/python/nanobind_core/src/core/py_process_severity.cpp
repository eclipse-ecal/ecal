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

#include <core/py_process_severity.h>
#include <ecal/process_severity.h>

namespace nb = nanobind;
using namespace nb::literals;

void AddProcessSeverity(nanobind::module_& m)
{
  nb::module_ m_process = m.def_submodule("process", "eCAL Process related functionality");

  nb::enum_<eCAL::Process::eSeverity>(m_process, "Severity")
    .value("UNKNOWN", eCAL::Process::eSeverity::unknown)
    .value("HEALTHY", eCAL::Process::eSeverity::healthy)
    .value("WARNING", eCAL::Process::eSeverity::warning)
    .value("CRITICAL", eCAL::Process::eSeverity::critical)
    .value("FAILED", eCAL::Process::eSeverity::failed);

  nb::enum_<eCAL::Process::eSeverityLevel>(m_process, "SeverityLevel")
    .value("LEVEL1", eCAL::Process::eSeverityLevel::level1)
    .value("LEVEL2", eCAL::Process::eSeverityLevel::level2)
    .value("LEVEL3", eCAL::Process::eSeverityLevel::level3)
    .value("LEVEL4", eCAL::Process::eSeverityLevel::level4)
    .value("LEVEL5", eCAL::Process::eSeverityLevel::level5);
}