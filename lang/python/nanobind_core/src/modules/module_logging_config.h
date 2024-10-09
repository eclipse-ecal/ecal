/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2024 Continental Corporation
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
 * @file   module_datatypeinfo.h
 * @brief  Nanobind module for structs of file config/logging.h
**/

#pragma once

#include <nanobind/nanobind.h>
#include <nanobind/operators.h>
#include <ecal/ecal.h>
#include <nanobind/operators.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/shared_ptr.h>
#include <nanobind/stl/tuple.h>
#include <cstdint>
#include <chrono>
#include <memory>
#include <string>
#include <cstddef>
#include <ecal/ecal_types.h>

/**
 * @brief  Function to Add Nanobind module
 *
 * @param module  The nanobind module variable
**/
void AddLoggingConfigStructToModule(nanobind::module_& module);
