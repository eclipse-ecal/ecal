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
 * @file   eh5_meas.h
 * @brief  eCALHDF5 measurement class
**/

#pragma once

#include <cstdint>
#include <functional>
#include <set>
#include <string>
#include <memory>

#include "eh5_types.h"

#if defined(ECAL_EH5_NO_DEPRECATION_WARNINGS)
#define ECAL_EH5_DEPRECATE(__message__)                             //!< Don't print deprecation warnigns
#else 
#define ECAL_EH5_DEPRECATE(__message__) [[deprecated(__message__)]] //!< Deprecate the following function 
#endif

#include "eh5_meas_api_v2.h"
#include "eh5_meas_api_v3.h"
