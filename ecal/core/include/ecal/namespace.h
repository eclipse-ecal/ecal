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
 * @file   namespace.h
 * @brief  eCAL namespace definitions (for switching between v5 / v6 namespaces
**/

#pragma once

#if ECAL_CORE_NAMESPACE_V5_INLINE
#define ECAL_CORE_NAMESPACE_V5 inline namespace v5
#define ECAL_CORE_NAMESPACE_V6 namespace v6
#else
#define ECAL_CORE_NAMESPACE_V5 namespace v5
#define ECAL_CORE_NAMESPACE_V6 inline namespace v6
#endif

