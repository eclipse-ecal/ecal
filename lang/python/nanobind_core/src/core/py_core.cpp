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

#include <core/py_core.h>

// ecal_core_bindings.cpp

#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>

// Include the header file that declares the following functions and types.
#include <ecal/core.h>

namespace nb = nanobind;
using namespace nanobind::literals;  // Bring the _a literal operator into scope

void AddCore(nb::module_& m)
{
  //-------------------------------------------------------------------------
  // Version related functions
  //-------------------------------------------------------------------------
  m.def("get_version_string",
    &eCAL::GetVersionString,
    "Get the full eCAL version string.");

  m.def("get_version_date_string",
    &eCAL::GetVersionDateString,
    "Get the full eCAL version date string.");

  m.def("get_version",
    &eCAL::GetVersion,
    "Get the eCAL version as separated integer values (SVersion).");

  //-------------------------------------------------------------------------
  // Initialize functions
  //-------------------------------------------------------------------------
  // Overload 1: Initialize using unit name and component flags.
  m.def("initialize",
    nb::overload_cast<const std::string&, unsigned int>(&eCAL::Initialize),
    "Initialize the eCAL API.",
    "unit_name"_a = "", "components"_a = eCAL::Init::Default);

  // Overload 2: Initialize using a configuration object, unit name and component flags.
  m.def("initialize",
    nb::overload_cast<eCAL::Configuration&, const std::string&, unsigned int>(&eCAL::Initialize),
    "Initialize the eCAL API with a configuration object.",
    "config"_a, "unit_name"_a = "", "components"_a = eCAL::Init::Default);

  //-------------------------------------------------------------------------
  // Finalize
  //-------------------------------------------------------------------------
  m.def("finalize",
    &eCAL::Finalize,
    "Finalize the eCAL API.");

  //-------------------------------------------------------------------------
  // IsInitialized functions
  //-------------------------------------------------------------------------
  // Overload 1: Check overall initialization state.
  m.def("is_initialized",
    nb::overload_cast<>(&eCAL::IsInitialized),
    "Check if eCAL is initialized.");

  // Overload 2: Check initialization state for a specific component.
  m.def("is_initialized_component",
    nb::overload_cast<unsigned int>(&eCAL::IsInitialized),
    "Check if a specific eCAL component is initialized.",
    "component"_a);

  //-------------------------------------------------------------------------
  // Process state
  //-------------------------------------------------------------------------
  m.def("ok",
    &eCAL::Ok,
    "Return True if eCAL is in proper state.");
}

