/* ========================= eCAL LICENSE =================================
 *
 * Copyright 2026 AUMOVIO and subsidiaries. All rights reserved.
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

// test_dynamic_library_plugin_api.h
#pragma once

#include <cstdint>

#if defined(_WIN32)
#define TEST_DYNLIB_EXPORT __declspec(dllexport)
#else
#define TEST_DYNLIB_EXPORT __attribute__((visibility("default")))
#endif

extern "C"
{
  // Exported functions (ABI-stable, no name mangling)
  TEST_DYNLIB_EXPORT int         etest_add(int a, int b);
  TEST_DYNLIB_EXPORT const char* etest_hello();
}

// Symbol names (avoid duplicating string literals in tests)
namespace test_dynlib
{
  constexpr const char* symbol_add = "etest_add";
  constexpr const char* symbol_hello = "etest_hello";
  
  using add_function = decltype(&etest_add);
  using hello_function = decltype(&etest_hello);
}
