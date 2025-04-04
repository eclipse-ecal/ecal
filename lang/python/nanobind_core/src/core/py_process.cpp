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

#include <core/py_process.h>
#include <ecal/process.h>

#include <nanobind/stl/string.h>

namespace nb = nanobind;

void AddProcess(nanobind::module_& m)
{
  nb::module_ m_process = m.def_submodule("process", "eCAL Process related functionality");

  // Wrap GetUnitName
  m_process.def("get_unit_name", &eCAL::Process::GetUnitName,
    "Returns the unit name as defined via eCAL::Initialize");

  // Wrap SetState
  m_process.def("set_state",
    &eCAL::Process::SetState,
    nb::arg("severity"),
    nb::arg("level"),
    nb::arg("info"),
    "Set the current process state info (severity, level, info)");
}