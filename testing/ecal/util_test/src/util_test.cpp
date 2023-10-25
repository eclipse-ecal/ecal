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
#include <gtest/gtest.h>


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

TEST(Util, CombineTopicEncodingAndType)
{
  TestCombinedTopicEncodingAndType("", "", "");
  TestCombinedTopicEncodingAndType("proto", "pb.Person.People", "proto:pb.Person.People");
  TestCombinedTopicEncodingAndType("base", "", "base:");
  TestCombinedTopicEncodingAndType("", "MyType", "MyType");
}

TEST(Util, SplitCombinedTopicType)
{
  TestSplitCombinedTopicType("", "", "");
  TestSplitCombinedTopicType("proto:pb.Person.People", "proto", "pb.Person.People");
  TestSplitCombinedTopicType("base:", "base", "");
  TestSplitCombinedTopicType("base:std::string", "base", "std::string");
  TestSplitCombinedTopicType("MyType", "", "MyType");
}
