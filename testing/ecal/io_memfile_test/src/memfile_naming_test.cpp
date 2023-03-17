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

#include "io/ecal_memfile_naming.h"

#include <atomic>
#include <chrono>
#include <memory>
#include <iostream>
#include <thread>

#include <gtest/gtest.h>


TEST(IO, MemfileNaming)
{
  std::chrono::steady_clock::time_point timepoint{};

  std::string memfile_name_1{ eCAL::memfile::BuildRandomMemFileName("test_")};
  std::string memfile_name_2{ eCAL::memfile::BuildRandomMemFileName("test_")};

  EXPECT_LE(memfile_name_1.size(), 13);
  EXPECT_NE(memfile_name_1, memfile_name_2);

}
