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
#include <ecal/msg/string/publisher.h>
#include <ecal/msg/string/subscriber.h>
#include <algorithm>
#include <atomic>
#include <thread>
#include <vector>
#include <gtest/gtest.h>

#define CMN_REGISTRATION_REFRESH   1000

namespace {
  // subscriber callback function
  void OnReceive(long long clock_)
  {
    static long long accumulated_clock = 0;
    accumulated_clock += clock_;
  }

#if 0
  // timer callback function
  std::atomic_size_t     g_callback_received{ 0 };
  std::vector<long long> g_timer_vec(100);
  void OnTimer()
  {
    if (g_callback_received < g_timer_vec.size()) g_timer_vec[g_callback_received] = eCAL::Time::GetMicroSeconds();
    g_callback_received += 1;
  }
#endif
}

TEST(Core, MultipleInitializeFinalize)
{
  // try to initialize / finalize multiple times
  for (auto i = 0; i < 4; ++i)
  {
    // initialize eCAL API
    EXPECT_EQ(0, eCAL::Initialize(0, nullptr, "multiple initialize/finalize"));

    // finalize eCAL API
    EXPECT_EQ(0, eCAL::Finalize());
  }
}

TEST(Core, LeakedPubSub)
{
  // initialize eCAL API
  EXPECT_EQ(0, eCAL::Initialize(0, nullptr, "leaked pub/sub"));

  // enable loop back communication in the same thread
  eCAL::Util::EnableLoopback(true);

  // create subscriber and register a callback
  eCAL::string::CSubscriber<std::string> sub("foo");
  sub.AddReceiveCallback(std::bind(OnReceive, std::placeholders::_4));

  // create publisher
  eCAL::string::CPublisher<std::string> pub("foo");

  // let's match them
  eCAL::Process::SleepMS(2 * CMN_REGISTRATION_REFRESH);

  // start publishing thread
  std::atomic<bool> pub_stop(false);
  std::thread pub_t([&]() {
    while (!pub_stop)
    {
      pub.Send("Hello World");
#if 0
      // some kind of busy waiting....
      int y = 0;
      for (int i = 0; i < 100000; i++)
      {
        y += i;
      }
#else
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
#endif
    }
  });

  // let them work together
  std::this_thread::sleep_for(std::chrono::seconds(2));

  // finalize eCAL API
  // without destroying any pub / sub
  EXPECT_EQ(0, eCAL::Finalize());

  // stop publishing thread
  pub_stop = true; pub_t.join();
}

TEST(Core, CallbackDestruction)
{
  // initialize eCAL API
  EXPECT_EQ(0, eCAL::Initialize(0, nullptr, "callback destruction"));

  // enable loop back communication in the same thread
  eCAL::Util::EnableLoopback(true);

  // create subscriber and register a callback
  std::shared_ptr< eCAL::string::CSubscriber<std::string>> sub;

  // create publisher
  eCAL::string::CPublisher<std::string> pub("foo");

  // start publishing thread
  std::atomic<bool> pub_stop(false);
  std::thread pub_t([&]() {
    while (!pub_stop)
    {
      pub.Send("Hello World");
#if 0
      // some kind of busy waiting....
      int y = 0;
      for (int i = 0; i < 100000; i++)
      {
        y += i;
      }
#else
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
#endif
    }
    });

  std::atomic<bool> sub_stop(false);
  std::thread sub_t([&]() {
    while (!sub_stop)
    {
      sub = std::make_shared<eCAL::string::CSubscriber<std::string>>("foo");
      sub->AddReceiveCallback(std::bind(OnReceive, std::placeholders::_4));
      std::this_thread::sleep_for(std::chrono::seconds(2));
    }
    });

  // let them work together
  std::this_thread::sleep_for(std::chrono::seconds(10));

  // stop publishing thread
  pub_stop = true;
  pub_t.join();

  sub_stop = true;
  sub_t.join();

  // finalize eCAL API
  // without destroying any pub / sub
  EXPECT_EQ(0, eCAL::Finalize());
}

/* excluded for now, system timer jitter too high */
#if 0
TEST(Core, TimerCallback)
{ 
  // initialize eCAL API
  EXPECT_EQ(0, eCAL::Initialize(0, nullptr, "timer callback"));

  // Is eCAL API initialized ?
  EXPECT_EQ(1, eCAL::IsInitialized());

  std::this_thread::sleep_for(std::chrono::seconds(1));

  eCAL::CTimer timer;
  const int tloop(10);
  timer.Start(tloop, &OnTimer);

  std::this_thread::sleep_for(std::chrono::seconds(1));

  // Expect that the timer callback was called a hundred times +/-1
  EXPECT_TRUE((g_callback_received >= 99) && (g_callback_received <= 101));
  std::cout << std::endl;
  std::cout << "Timer called " << g_callback_received << " times." << std::endl;
  std::cout << std::endl;

  // Calculate loop jitter
  std::vector<long long> timer_vec_diff;
  long long last_time(0);
  for (auto time : g_timer_vec)
  {
    if (last_time) timer_vec_diff.push_back(time - last_time);
    last_time = time;
  }

  // Print out loop times
  //for (auto time : timer_vec_diff) std::cout << static_cast<double>(time) / 1000.0 << " ms" << std::endl;

  // Print out min / max loop times
  auto tmin = *min_element(std::begin(timer_vec_diff), std::end(timer_vec_diff));
  auto tmax = *max_element(std::begin(timer_vec_diff), std::end(timer_vec_diff));
  std::cout << "Min loop time [ms]: " << tmin / 1000.0 << std::endl;
  std::cout << "Max loop time [ms]: " << tmax / 1000.0 << std::endl;
  std::cout << std::endl;

  // check precision loop time +/- 2 ms
  const int max_jitter_ms(2);
  EXPECT_TRUE(tmin > (tloop - max_jitter_ms) * 1000);
  EXPECT_TRUE(tmax < (tloop + max_jitter_ms) * 1000);

  // this is an issue !
  // we need to stop timer before we finalize eCAL API
  // otherwise it will hang forever
  timer.Stop();

  // finalize second time eCAL API we still expect 0 because 
  // but now reference counter is 0 and destruction should be succeeded
  EXPECT_EQ(0, eCAL::Finalize());

  // finalize eCAL API again we expect 1 because yet finalized
  EXPECT_EQ(1, eCAL::Finalize());
}
#endif /* !defined ECAL_OS_WINDOWS */
