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

#include <ecal/ecal.h>
#include "util/ecal_expmap.h"

#include <chrono>
#include <string>
#include <type_traits>

#include <gtest/gtest.h>

class TestingClock {
public:
  // Define the required types for TrivialClock
  using duration = std::chrono::milliseconds;
  using rep = duration::rep;
  using period = duration::period;
  using time_point = std::chrono::time_point<TestingClock>;
  static const bool is_steady = false;

  // Function to get the current time
  static time_point now() noexcept {
    return time_point(current_time);
  }

  // Function to manually set the current time
  static void set_time(const time_point& tp) {
    current_time = tp.time_since_epoch();
  }

  // Function to manually increment the current time by a given duration
  static void increment_time(const duration& d) {
    current_time += d;
  }

private:
  static duration current_time;
};

// Initialize the static member
TestingClock::duration TestingClock::current_time{ 0 };

TEST(core_cpp_core, ExpMap_SetGet)
{
  // create the map with 2500 ms expiration
  eCAL::Util::CExpirationMap<std::string, int, TestingClock> expmap(std::chrono::milliseconds(200));

  // set "A"
  expmap["A"] = 1;

  // get "A"
  EXPECT_EQ(1, expmap["A"]);

  // check size
  //std::map<std::string, int> content = expmap.clone();
  EXPECT_EQ(1, expmap.size());

  // sleep
  TestingClock::increment_time(std::chrono::milliseconds(150));

  // access and reset timer
  EXPECT_EQ(1, expmap["A"]);

  // check size
  //content = expmap.clone();
  expmap.erase_expired();
  EXPECT_EQ(1, expmap.size());

  // sleep
  TestingClock::increment_time(std::chrono::milliseconds(150));

  // check size
  //content = expmap.clone();
  expmap.erase_expired();
  EXPECT_EQ(1, expmap.size());

  // sleep
  TestingClock::increment_time(std::chrono::milliseconds(150));

  // check size
  //content = expmap.clone();
  expmap.erase_expired();
  EXPECT_EQ(0, expmap.size());

  expmap["A"] = 1;
  TestingClock::increment_time(std::chrono::milliseconds(150));
  expmap["B"] = 2;
  expmap["C"] = 3;

  {
    auto erased = expmap.erase_expired();
    EXPECT_EQ(3, expmap.size());
    EXPECT_EQ(0, erased.size());
  }
  
  TestingClock::increment_time(std::chrono::milliseconds(150));
  expmap["B"] = 4;
  
  {
    auto erased = expmap.erase_expired();
    EXPECT_EQ(2, expmap.size());
    EXPECT_EQ(1, erased.size());
    auto a = erased.find("A");
    EXPECT_NE(a, erased.end());
    EXPECT_EQ(a->second, 1);
  }

  TestingClock::increment_time(std::chrono::milliseconds(150));
  
  {
    auto erased = expmap.erase_expired();
    EXPECT_EQ(1, expmap.size());
    EXPECT_EQ(1, erased.size());
    auto c = erased.find("C");
    EXPECT_NE(c, erased.end());
    EXPECT_EQ(c->second, 3);
  }

  // sleep
  TestingClock::increment_time(std::chrono::milliseconds(150));
}

TEST(core_cpp_core, ExpMap_EraseMultiple)
{
  // create the map with 2500 ms expiration
  eCAL::Util::CExpirationMap<std::string, int, TestingClock> expmap(std::chrono::milliseconds(200));

  expmap["A"] = 1;
  expmap["B"] = 2;
  expmap["C"] = 3;

  TestingClock::increment_time(std::chrono::milliseconds(250));

  auto erased = expmap.erase_expired();
  EXPECT_EQ(0, expmap.size());
  EXPECT_EQ(3, erased.size());

  auto a = erased.find("A");
  EXPECT_NE(a, erased.end());
  EXPECT_EQ(a->second, 1);

  auto b = erased.find("B");
  EXPECT_NE(b, erased.end());
  EXPECT_EQ(b->second, 2);

  auto c = erased.find("C");
  EXPECT_NE(c, erased.end());
  EXPECT_EQ(c->second, 3);
}


