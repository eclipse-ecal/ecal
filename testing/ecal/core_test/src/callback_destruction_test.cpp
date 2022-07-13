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
  void OnReceive(const std::string& /*data_*/, long long clock_)
  {
    static long long accumulated_clock = 0;
    accumulated_clock += clock_;
  }
}


TEST(Core, CallbackDestruction)
{
  // initialize eCAL API
  EXPECT_EQ(0, eCAL::Initialize(0, nullptr, "callback destruction"));

  // enable loop back communication in the same thread
  eCAL::Util::EnableLoopback(true);

  // create subscriber and register a callback
  std::shared_ptr< eCAL::string::CSubscriber<std::string>> sub;

  // start publishing thread
  eCAL::string::CPublisher<std::string> pub("foo");
  std::atomic<bool> pub_stop(false);
  std::thread pub_t([&]() {
  while (!pub_stop)
  {
    int y = 0;
    pub.Send("Hello World");
    // some kind of busy waiting....
    for (int i = 0; i < 1000; i++)
    {
      y += i;
    }
  }
  });

  std::atomic<bool> sub_stop(false);
  std::thread sub_t([&]() {
    while (!sub_stop)
    {
      sub = std::make_shared<eCAL::string::CSubscriber<std::string>>("foo");
      sub->AddReceiveCallback(std::bind(OnReceive, std::placeholders::_2, std::placeholders::_4));
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