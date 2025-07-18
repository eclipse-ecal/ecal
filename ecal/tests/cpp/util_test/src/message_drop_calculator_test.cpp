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

#include "util/message_drop_calculator.h"
#include <gtest/gtest.h>
#include <vector>
#include <cstdint>

using eCAL::MessageDropCalculator;

struct SummaryExpect {
  uint64_t drops;
  bool new_drops;
};

class MessageDropCalculatorTest : public ::testing::Test {
protected:
  MessageDropCalculator calc;

  void ApplyUpdates(const std::vector<uint64_t>& updates) {
    for (auto u : updates) {
      calc.RegisterReceivedMessage(u);
    }
  }

  void ExpectSummary(const SummaryExpect& expected) {
    auto summary = calc.GetSummary();
    EXPECT_EQ(summary.drops, expected.drops);
    EXPECT_EQ(summary.new_drops, expected.new_drops);
  }
};

TEST_F(MessageDropCalculatorTest, NoDropsOnEmptyCalculator) {
  ExpectSummary({ 0, false });
  ExpectSummary({ 0, false });
}

TEST_F(MessageDropCalculatorTest, NoDropsOnOneUpdate) {
  ApplyUpdates({ 0 });
  ExpectSummary({ 0, false });
}

TEST_F(MessageDropCalculatorTest, NoDropsOnSequentialUpdates) {
  ApplyUpdates({ 0, 1 });
  ExpectSummary({ 0, false });
  ApplyUpdates({ 2 });
  ExpectSummary({ 0, false });
}

TEST_F(MessageDropCalculatorTest, DropsOnMissingValues) {
  ApplyUpdates({ 0, 1, 3 });
  ExpectSummary({ 1, true });
  ApplyUpdates({ 4 });
  ExpectSummary({ 1, false });
  ApplyUpdates({ 5, 8, 10 });
  ExpectSummary({ 4, true });
}

