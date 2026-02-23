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

#include <ecal/process.h>

#include <ecal_utils/barrier.h>
#include <ecal_utils/filesystem.h>

#include <gtest/gtest.h>

#include <string>
#include <thread>
#include <vector>

TEST(process_cpp_public, GetProcessNameReturnsExpectedExecutable)
{
  const std::string process_name = eCAL::Process::GetProcessName();

  ASSERT_FALSE(process_name.empty());
  EXPECT_EQ(std::string::npos, process_name.find('\0'));

  const std::string executable_name = EcalUtils::Filesystem::BaseName(process_name);
  EXPECT_EQ(std::string(ECAL_PROCESS_TEST_EXECUTABLE_NAME), executable_name);
}

TEST(process_cpp_public, GetProcessNameIsThreadSafe)
{
  constexpr int thread_count = 16;
  constexpr int calls_per_thread = 200;

  Barrier barrier(thread_count);
  std::vector<std::thread> threads;
  threads.reserve(thread_count);

  for (int i = 0; i < thread_count; ++i)
  {
    threads.emplace_back([&barrier]()
    {
      barrier.wait();

      for (int call = 0; call < calls_per_thread; ++call)
      {
        const std::string process_name = eCAL::Process::GetProcessName();

        ASSERT_FALSE(process_name.empty());
        EXPECT_EQ(std::string::npos, process_name.find('\0'));

        const std::string executable_name = EcalUtils::Filesystem::BaseName(process_name);
        EXPECT_EQ(std::string(ECAL_PROCESS_TEST_EXECUTABLE_NAME), executable_name);
      }
    });
  }

  for (auto& thread : threads)
  {
    thread.join();
  }
}
