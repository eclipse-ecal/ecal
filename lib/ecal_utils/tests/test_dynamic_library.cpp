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

// test_dynamic_library_gtest.cpp
#include <gtest/gtest.h>

#include <ecal_utils/dynamic_library.h>
#include "dynamic_library.h"

#include <string>
#include <variant>

#ifndef TEST_DYNLIB_PATH
#error "TEST_DYNLIB_PATH is not defined. Pass the built test plugin path via target_compile_definitions."
#endif

namespace
{
  const char* kLibPath = TEST_DYNLIB_PATH;
}

using namespace EcalUtils;

TEST(DynamicLibrary, Load_Fails_OnEmptyPath)
{
  auto res = DynamicLibrary::Load("");
  ASSERT_TRUE(std::holds_alternative<DlError>(res));
  const auto& err = std::get<DlError>(res);
  EXPECT_EQ(err.code, DlError::Code::InvalidArgument);
  EXPECT_FALSE(err.message.empty());
}

TEST(DynamicLibrary, Load_Fails_OnNonExistingPath)
{
  auto res = DynamicLibrary::Load(std::string("this/does/not/exist/") + "missing_library_12345");
  ASSERT_TRUE(std::holds_alternative<DlError>(res));
  const auto& err = std::get<DlError>(res);
  EXPECT_EQ(err.code, DlError::Code::LoadFailed);
  EXPECT_FALSE(err.message.empty());
}

TEST(DynamicLibrary, Load_Succeeds_ForTestLibrary)
{
  auto res = DynamicLibrary::Load(kLibPath);
  ASSERT_TRUE(std::holds_alternative<DynamicLibrary>(res));
}

TEST(DynamicLibrary, LoadSymbol_FindsExportedFunctions)
{
  auto lib_res = DynamicLibrary::Load(kLibPath);
  ASSERT_TRUE(std::holds_alternative<DynamicLibrary>(lib_res));
  DynamicLibrary lib = std::move(std::get<DynamicLibrary>(lib_res));

  auto add_res = lib.LoadSymbol<test_dynlib::add_function>(test_dynlib::symbol_add);
  ASSERT_TRUE(std::holds_alternative<test_dynlib::add_function>(add_res));
  EXPECT_EQ(std::get<test_dynlib::add_function>(add_res)(2, 3), 5);

  auto hello_res = lib.LoadSymbol<test_dynlib::hello_function>(test_dynlib::symbol_hello);
  ASSERT_TRUE(std::holds_alternative<test_dynlib::hello_function>(hello_res));
  EXPECT_STREQ(std::get<test_dynlib::hello_function>(hello_res)(), "hello");
}

TEST(DynamicLibrary, LoadSymbol_Fails_OnMissingSymbol)
{
  auto lib_res = DynamicLibrary::Load(kLibPath);
  ASSERT_TRUE(std::holds_alternative<DynamicLibrary>(lib_res));
  DynamicLibrary lib = std::move(std::get<DynamicLibrary>(lib_res));

  auto sym_res = lib.LoadSymbol<test_dynlib::add_function>("this_symbol_does_not_exist_12345");
  ASSERT_TRUE(std::holds_alternative<DlError>(sym_res));
  const auto& err = std::get<DlError>(sym_res);
  EXPECT_EQ(err.code, DlError::Code::SymbolNotFound);
  EXPECT_FALSE(err.message.empty());
}

TEST(DynamicLibrary, SymbolOrNone_ReturnsNullptrForMissingSymbol)
{
  auto lib_res = DynamicLibrary::Load(kLibPath);
  ASSERT_TRUE(std::holds_alternative<DynamicLibrary>(lib_res));
  DynamicLibrary lib = std::move(std::get<DynamicLibrary>(lib_res));

  auto fn = lib.SymbolOrNone<test_dynlib::add_function>("this_symbol_does_not_exist_12345");
  EXPECT_EQ(fn, nullptr);
}

TEST(DynamicLibrary, LoadRawSymbol_ReturnsPointerForExistingSymbol)
{
  auto lib_res = DynamicLibrary::Load(kLibPath);
  ASSERT_TRUE(std::holds_alternative<DynamicLibrary>(lib_res));
  DynamicLibrary lib = std::move(std::get<DynamicLibrary>(lib_res));

  auto raw_res = lib.LoadRawSymbol(test_dynlib::symbol_add);
  ASSERT_TRUE(std::holds_alternative<void*>(raw_res));
  EXPECT_NE(std::get<void*>(raw_res), nullptr);
}

TEST(DynamicLibrary, MoveConstruction_PreservesHandle)
{
  auto lib_res = DynamicLibrary::Load(kLibPath);
  ASSERT_TRUE(std::holds_alternative<DynamicLibrary>(lib_res));
  DynamicLibrary lib1 = std::move(std::get<DynamicLibrary>(lib_res));

  DynamicLibrary lib2 = std::move(lib1);

  auto add_res = lib2.LoadSymbol<test_dynlib::add_function>(test_dynlib::symbol_add);
  ASSERT_TRUE(std::holds_alternative<test_dynlib::add_function>(add_res));
  EXPECT_EQ(std::get<test_dynlib::add_function>(add_res)(10, 5), 15);

  // Don't call methods on moved-from lib1; your implementation asserts.
}
