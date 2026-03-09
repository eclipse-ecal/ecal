
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

#include "util/single_instance_helper.h"
#include "ecal_utils/barrier.h"

#include <thread>

class TestClass
  {
    friend class eCAL::Util::CSingleInstanceHelper<TestClass>;
    public:
      static std::shared_ptr<TestClass> Create(int value) {
        return eCAL::Util::CSingleInstanceHelper<TestClass>::Create(value);
      }

      int GetValue() const { return value_; }

      ~TestClass() = default;

    private:
      TestClass(int value) : value_(value) {}
      
      int value_;
  };

TEST(SingleInstanceHelperTest /*unused*/, BasicCreate /*unused*/) 
{
    auto instance1 = eCAL::Util::CSingleInstanceHelper<int>::Create(42);
    EXPECT_NE(instance1, nullptr);
    EXPECT_EQ(*instance1, 42);

    auto instance2 = eCAL::Util::CSingleInstanceHelper<int>::Create(42);
    EXPECT_EQ(instance2, instance1);

    auto instance_float = eCAL::Util::CSingleInstanceHelper<float>::Create(3.14f);
    EXPECT_NE(instance_float, nullptr);
    EXPECT_EQ(*instance_float, 3.14f);

    auto instance_float2 = eCAL::Util::CSingleInstanceHelper<float>::Create(2.71f);
    EXPECT_EQ(instance_float2, instance_float);
}

TEST(SingleInstanceHelperTest /*unused*/, ClassImplementation /*unused*/) 
{
  auto instance1 = TestClass::Create(100);
  EXPECT_NE(instance1, nullptr);
  EXPECT_EQ(instance1->GetValue(), 100);

  auto instance2 = TestClass::Create(200);
  EXPECT_EQ(instance2, instance1);

  instance1.reset();

  // One ptr still exists, so instance3 would be same as instance2
  auto instance3 = TestClass::Create(300);
  EXPECT_EQ(instance3, instance2);
  EXPECT_EQ(instance3->GetValue(), 100);

  instance2.reset();
  instance3.reset();

  // Now a new instance should be created
  auto instance4 = TestClass::Create(400);
  EXPECT_NE(instance4, nullptr);
  EXPECT_EQ(instance4->GetValue(), 400);
}

TEST(SingleInstanceHelperTest /*unused*/, MultithreadedCreation /*unused*/)
{
  constexpr int thread_count = 50;
  constexpr int instance_value = 123;
  Barrier barrier(thread_count + 1); // +1 for the main thread that will wait for all threads to be ready
  std::vector<std::shared_ptr<TestClass>> instances(thread_count);

  auto thread_function = [&](int thread_id) {
    barrier.wait();
    instances[thread_id] = TestClass::Create(instance_value);
  };

  std::vector<std::thread> threads;
  for (int i = 0; i < thread_count; ++i) {
    threads.emplace_back(thread_function, i);
  }

  barrier.wait(); // Wait for all threads to be ready before they start creating instances

  std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Give some time for all threads to finish creating instances
  
  // Compare each shared pointer in the vector the first one, they should all be the same instance
  for (size_t i = 1; i < instances.size(); ++i) {
    EXPECT_EQ(instances[i], instances[0]);
    EXPECT_EQ(instances[i]->GetValue(), instance_value);
  }

  for (auto& thread : threads) {
    thread.join();
  }
}
