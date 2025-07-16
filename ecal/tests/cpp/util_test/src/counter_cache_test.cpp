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


#include "util/counter_cache.h"
#include <gtest/gtest.h>
#include <thread>
#include <limits>
#include <string>

using namespace eCAL;

using CounterCache64 = CounterCache<64>;

TEST(CounterCacheTest, InitialStateHasCounter) {
  CounterCache64 cache;
  for (std::size_t i = 0; i < 100; ++i) {
    EXPECT_NE(cache.HasCounter(i), CounterCache64::CounterInCache::True);
    EXPECT_TRUE(cache.IsMonotonic(i));
  }
}

TEST(CounterCacheTest, BasicRegisterAndQuery) {
  CounterCache64 cache;

  cache.SetCounter(0);
  EXPECT_EQ(cache.HasCounter(0), CounterCache64::CounterInCache::True);
  EXPECT_TRUE(cache.IsMonotonic(1));
  cache.SetCounter(1);
  EXPECT_FALSE(cache.IsMonotonic(0));
  EXPECT_FALSE(cache.IsMonotonic(1));
  cache.SetCounter(4);
  EXPECT_TRUE(cache.IsMonotonic(5));
  EXPECT_EQ(cache.HasCounter(6), CounterCache64::CounterInCache::False);
  EXPECT_EQ(cache.HasCounter(5), CounterCache64::CounterInCache::False);
  EXPECT_EQ(cache.HasCounter(4), CounterCache64::CounterInCache::True);
  EXPECT_EQ(cache.HasCounter(3), CounterCache64::CounterInCache::False);
  EXPECT_EQ(cache.HasCounter(2), CounterCache64::CounterInCache::False);
  EXPECT_EQ(cache.HasCounter(1), CounterCache64::CounterInCache::True);
  EXPECT_EQ(cache.HasCounter(0), CounterCache64::CounterInCache::True);
}

TEST(CounterCacheTest, SlidingOneWindow) {
  constexpr std::size_t window_size = 64;
  CounterCache<window_size> cache;

  std::size_t base = window_size;
  std::size_t next_window = base + window_size;
  for (std::size_t i = base; i < next_window ; ++i) {

    cache.SetCounter(i);
    EXPECT_EQ(cache.HasCounter(i), CounterCache64::CounterInCache::True);
  }

  // Trigger one-window slide
  cache.SetCounter(next_window);
  EXPECT_EQ(cache.HasCounter(next_window), CounterCache64::CounterInCache::True);

  // Values in old (previous) window still available
  EXPECT_EQ(cache.HasCounter(base), CounterCache64::CounterInCache::True);

  // Value before both windows
  EXPECT_EQ(cache.HasCounter(base - 1), CounterCache64::CounterInCache::Unsure);
}

TEST(CounterCacheTest, ResetAfterLargeJump) {
  CounterCache64 cache;

  cache.SetCounter(10);
  EXPECT_EQ(cache.HasCounter(10), CounterCache64::CounterInCache::True);

  // Far jump → full reset
  cache.SetCounter(1000);

  EXPECT_EQ(cache.HasCounter(10), CounterCache64::CounterInCache::Unsure);
  EXPECT_EQ(cache.HasCounter(1000), CounterCache64::CounterInCache::True);
}

TEST(CounterCacheTest, MonotonicityChecks) {
  CounterCache64 cache;

  EXPECT_TRUE(cache.IsMonotonic(5));
  cache.SetCounter(5);
  EXPECT_TRUE(cache.IsMonotonic(10));
  cache.SetCounter(10);
  EXPECT_FALSE(cache.IsMonotonic(5));
  EXPECT_TRUE(cache.IsMonotonic(100));
}

