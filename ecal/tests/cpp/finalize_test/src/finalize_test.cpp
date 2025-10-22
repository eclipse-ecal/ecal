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

#include <chrono>
#include <cstddef>
#include <ecal/ecal.h>
#include <ecal/pubsub/publisher.h>
#include <ecal/pubsub/subscriber.h>

#include <atomic>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include <gtest/gtest.h>

constexpr size_t CMN_REGISTRATION_REFRESH_MS = 1000;

TEST(core_cpp_finalize, finalize_with_segfault_provocation)
{
  // initialize eCAL API
  EXPECT_EQ(true, eCAL::Initialize("finalize"));

  unsigned int receive_counter = 0;
  // create subscriber and register a callback
  eCAL::CSubscriber sub("foo");
  sub.SetReceiveCallback([&receive_counter](const eCAL::STopicId& /*topic_id*/, const eCAL::SDataTypeInformation& /*datatype_info_*/, const eCAL::SReceiveCallbackData& /*data_*/)
  {
    std::cout << receive_counter++ << std::endl;
    return;
  });

  std::vector<eCAL::CPublisher> pubs;
  
  for (size_t i = 0; i < 100; ++i)
  {
    pubs.emplace_back("foo");
  }

  // let's match them
  eCAL::Process::SleepMS(10 * CMN_REGISTRATION_REFRESH_MS);

  std::atomic<bool> pub_stop(false);

  auto send_function = [&pubs, &pub_stop, &receive_counter]() {
    while(pub_stop == false)
    {
      for (auto& pub : pubs)
      {
        pub.Send("Hello from send thread");
      }
    }
  };

  std::thread send_thread(send_function);

  // finalize eCAL API
  // without destroying any pub / sub
  EXPECT_EQ(true, eCAL::Finalize());

  std::this_thread::sleep_for(std::chrono::seconds(2));

  EXPECT_EQ(true, eCAL::Initialize("finalize"));

  std::this_thread::sleep_for(std::chrono::seconds(2));

  pub_stop.exchange(true);
  send_thread.join();
}