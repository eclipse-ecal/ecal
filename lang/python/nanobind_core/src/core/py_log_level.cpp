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

#include <core/py_log_level.h>
#include <ecal/log_level.h>

namespace nb = nanobind;
using namespace eCAL::Logging;

void AddLogLevel(nanobind::module_& module)
{
  nb::enum_<eLogLevel>(module, "LogLevel")
    .value("NONE", log_level_none)
    .value("ALL", log_level_all)
    .value("INFO", log_level_info)
    .value("WARNING", log_level_warning)
    .value("ERROR", log_level_error)
    .value("FATAL", log_level_fatal)
    .value("DEBUG1", log_level_debug1)
    .value("DEBUG2", log_level_debug2)
    .value("DEBUG3", log_level_debug3)
    .value("DEBUG4", log_level_debug4);
}