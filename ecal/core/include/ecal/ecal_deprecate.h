/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2019 Continental Corporation
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
 * @file   ecal_deprecate.h
 * @brief  eCAL function / variable deprecation macros
**/

#pragma once

#include <ecal/ecal_defs.h>

#if ECAL_VERSION_INTEGER >= ECAL_VERSION_CALCULATE(5, 4, 0)
#define ECAL_DEPRECATE_SINCE_5_4(__message__) [[deprecated(__message__)]]
#else 
#define ECAL_DEPRECATE_SINCE_5_4(__message__)
#endif


#if ECAL_VERSION_INTEGER >= ECAL_VERSION_CALCULATE(5, 10, 0)
#define ECAL_DEPRECATE_SINCE_5_10(__message__) [[deprecated(__message__)]]
#else 
#define ECAL_DEPRECATE_SINCE_5_10(__message__)
#endif

#if ECAL_VERSION_INTEGER >= ECAL_VERSION_CALCULATE(5, 11, 0)
#define ECAL_DEPRECATE_SINCE_5_11(__message__) [[deprecated(__message__)]]
#else 
#define ECAL_DEPRECATE_SINCE_5_11(__message__)
#endif

#if ECAL_VERSION_INTEGER >= ECAL_VERSION_CALCULATE(5, 12, 0)
#define ECAL_DEPRECATE_SINCE_5_12(__message__) [[deprecated(__message__)]]
#else 
#define ECAL_DEPRECATE_SINCE_5_12(__message__)
#endif

#if ECAL_VERSION_INTEGER >= ECAL_VERSION_CALCULATE(5, 13, 0)
#define ECAL_DEPRECATE_SINCE_5_13(__message__) [[deprecated(__message__)]]
#else 
#define ECAL_DEPRECATE_SINCE_5_13(__message__)
#endif
