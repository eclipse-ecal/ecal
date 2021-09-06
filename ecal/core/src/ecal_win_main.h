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
 * @brief  eCAL windows includes
**/

#pragma once

#if defined(_MSC_VER) && defined(__clang__) && !defined(CINTERFACE)
#define CINTERFACE
#endif

#if !defined(NOMINMAX)
  #define NOMINMAX
#endif // !defined(NOMINMAX)

#if !defined(WIN32_LEAN_AND_CLEAN)
  #define WIN32_LEAN_AND_CLEAN
#endif // !defined(WIN32_LEAN_AND_CLEAN)

#include <windows.h>
#include <direct.h>
#include <psapi.h>
#include <mmsystem.h>

#undef CINTERFACE
