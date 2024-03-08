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

#include <ecal/ecal.h>
#include "util/ecal_expmap.h"

#include <string>
#include <chrono>
#include <thread>
#include <type_traits>

#include <gtest/gtest.h>

TEST(core_cpp_util, ExpMap_SetGet)
{
  // create the map with 2500 ms expiration
  eCAL::Util::CExpMap<std::string, int> expmap(std::chrono::milliseconds(200));

  // set "A"
  expmap["A"] = 1;

  // get "A"
  EXPECT_EQ(1, expmap["A"]);

  // check size
  //std::map<std::string, int> content = expmap.clone();
  EXPECT_EQ(1, expmap.size());

  // sleep
  std::this_thread::sleep_for(std::chrono::milliseconds(150));

  // access and reset timer
  EXPECT_EQ(1, expmap["A"]);

  // check size
  //content = expmap.clone();
  expmap.remove_deprecated();
  EXPECT_EQ(1, expmap.size());

  // sleep
  std::this_thread::sleep_for(std::chrono::milliseconds(150));

  // check size
  //content = expmap.clone();
  expmap.remove_deprecated();
  EXPECT_EQ(1, expmap.size());

  // sleep
  std::this_thread::sleep_for(std::chrono::milliseconds(150));

  // check size
  //content = expmap.clone();
  expmap.remove_deprecated();
  EXPECT_EQ(0, expmap.size());

  expmap["A"] = 1;
  std::this_thread::sleep_for(std::chrono::milliseconds(150));
  expmap["B"] = 2;
  expmap["C"] = 3;
  expmap.remove_deprecated();
  EXPECT_EQ(3, expmap.size());
  std::this_thread::sleep_for(std::chrono::milliseconds(150));
  expmap["B"] = 4;
  expmap.remove_deprecated();
  EXPECT_EQ(2, expmap.size());
  std::this_thread::sleep_for(std::chrono::milliseconds(150));
  expmap.remove_deprecated();
  EXPECT_EQ(1, expmap.size());
  // sleep
  std::this_thread::sleep_for(std::chrono::milliseconds(150));
}

TEST(core_cpp_util, ExpMap_Insert)
{
  eCAL::Util::CExpMap<std::string, int> expmap(std::chrono::milliseconds(200));
  auto ret = expmap.insert(std::make_pair("A", 1));

  auto key = (*ret.first).first;
  auto value = (*ret.first).second;
  EXPECT_EQ(std::string("A"), key);
  EXPECT_EQ(1, value);

  int i = expmap["A"];

  EXPECT_EQ(i, 1);

  std::this_thread::sleep_for(std::chrono::milliseconds(300));
  expmap.remove_deprecated();
  EXPECT_EQ(0, expmap.size());
}

// This tests uses find to find an element
TEST(core_cpp_util, ExpMap_Find)
{
  eCAL::Util::CExpMap<std::string, int> expmap(std::chrono::milliseconds(200));

  auto it = expmap.find("A");
  EXPECT_EQ(expmap.end(), it);


  expmap["A"] = 1;
  it = expmap.find("A");
  int i = (*it).second;
  EXPECT_EQ(i, 1);

  std::this_thread::sleep_for(std::chrono::milliseconds(300));
  expmap.remove_deprecated();
  EXPECT_EQ(0, expmap.size());
}

// This test assures that find can be called on a const CExpMap and returns an CExpMap::const_iterator
TEST(core_cpp_util, ExpMap_FindConst)
{
  eCAL::Util::CExpMap<std::string, int> expmap(std::chrono::milliseconds(200));

  auto it = expmap.find("A");
  EXPECT_EQ(expmap.end(), it);

  expmap["A"] = 1;

  const auto& const_ref_exmap = expmap;
  auto const_it = const_ref_exmap.find("A");
  // assert that we are actually getting a const_iterator here!
  static_assert(std::is_same<decltype(const_it), eCAL::Util::CExpMap<std::string, int>::const_iterator>::value, "We're not being returned a const_iterator from find.");
  int i = (*const_it).second;
  EXPECT_EQ(i, 1);

  std::this_thread::sleep_for(std::chrono::milliseconds(300));
  expmap.remove_deprecated();
  EXPECT_EQ(0, expmap.size());
}

TEST(core_cpp_util, ExpMap_Iterate)
{
  // create the map with 2500 ms expiration
  eCAL::Util::CExpMap<std::string, int> expmap(std::chrono::milliseconds(200));
  expmap["A"] = 1;

  std::string key;
  int value;

  for (auto&& entry : expmap)
  {
    key = entry.first;
    value = entry.second;
  }

  EXPECT_EQ(std::string("A"), key);
  EXPECT_EQ(1, value);
}

void ConstRefIterate(const eCAL::Util::CExpMap<std::string, int>& map)
{
  std::string key;
  int value;

  for (auto&& entry : map)
  {
    key = entry.first;
    value = entry.second;
  }

  EXPECT_EQ(std::string("A"), key);
  EXPECT_EQ(1, value);
}

TEST(core_cpp_util, ExpMap_ConstExpMapIterate)
{
  // create the map with 2500 ms expiration
  eCAL::Util::CExpMap<std::string, int> expmap(std::chrono::milliseconds(200));
  expmap["A"] = 1;

  ConstRefIterate(expmap);
}

TEST(core_cpp_util, ExpMap_Empty)
{
  eCAL::Util::CExpMap<std::string, int> expmap(std::chrono::milliseconds(200));
  EXPECT_EQ(true, expmap.empty());
  expmap["A"] = 1;
  EXPECT_EQ(false, expmap.empty());
}

TEST(core_cpp_util, ExpMap_Size)
{
  eCAL::Util::CExpMap<std::string, int> expmap(std::chrono::milliseconds(200));
  EXPECT_EQ(0, expmap.size());
  expmap["A"] = 1;
  EXPECT_EQ(1, expmap.size());
}

TEST(core_cpp_util, ExpMap_Remove)
{
  eCAL::Util::CExpMap<std::string, int> expmap(std::chrono::milliseconds(200));
  expmap["A"] = 1;
  EXPECT_EQ(1, expmap.size());
  EXPECT_TRUE(expmap.erase("A"));
  EXPECT_EQ(0, expmap.size());
  EXPECT_FALSE(expmap.erase("B"));
}