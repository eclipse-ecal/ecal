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

#include <filesystem>
#include <string>
#include <thread>
#include <vector>

#include <ecal_utils/barrier.h>

#include <gtest/gtest.h>


std::string normalize_path(const std::string& path_string)
{
  std::filesystem::path path(path_string);
  std::filesystem::path canonicalPath = std::filesystem::weakly_canonical(path);
  return canonicalPath.make_preferred().string();
}

TEST(process_cpp_public, GetProcessNameReturnsExpectedExecutable)
{
  const std::string process_name = eCAL::Process::GetProcessName();


  EXPECT_EQ(normalize_path(std::string(ECAL_PROCESS_TEST_EXECUTABLE_NAME)), normalize_path(process_name));
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
    threads.emplace_back([&barrier, &calls_per_thread]()
    {
      barrier.wait();

      for (int call = 0; call < calls_per_thread; ++call)
      {
        const std::string process_name = eCAL::Process::GetProcessName();
        EXPECT_EQ(normalize_path(std::string(ECAL_PROCESS_TEST_EXECUTABLE_NAME)), normalize_path(process_name));
      }
    });
  }

  for (auto& thread : threads)
  {
    thread.join();
  }
}
