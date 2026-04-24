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

#include "io/shm/linux/posix_shm_region.h"
#include "io/shm/linux/umask_guard.h"

#include <gtest/gtest.h>
#include <thread>

#include <sys/stat.h>
#include <sys/types.h>

#include <ecal_utils/barrier.h>

// This test ensures that all SHM files are created with correct file permissions
// and concurrent creation of files do not lead to incorrect file permissons
TEST(core_cpp_internal, shm_region_umask_test)
{
  constexpr int number_of_threads = 100;
  constexpr mode_t expected_permissions = 0666;

  Barrier barrier(number_of_threads);

  auto initializer = [](void* /*mapped_addr*/) { return true; };

  std::vector<std::string> shm_names;
  shm_names.reserve(number_of_threads);

  for (int i = 0; i < number_of_threads; ++i)
    shm_names.emplace_back("shm_region_umask_test_" + std::to_string(i));

  std::vector<std::optional<eCAL::posix::ShmRegion>> regions(number_of_threads);
  std::vector<std::thread> threads;
  threads.reserve(number_of_threads);

  // Restrictive umask on purpose: the implementation under test should still
  // create the shm objects with 0666 permissions.
  {
    auto scoped_umask = eCAL::posix::ScopedUmaskRestore(0077);

    for (int i = 0; i < number_of_threads; ++i)
    {
      threads.emplace_back([&, i]()
      {
        barrier.wait();

        regions[i].emplace(
          eCAL::posix::open_or_create_mapped_region(
            shm_names[i],
            100,
            initializer
          )
        );
      });
    }

    for (auto& t : threads)
      t.join();
  }

  auto to_octal_string = [](mode_t mode)
  {
    std::ostringstream oss;
    oss << '0' << std::oct << mode;
    return oss.str();
  };

  for (int i = 0; i < number_of_threads; ++i)
  {
    ASSERT_TRUE(regions[i].has_value()) << "Region creation failed for " << shm_names[i];

    // Adjust this path if your implementation uses a leading slash internally
    // or another backing location.
    const std::string shm_path = "/dev/shm/" + shm_names[i];

    struct stat st {};
    ASSERT_EQ(::stat(shm_path.c_str(), &st), 0)
      << "stat() failed for " << shm_path;

    const mode_t actual_permissions = (st.st_mode & 0777);
    EXPECT_EQ(actual_permissions, expected_permissions)
      << "Wrong permissions for " << shm_path
      << ", expected " << to_octal_string(expected_permissions)
      << ", got " << to_octal_string(actual_permissions);
  }

  // Cleanup from main thread, as requested.
  for (int i = 0; i < number_of_threads; ++i)
  {
    ASSERT_TRUE(regions[i].has_value());

    eCAL::posix::close_region(*regions[i]);

    // If your API provides only a name-based unlink, replace this with:
    // eCAL::posix::unlink_region(shm_names[i]);
    eCAL::posix::unlink_region(*regions[i]);
  }
}
