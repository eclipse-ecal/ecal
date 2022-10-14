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

#include <atomic>
#include <string>
#include <sstream>
#include <thread>

#include <gtest/gtest.h>

#define CMN_REGISTRATION_REFRESH 1000

// This tests test for sporadically received empty messages which were a problem.
TEST(SUBSCRIBER, SPORADIC_EMPTY_RECEIVES)
{ 
  // initialize eCAL API
  EXPECT_EQ(0, eCAL::Initialize(0, nullptr, "inproc_clock_test"));

  // publish / subscribe match in the same process
  eCAL::Util::EnableLoopback(true);

  // create simple string publisher
  eCAL::string::CPublisher<std::string> pub("CLOCK");
  eCAL::string::CSubscriber<std::string> sub("CLOCK");

  // let's match them
  eCAL::Process::SleepMS(2 * CMN_REGISTRATION_REFRESH);

  // start publishing thread
  std::atomic<bool> pub_stop(false);
  std::thread pub_t([&pub, &pub_stop]() {
    std::string abc{ "abc" };
    while (!pub_stop)
    {

      pub.Send(abc);
      std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    });

  std::atomic<bool> sub_stop(false);
  std::thread sub_t([&]() {
    std::string received;
    while (!sub_stop)
    {
      bool got_data = sub.Receive(received);
      if (got_data && received.empty())
      {
        FAIL() << "received empty string";
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
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
  EXPECT_EQ(0, eCAL::Finalize());
}
