
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

#include <gtest/gtest.h>

#include "util/unique_single_instance.h"

TEST(UniqueSingleInstanceTest /*unused*/, BasicCreate /*unused*/) 
{
    auto instance1 = eCAL::Util::CUniqueSingleInstance<int>::Create(42);
    EXPECT_NE(instance1, nullptr);
    EXPECT_EQ(*instance1, 42);

    auto instance2 = eCAL::Util::CUniqueSingleInstance<int>::Create(42);
    EXPECT_EQ(instance2, nullptr); // Should not create a second instance

    auto instance_float = eCAL::Util::CUniqueSingleInstance<float>::Create(3.14f);
    EXPECT_NE(instance_float, nullptr);
    EXPECT_EQ(*instance_float, 3.14f);

    auto instance_float2 = eCAL::Util::CUniqueSingleInstance<float>::Create(2.71f);
    EXPECT_EQ(instance_float2, nullptr); // Should not create a second instance
}

TEST(UniqueSingleInstanceTest /*unused*/, ClassImplementation /*unused*/) 
{
  class TestClass
  {
    friend class eCAL::Util::CUniqueSingleInstance<TestClass>;
    public:
      using TestClassUniquePtrT = eCAL::Util::CUniqueSingleInstance<TestClass>::UniqueT;
      
      static TestClassUniquePtrT Create(int value) {
        return eCAL::Util::CUniqueSingleInstance<TestClass>::Create(value);
      }
      ~TestClass() = default;

      int GetValue() const { return value_; }

    private:
      TestClass(int value) : value_(value) {}
      int value_;
  };

  auto instance1 = TestClass::Create(100);
  EXPECT_NE(instance1, nullptr);
  EXPECT_EQ(instance1->GetValue(), 100);

  auto instance2 = TestClass::Create(200);
  EXPECT_EQ(instance2, nullptr); // Should not create a second instance

  instance1.reset(); // Destroy the first instance

  auto instance3 = TestClass::Create(300);
  EXPECT_NE(instance3, nullptr);
  EXPECT_EQ(instance3->GetValue(), 300);
}