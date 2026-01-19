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

#include <condition_variable>
#include <mutex>
#include <stdexcept>

/*
* A reusable barrier for synchronizing a set of threads.
* The same barrier instance can be waited upon by multiple threads.
* It can be waited upon multiple times, resetting automatically after all threads have arrived.
*/
class Barrier {
public:
  explicit Barrier(std::size_t required_thread_count)
    : required_thread_count(required_thread_count)
  {
    if (required_thread_count == 0) {
      throw std::invalid_argument("Barrier requires at least one thread");
    }
  }

  void wait() {
    std::unique_lock<std::mutex> lock(mutex);
    const std::uint64_t current_generation = generation;

    if (++arrived_thread_count == required_thread_count) {
      arrived_thread_count = 0;
      ++generation;
      lock.unlock();
      condition.notify_all();
    }
    else {
      condition.wait(lock, [&] {
        return generation != current_generation;
        });
    }
  }

private:
  const std::size_t required_thread_count;
  std::size_t arrived_thread_count{0};
  std::uint64_t generation{0};

  std::mutex mutex;
  std::condition_variable condition;
};
