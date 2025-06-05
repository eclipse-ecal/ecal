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

#include <core/py_util.h>
#include <ecal/util.h>

#include <nanobind/stl/string.h>

namespace nb = nanobind;

void AddUtil(nanobind::module_& m)
{
  // Get eCAL data directory
  m.def("get_ecal_data_dir", []() {
    return eCAL::Util::GeteCALDataDir();
    }, "Retrieve the eCAL data path, where eCAL will search for configuration files.");

  // Get eCAL log directory
  m.def("get_ecal_log_dir", []() {
    return eCAL::Util::GeteCALLogDir();
    }, "Retrieve the eCAL standard logging path.");

  // Shutdown by unit name
  m.def("shutdown_process_by_name", [](const std::string& process_name) {
    nb::gil_scoped_release release;
    eCAL::Util::ShutdownProcess(process_name);
    }, "Shutdown a process by its unit name");

  // Shutdown by process ID
  m.def("shutdown_process_by_pid", [](int process_id) {
    nb::gil_scoped_release release;
    eCAL::Util::ShutdownProcess(process_id);
    }, "Shutdown a process by its process ID");

  // Shutdown all processes
  m.def("shutdown_all_processes", []() {
    nb::gil_scoped_release release;
    eCAL::Util::ShutdownProcesses();
    }, "Shutdown all local user processes");
}