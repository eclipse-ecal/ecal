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

#include <string>
#include <vector>

#include <gtest/gtest.h>
#include <monitoring/ecal_monitoring_filter.h>

// Unit test class for CExpandingVector
class CMonitoringFilterTest : public ::testing::Test {
protected:
  std::vector<std::string> topic_names;  // Create a vector of MyElement type

  void SetUp() override {
    // Add some initial values
    topic_names.push_back("topic_1");
    topic_names.push_back("topic_2");
    topic_names.push_back("topic_3");
    topic_names.push_back("apple");
    topic_names.push_back("banana");
    topic_names.push_back("__internal");
  }
};

using core_cpp_monitoring_filter = CMonitoringFilterTest;

void EvaluateFilter(const eCAL::CMonitoringFilter& filter, const std::vector<std::string>& input, const std::vector<bool>& expected_outcome)
{
  for (size_t i = 0; i < input.size() && i < expected_outcome.size(); ++i)
  {
    auto accepted = filter.AcceptTopic(input[i]);
    if (expected_outcome[i])
      EXPECT_EQ(accepted, expected_outcome[i]) << input[i] << " should be accepted";
    else
      EXPECT_EQ(accepted, expected_outcome[i]) << input[i] << " should not be accepted";
  }
}


// Test push_back and size functionality
TEST_F(core_cpp_monitoring_filter, DefaultFilter) {
  // A default filter should accept all topics
  eCAL::CMonitoringFilter filter{ eCAL::Monitoring::SAttributes() };
  std::vector<bool> expected = { true, true, true, true, true, true };

  EvaluateFilter(filter, topic_names, expected);
}

TEST_F(core_cpp_monitoring_filter, ExcludeFilter) {
  // A default filter should accept all topics
  eCAL::Monitoring::SAttributes attr;
  attr.filter_excl = "^__.*$";
  attr.filter_incl = "";
  eCAL::CMonitoringFilter filter{ attr };
  std::vector<bool> expected = { true, true, true, true, true, false };
  EvaluateFilter(filter, topic_names, expected);
}

TEST_F(core_cpp_monitoring_filter, IncludeFilter) {
  // A default filter should accept all topics
  eCAL::Monitoring::SAttributes attr;
  attr.filter_excl = "";
  attr.filter_incl = "^topic_.*$";
  eCAL::CMonitoringFilter filter{ attr };
  std::vector<bool> expected = { true, true, true, false, false, false };
  EvaluateFilter(filter, topic_names, expected);
}

TEST_F(core_cpp_monitoring_filter, IncludeExcludeFilter) {
  // A default filter should accept all topics
  eCAL::Monitoring::SAttributes attr;
  attr.filter_excl = "^topic_1$";
  attr.filter_incl = "^topic_.*$";
  eCAL::CMonitoringFilter filter{ attr };
  std::vector<bool> expected = { false, true, true, false, false, false };
  EvaluateFilter(filter, topic_names, expected);
}