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

namespace {
  // subscriber callback function
  void OnReceiveLazy(const char* topic_name_, const std::string& msg_, long long time_, long long clock_, long long id_)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
  }

  void OnReceiveCrazy(const char* topic_name_, const std::string& msg_, long long time_, long long clock_, long long id_)
  {
    static long long last_clock = 0;
    EXPECT_LE(last_clock, clock_);
    last_clock = clock_;
  }
}


TEST(Core, OutOfOrderMessages)
{
  // initialize eCAL API
  EXPECT_EQ(0, eCAL::Initialize(0, nullptr, "callback destruction"));

  // enable loop back communication in the same thread
  eCAL::Util::EnableLoopback(true);

  // start publishing thread
  eCAL::string::CPublisher<std::string> pub("foo");
  eCAL::string::CSubscriber<std::string> sub_lazy("foo");
  sub_lazy.AddReceiveCallback(OnReceiveLazy);
  eCAL::string::CSubscriber<std::string> sub_crazy("foo");
  sub_crazy.AddReceiveCallback(OnReceiveCrazy);


  std::atomic<bool> pub_stop(false);
  std::thread pub_t([&]() {
    while (!pub_stop)
    {
      pub.Send("Hello World");
      // some kind of busy waiting....
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    });

  std::atomic<bool> sub_lazy_stop(false);
  std::thread sub_lazy_t([&]() {
    while (!sub_lazy_stop)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    });

  std::atomic<bool> sub_crazy_stop(false);
  std::thread sub_crazy_t([&]() {
    while (!sub_crazy_stop)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    });
  
  // let them work together
  std::this_thread::sleep_for(std::chrono::seconds(10));

  // stop publishing thread
  pub_stop = true;
  pub_t.join();

  sub_lazy_stop = true;
  sub_lazy_t.join();

  sub_crazy_stop = true;
  sub_crazy_t.join();

  // finalize eCAL API
  // without destroying any pub / sub
  EXPECT_EQ(0, eCAL::Finalize());

}