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

/*
#include "util/message_drop_calculator.h."
#include <gtest/gtest.h>
#include <thread>
#include <limits>
#include <string>

using ecal::MessageDropCalculator;
using ecal::MessageDropManager;

//----------------------------------------------------------------------
// Tests for MessageDropCalculator (not thread-safe)
//----------------------------------------------------------------------

TEST(MessageDropCalculatorTest, PublisherOnlyCountsFullDiff) {
  MessageDropCalculator c;
  c.applyReceivedPublisherUpdate(100);
  c.applyReceivedPublisherUpdate(105);
  auto s = c.getSummary();
  EXPECT_EQ(s.newDrops,   5ull);
  EXPECT_EQ(s.totalDrops, 5ull);
}

TEST(MessageDropCalculatorTest, PublisherThenFirstMessage_NoDrop) {
  MessageDropCalculator c;
  c.applyReceivedPublisherUpdate(50);
  c.registerReceivedMessage(55);
  auto s = c.getSummary();
  EXPECT_EQ(s.newDrops,   0ull);
  EXPECT_EQ(s.totalDrops, 0ull);
}

TEST(MessageDropCalculatorTest, MultiplePublisherUpdatesAccumulate) {
  MessageDropCalculator c;
  c.applyReceivedPublisherUpdate(10);
  c.applyReceivedPublisherUpdate(20);
  c.applyReceivedPublisherUpdate(30);
  auto s = c.getSummary();
  EXPECT_EQ(s.newDrops,   20ull);
  EXPECT_EQ(s.totalDrops, 20ull);
}

TEST(MessageDropCalculatorTest, SuppressPublisherAfterMessage) {
  MessageDropCalculator c;
  c.applyReceivedPublisherUpdate(100);
  c.registerReceivedMessage(105);      // first real message → no drop
  c.applyReceivedPublisherUpdate(110); // suppressed
  auto s = c.getSummary();
  EXPECT_EQ(s.newDrops,   0ull);
  EXPECT_EQ(s.totalDrops, 0ull);
}

TEST(MessageDropCalculatorTest, PublisherAfterSuppressionThenCount) {
  MessageDropCalculator c;
  c.applyReceivedPublisherUpdate(200);
  c.registerReceivedMessage(205);       // first message → no drop
  c.applyReceivedPublisherUpdate(210);  // suppressed
  (void)c.getSummary();                 // reset counters
  c.applyReceivedPublisherUpdate(220);  // now counts 10
  auto s = c.getSummary();
  EXPECT_EQ(s.newDrops,   10ull);
  EXPECT_EQ(s.totalDrops, 10ull);
}

TEST(MessageDropCalculatorTest, WrapAroundInRegister_NoDrop) {
  MessageDropCalculator c;
  uint64_t M = std::numeric_limits<uint64_t>::max();
  c.registerReceivedMessage(M - 2);
  c.registerReceivedMessage(0);
  c.registerReceivedMessage(1);
  auto s = c.getSummary();
  EXPECT_EQ(s.totalDrops, 0ull);
  EXPECT_EQ(s.newDrops,   0ull);
}

TEST(MessageDropCalculatorTest, WrapAroundInPublisher_NoDrop) {
  MessageDropCalculator c;
  uint64_t M = std::numeric_limits<uint64_t>::max();
  c.applyReceivedPublisherUpdate(M - 2);
  c.applyReceivedPublisherUpdate(0);
  c.applyReceivedPublisherUpdate(1);
  auto s = c.getSummary();
  EXPECT_EQ(s.totalDrops, 0ull);
  EXPECT_EQ(s.newDrops,   0ull);
}

//----------------------------------------------------------------------
// Tests for MessageDropManager (thread-safe, per-key + aggregate)
//----------------------------------------------------------------------

TEST(MessageDropManagerTest, PerKeySummaryAndIsolation) {
  MessageDropManager<std::string> m;
  m.applyReceivedPublisherUpdate("A", 10);
  m.applyReceivedPublisherUpdate("A", 20); // +10
  m.applyReceivedPublisherUpdate("B", 100);
  m.applyReceivedPublisherUpdate("B", 110); // +10

  auto sA = m.getSummary("A");
  EXPECT_EQ(sA.totalDrops, 10ull);
  EXPECT_EQ(sA.newDrops,   10ull);

  auto sB = m.getSummary("B");
  EXPECT_EQ(sB.totalDrops, 10ull);
  EXPECT_EQ(sB.newDrops,   10ull);

  // After reading A and B individually, aggregate should be zero
  auto sAll0 = m.getSummary();
  EXPECT_EQ(sAll0.totalDrops, 0ull);
  EXPECT_EQ(sAll0.newDrops,   0ull);

  // trigger more drops
  m.applyReceivedPublisherUpdate("A", 30);  // +10 for A
  m.applyReceivedPublisherUpdate("C", 1);   // init C = no drop
  m.applyReceivedPublisherUpdate("C", 5);   // +4 for C

  auto sAll1 = m.getSummary();
  EXPECT_EQ(sAll1.totalDrops, 14ull);
  EXPECT_EQ(sAll1.newDrops,   14ull);
}

TEST(MessageDropManagerTest, ConcurrentRegisterAndPublisherUpdates) {
  MessageDropManager<int> m;
  constexpr int N_KEYS = 5;
  constexpr uint64_t N_UPDATES = 50000;

  // Worker that alternates register and publisher on a given key
  auto worker = [&](int key){
    for (uint64_t i = 1; i <= N_UPDATES; ++i) {
      m.registerReceivedMessage(key, i);
      m.applyReceivedPublisherUpdate(key, i);
    }
  };

  // Launch one thread per key
  std::vector<std::thread> threads;
  for (int k = 0; k < N_KEYS; ++k) {
    threads.emplace_back(worker, k);
  }
  for (auto &t : threads) t.join();

  // In a perfect interleaving, no drops should accumulate
  auto agg = m.getSummary();
  EXPECT_EQ(agg.totalDrops, 0ull);
  EXPECT_EQ(agg.newDrops,   0ull);
}
*/