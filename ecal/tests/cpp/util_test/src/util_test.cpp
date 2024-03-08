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

#include <vector>
#include <ecal/ecal.h>
#include <gtest/gtest.h>
#include <util/frequency_calculator.h>


namespace {
  void TestCombinedTopicEncodingAndType(const std::string& encoding, const std::string& type, const std::string& expected_result)
  {
    auto combined = eCAL::Util::CombinedTopicEncodingAndType(encoding, type);
    EXPECT_EQ(combined, expected_result);
  }

  void TestSplitCombinedTopicType(const std::string& combined_topic_type, const std::string& expected_encoding, const std::string& expected_type)
  {
    auto split = eCAL::Util::SplitCombinedTopicType(combined_topic_type);
    EXPECT_EQ(split.first, expected_encoding);
    EXPECT_EQ(split.second, expected_type);
  }
}

TEST(core_cpp_util, Util_CombineTopicEncodingAndType)
{
  TestCombinedTopicEncodingAndType("", "", "");
  TestCombinedTopicEncodingAndType("proto", "pb.Person.People", "proto:pb.Person.People");
  TestCombinedTopicEncodingAndType("base", "", "base:");
  TestCombinedTopicEncodingAndType("", "MyType", "MyType");
}

TEST(core_cpp_util, Util_SplitCombinedTopicType)
{
  TestSplitCombinedTopicType("", "", "");
  TestSplitCombinedTopicType("proto:pb.Person.People", "proto", "pb.Person.People");
  TestSplitCombinedTopicType("base:", "base", "");
  TestSplitCombinedTopicType("base:std::string", "base", "std::string");
  TestSplitCombinedTopicType("MyType", "", "MyType");
}

struct MillisecondFrequencyPair
{
  std::chrono::milliseconds delta_t;
  double frequency;
};

const std::vector<MillisecondFrequencyPair> frequency_pairs =
{
  {std::chrono::milliseconds(1000),   1.0},
  {std::chrono::milliseconds(1250),   0.8},
  {std::chrono::milliseconds(5000),   0.2},
  {std::chrono::milliseconds(20000),  0.05},
  {std::chrono::milliseconds(500),    2.0},
  {std::chrono::milliseconds(200),    5.0},
  {std::chrono::milliseconds(100),   10.0},
  {std::chrono::milliseconds(20),    50.0},
  {std::chrono::milliseconds(2),    500.0},
};


TEST(Util, FrequencyCalculator)
{
  for (const auto& pair : frequency_pairs)
  {
    {
      auto now = std::chrono::steady_clock::now();
      eCAL::FrequencyCalculator<std::chrono::steady_clock> calculator(now);
      EXPECT_DOUBLE_EQ(calculator.getFrequency(), 0.0f);

      for (int j = 0; j < 100; ++j)
      {
        for (int i = 0; i < 20; ++i)
        {
          now = now + pair.delta_t;
          calculator.addTick(now);
        }

        EXPECT_DOUBLE_EQ(calculator.getFrequency(), pair.frequency);
      }
    }

    {
      auto now = std::chrono::steady_clock::now();
      eCAL::FrequencyCalculator<std::chrono::steady_clock> calculator(now);

      for (int i = 0; i < 5; ++i)
      {
        now = now + pair.delta_t;
        calculator.addTick(now);
        EXPECT_DOUBLE_EQ(calculator.getFrequency(), pair.frequency);
      }
    }
  }
}

TEST(Util, ResettableFrequencyCalculator)
{
  const auto check_delta_t = std::chrono::milliseconds(999);


  for (const auto& pair : frequency_pairs)
  {
    {
      //auto start = std::chrono::steady_clock::now();
      auto start = std::chrono::steady_clock::time_point(std::chrono::milliseconds(0));
      auto next_frequency_update = start;
      auto next_tick = start;
      eCAL::ResettableFrequencyCalculator<std::chrono::steady_clock> calculator(3.0f);
      calculator.addTick(next_tick);
      EXPECT_DOUBLE_EQ(calculator.getFrequency(next_frequency_update), 0.0f);

      for (int i = 0; i < 100; ++i)
      {
        next_frequency_update = next_frequency_update + check_delta_t;

        while (next_tick + pair.delta_t <= next_frequency_update)
        {
          next_tick = next_tick + pair.delta_t;
          calculator.addTick(next_tick);
        }

        if (next_frequency_update - start < pair.delta_t)
        {
          // no updates happened yet - expect 0 frequency
          EXPECT_DOUBLE_EQ(calculator.getFrequency(next_frequency_update), 0.0);
        }
        else
        {
          EXPECT_DOUBLE_EQ(calculator.getFrequency(next_frequency_update), pair.frequency);
        }
      }

      // Now check timeout behavior
      // Calculate when timeout should happen
      auto timeout_time = next_tick + 3.0f * pair.delta_t;
      for (int i = 0; i < 100; ++i)
      {
        next_frequency_update = next_frequency_update + check_delta_t;
        if (next_frequency_update <= timeout_time)
        {
          EXPECT_DOUBLE_EQ(calculator.getFrequency(next_frequency_update), pair.frequency);
        }
        else
        {
          EXPECT_DOUBLE_EQ(calculator.getFrequency(next_frequency_update), 0.0);
        }
      }

      // Finally we're getting new ticks again! let's check frequencies again!
      auto new_start = next_frequency_update;
      next_tick = next_frequency_update;
      // Check that filter goes back
      calculator.addTick(next_tick);
      for (int i = 0; i < 100; ++i)
      {
        next_frequency_update = next_frequency_update + check_delta_t;

        while (next_tick + pair.delta_t <= next_frequency_update)
        {
          next_tick = next_tick + pair.delta_t;
          calculator.addTick(next_tick);
        }

        if (next_frequency_update - new_start < pair.delta_t)
        {
          // no updates happened yet - expect 0 frequency
          EXPECT_DOUBLE_EQ(calculator.getFrequency(next_frequency_update), 0.0);
        }
        else
        {
          EXPECT_DOUBLE_EQ(calculator.getFrequency(next_frequency_update), pair.frequency);
        }
      }
    }
  }
}

