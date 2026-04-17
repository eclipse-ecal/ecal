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

#if defined(__linux__)

#include "io/shm/linux/umask_guard.h"

#include <gtest/gtest.h>

#include <array>
#include <atomic>
#include <fcntl.h>
#include <mutex>
#include <random>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>

namespace
{
  inline std::string RandomShmName20(const std::string& prefix)
  {
    static constexpr std::array<char, 36> alphabet{
      'a','b','c','d','e','f','g','h','i','j','k','l','m',
      'n','o','p','q','r','s','t','u','v','w','x','y','z',
      '0','1','2','3','4','5','6','7','8','9'
    };

    thread_local std::mt19937_64 rng{ std::random_device{}() };
    std::uniform_int_distribution<size_t> dist(0, alphabet.size() - 1);

    std::string name = "/" + prefix + "_";
    for (size_t i = 0; i < 20; ++i)
      name.push_back(alphabet[dist(rng)]);

    return name;
  }

  class ScopedUmask
  {
  public:
    explicit ScopedUmask(mode_t target_umask)
      : previous_umask_(::umask(target_umask))
    {}

    ~ScopedUmask()
    {
      ::umask(previous_umask_);
    }

    ScopedUmask(const ScopedUmask&) = delete;
    ScopedUmask& operator=(const ScopedUmask&) = delete;

  private:
    mode_t previous_umask_;
  };

  mode_t GetCurrentUmask(mode_t probe)
  {
    const mode_t current_umask = ::umask(probe);
    ::umask(current_umask);
    return current_umask;
  }

  bool CreateShmGuardedAndCheckPermissions(const std::string& shm_name)
  {
    const std::lock_guard<std::mutex> lock{eCAL::io::shm::linux::GetUmaskCreationMutex()};
    const eCAL::io::shm::linux::ScopedUmaskRestore scoped_umask{000};

    int fd = ::shm_open(shm_name.c_str(), O_CREAT | O_RDWR | O_EXCL, 0666);
    if (fd == -1)
      return false;

    ::close(fd);

    const std::string dev_shm_path = "/dev/shm" + shm_name;
    struct stat st{};
    if (::stat(dev_shm_path.c_str(), &st) != 0)
    {
      ::shm_unlink(shm_name.c_str());
      return false;
    }

    const mode_t mode = static_cast<mode_t>(st.st_mode & 0777);
    const bool mode_ok = (mode == static_cast<mode_t>(0666));

    ::shm_unlink(shm_name.c_str());

    return mode_ok;
  }

}

TEST(core_cpp_io, UnguardedUmaskParallelSequenceCanLeaveWrongUmask)
{
  ScopedUmask force_umask_022{022};

  std::atomic<int> stage{0};
  mode_t previous_umask_a = 0;
  mode_t previous_umask_b = 0;

  std::thread thread_a([&]()
    {
      while (stage.load() < 1)
        std::this_thread::yield();

      previous_umask_a = ::umask(000);
      stage.store(2);

      while (stage.load() < 4)
        std::this_thread::yield();

      ::umask(previous_umask_a);
      stage.store(5);
    });

  std::thread thread_b([&]()
    {
      while (stage.load() < 2)
        std::this_thread::yield();

      previous_umask_b = ::umask(000);
      stage.store(3);

      while (stage.load() < 5)
        std::this_thread::yield();

      ::umask(previous_umask_b);
    });

  stage.store(1);
  while (stage.load() < 3)
    std::this_thread::yield();
  stage.store(4);

  thread_a.join();
  thread_b.join();

  // Demonstrates the original race: one thread may capture the transient umask (000)
  // and restore that value last, leaving the process umask unexpectedly changed.
  const mode_t current_umask = GetCurrentUmask(022);
  EXPECT_EQ(static_cast<mode_t>(000), current_umask);

  const std::string tmp_file = "/tmp/ecal_umask_race_test_file";
  const int tmp_fd = ::open(tmp_file.c_str(), O_CREAT | O_TRUNC | O_RDWR, 0666);
  ASSERT_NE(-1, tmp_fd);
  ::close(tmp_fd);

  struct stat st{};
  ASSERT_EQ(0, ::stat(tmp_file.c_str(), &st));
  EXPECT_EQ(static_cast<mode_t>(0666), static_cast<mode_t>(st.st_mode & 0777));

  ::unlink(tmp_file.c_str());
}

TEST(core_cpp_io, GuardedShmCreationKeepsPermissionsAndRestoresUmask)
{
  ScopedUmask force_umask_022{022};

  constexpr int iterations_per_thread = 100;
  std::atomic<int> success_count{0};

  auto guarded_creator = [&success_count](int thread_index)
  {
    for (int i = 0; i < iterations_per_thread; ++i)
    {
      const std::string shm_name = RandomShmName20("guarded_shm_" + std::to_string(thread_index) + "_" + std::to_string(i));
      if (CreateShmGuardedAndCheckPermissions(shm_name))
      {
        success_count.fetch_add(1);
      }
    }
  };

  std::thread t1(guarded_creator, 1);
  std::thread t2(guarded_creator, 2);
  t1.join();
  t2.join();

  EXPECT_EQ(iterations_per_thread * 2, success_count.load());
  EXPECT_EQ(static_cast<mode_t>(022), GetCurrentUmask(022));

  const std::string tmp_file = "/tmp/ecal_umask_guarded_test_file";
  const int tmp_fd = ::open(tmp_file.c_str(), O_CREAT | O_TRUNC | O_RDWR, 0666);
  ASSERT_NE(-1, tmp_fd);
  ::close(tmp_fd);

  struct stat st{};
  ASSERT_EQ(0, ::stat(tmp_file.c_str(), &st));
  EXPECT_EQ(static_cast<mode_t>(0644), static_cast<mode_t>(st.st_mode & 0777));

  ::unlink(tmp_file.c_str());
}

#endif // defined(__linux__)
