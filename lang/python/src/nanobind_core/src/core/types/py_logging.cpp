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
 * @file   types/logging.h
 * @brief  eCAL logging struct interface
**/

#include <core/types/py_logging.h>
#include <ecal/types/logging.h>

#include <nanobind/stl/list.h>
#include <nanobind/stl/string.h>

namespace nb = nanobind;
using namespace eCAL::Logging;

void AddTypesLogging(nanobind::module_& module)
{
  // Bind SLogMessage struct
  nb::class_<SLogMessage>(module, "LogMessage")
    .def(nb::init<>())
    .def_rw("time", &SLogMessage::time)
    .def_rw("host_name", &SLogMessage::host_name)
    .def_rw("process_id", &SLogMessage::process_id)
    .def_rw("process_name", &SLogMessage::process_name)
    .def_rw("unit_name", &SLogMessage::unit_name)
    .def_rw("level", &SLogMessage::level)
    .def_rw("content", &SLogMessage::content)
    .def("__repr__", [](const SLogMessage& msg) {
    return "<LogMessage time=" + std::to_string(msg.time) +
      " host='" + msg.host_name +
      "' process_id=" + std::to_string(msg.process_id) +
      " process='" + msg.process_name +
      "' unit='" + msg.unit_name +
      "' level=" + std::to_string(msg.level) +
      " content='" + msg.content + "'>";
      });

  // We are not binding the SLogging struct, because the function directly returns a list of log entries.
  //nb::class_<SLogging>(module, "Logging")
  //  .def(nb::init<>())
  //  .def_rw("log_messages", &SLogging::log_messages);
}