TEST(core_cpp_core, ExpMap_EraseEmptyMap)
{
  eCAL::Util::CExpirationMap<std::string, int, TestingClock> expmap(std::chrono::milliseconds(200));
  expmap.erase_expired();
  EXPECT_TRUE(expmap.empty());
}

TEST(core_cpp_core, ExpMap_Insert)
{
  eCAL::Util::CExpirationMap<std::string, int, TestingClock> expmap(std::chrono::milliseconds(200));
  auto ret = expmap.insert(std::make_pair("A", 1));

  auto key = (*ret.first).first;
  auto value = (*ret.first).second;
  EXPECT_EQ(std::string("A"), key);
  EXPECT_EQ(1, value);

  int i = expmap["A"];

  EXPECT_EQ(i, 1);

  TestingClock::increment_time(std::chrono::milliseconds(300));
  expmap.erase_expired();
  EXPECT_EQ(0, expmap.size());
}

// This tests uses find to find an element
TEST(core_cpp_core, ExpMap_Find)
{
  eCAL::Util::CExpirationMap<std::string, int, TestingClock> expmap(std::chrono::milliseconds(200));

  auto it = expmap.find("A");
  EXPECT_EQ(expmap.end(), it);


  expmap["A"] = 1;
  it = expmap.find("A");
  int i = (*it).second;
  EXPECT_EQ(i, 1);

  TestingClock::increment_time(std::chrono::milliseconds(300));
  expmap.erase_expired();
  EXPECT_EQ(0, expmap.size());
}

// This test assures that find can be called on a const CExpirationMap and returns an CExpirationMap::const_iterator
TEST(core_cpp_core, ExpMap_FindConst)
{
  eCAL::Util::CExpirationMap<std::string, int, TestingClock> expmap(std::chrono::milliseconds(200));

  auto it = expmap.find("A");
  EXPECT_EQ(expmap.end(), it);

  expmap["A"] = 1;

  const auto& const_ref_exmap = expmap;
  auto const_it = const_ref_exmap.find("A");
  // assert that we are actually getting a const_iterator here!
  static_assert(std::is_same<decltype(const_it), eCAL::Util::CExpirationMap<std::string, int, TestingClock>::const_iterator>::value, "We're not being returned a const_iterator from find.");
  int i = (*const_it).second;
  EXPECT_EQ(i, 1);

  TestingClock::increment_time(std::chrono::milliseconds(300));
  expmap.erase_expired();
  EXPECT_EQ(0, expmap.size());
}

TEST(core_cpp_core, ExpMap_Iterate)
{
  // create the map with 2500 ms expiration
  eCAL::Util::CExpirationMap<std::string, int, TestingClock> expmap(std::chrono::milliseconds(200));
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

void ConstRefIterate(const eCAL::Util::CExpirationMap<std::string, int, TestingClock>& map)
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

TEST(core_cpp_core, ExpMap_ConstExpMapIterate)
{
  // create the map with 2500 ms expiration
  eCAL::Util::CExpirationMap<std::string, int, TestingClock> expmap(std::chrono::milliseconds(200));
  expmap["A"] = 1;

  ConstRefIterate(expmap);
}

TEST(core_cpp_core, ExpMap_Empty)
{
  eCAL::Util::CExpirationMap<std::string, int, TestingClock> expmap(std::chrono::milliseconds(200));
  EXPECT_EQ(true, expmap.empty());
  expmap["A"] = 1;
  EXPECT_EQ(false, expmap.empty());
}

TEST(core_cpp_core, ExpMap_Size)
{
  eCAL::Util::CExpirationMap<std::string, int, TestingClock> expmap(std::chrono::milliseconds(200));
  EXPECT_EQ(0, expmap.size());
  expmap["A"] = 1;
  EXPECT_EQ(1, expmap.size());
}

TEST(core_cpp_core, ExpMap_Remove)
{
  eCAL::Util::CExpirationMap<std::string, int, TestingClock> expmap(std::chrono::milliseconds(200));
  expmap["A"] = 1;
  EXPECT_EQ(1, expmap.size());
  EXPECT_TRUE(expmap.erase("A"));
  EXPECT_EQ(0, expmap.size());
  EXPECT_FALSE(expmap.erase("B"));
}