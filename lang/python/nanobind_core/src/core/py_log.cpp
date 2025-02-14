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
#include <nanobind/stl/string.h>

namespace nb = nanobind;
using namespace eCAL::Logging;

void AddLog(nanobind::module_& module)
{
  module.def("log", &Log, "Log a message", nb::arg("level"), nb::arg("message"));
  
  module.def("get_logging", []() -> std::optional<SLogging> {
    SLogging log_output;
    if (GetLogging(log_output)) return log_output;
    return std::nullopt;
    }, "Get logging as Logging | None");
}


