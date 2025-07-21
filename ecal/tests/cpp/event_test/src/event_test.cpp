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

#include "ecal_event.h"

#include <gtest/gtest.h>
#include <string>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <thread>

class Barrier 
{
public:
  Barrier(int count) : thread_count(count), waiting(0), step(0) 
  {}

  void wait() 
  {
    std::unique_lock<std::mutex> lock(mtx);
    int current_step = step;

    if (++waiting == thread_count) {
      waiting = 0;       // Reset for the next use
      ++step;            // Increment the step to release all threads
      cv.notify_all();   // Release all waiting threads
    }
    else {
      cv.wait(lock, [this, current_step] { return current_step != step; });
    }
  }

private:
  int thread_count;
  std::atomic<int> waiting;
  int step;
  std::mutex mtx;
  std::condition_variable cv;
};


TEST(core_cpp_core, Event_EventSetGet)
{ 
  // global parameter
  const std::string event_name = "my_event";

  // create named event
  eCAL::EventHandleT event_handle;
  EXPECT_EQ(true, eCAL::gOpenEvent(&event_handle, event_name));

  // get none set event
  EXPECT_EQ(false, gWaitForEvent(event_handle, 10));

  // set event
  EXPECT_EQ(true, gSetEvent(event_handle));

  // get set event
  EXPECT_EQ(true, gWaitForEvent(event_handle, 100));
}

TEST(core_cpp_core, Event_OpenEventInParallel)
{
  // parameter
  const std::string event_name = "my_parallel_event";
  const int runs = 10000;

  Barrier barrier(2);
  
  auto event_worker = [&barrier, &event_name, runs](bool has_ownership)
  {
      for (int i = 0; i < runs; ++i)
      {
        eCAL::EventHandleT event_handle;
        barrier.wait();
        eCAL::gOpenNamedEvent(&event_handle, event_name, has_ownership);
        ASSERT_NE(eCAL::gGetNativeEventHandle(event_handle), nullptr);
        barrier.wait();
        eCAL::gCloseEvent(event_handle);
      }
  };

  std::thread event_worker_thread_1(event_worker, true);
  std::thread event_worker_thread_2(event_worker, false);
  event_worker_thread_1.join();
  event_worker_thread_2.join();
}
