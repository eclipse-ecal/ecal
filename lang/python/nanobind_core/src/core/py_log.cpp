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
 * @file   log.h
 * @brief  eCAL logging interface
**/

#include <core/py_log.h>
#include <ecal/log.h>

#include <optional>

#include <nanobind/stl/optional.h>
#include <nanobind/stl/list.h>
#include <nanobind/stl/string.h>

namespace nb = nanobind;
using namespace eCAL::Logging;

void AddLog(nanobind::module_& module)
{
  nb::module_ m_logging = module.def_submodule("logging", "eCAL Logging related functionality");

  m_logging.def("log", &Log, "Log a message", nb::arg("level"), nb::arg("message"));
  
  m_logging.def("get_logging", []() -> std::optional<std::list<SLogMessage>> {
    SLogging log_output;
    if (GetLogging(log_output)) return log_output.log_messages;
    return std::nullopt;
    }, "Get logging as Logging | None");
}


