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
#include <ecal/ecal.h>
#include <ecal/pubsub/publisher.h>
#include <ecal/pubsub/subscriber.h>

#include <functional>
#include <memory>
#include <string>
#include <thread>

#include <gtest/gtest.h>

namespace
{
  std::chrono::nanoseconds TimeOperation(const std::function<void()>& func)
  {
    auto start = std::chrono::steady_clock::now();
    func();
    auto end = std::chrono::steady_clock::now();
    return end - start;
  }

  template <typename Rep, typename Period>
  void AssertOperationExecutionTimeInRange(const std::function<void()>& func, std::chrono::duration<Rep, Period> min, std::chrono::duration<Rep, Period> max)
  {
    auto operation_time = TimeOperation(func);
    EXPECT_GE(operation_time.count(), std::chrono::duration_cast<std::chrono::nanoseconds>(min).count()) << "Timed operation less than minimum threshold";
    EXPECT_LE(operation_time.count(), std::chrono::duration_cast<std::chrono::nanoseconds>(max).count()) << "Timed operation greater than maximum threshold";
  }
}

// This test asserts that a timeouted acknowledge does not break subsequent calls
TEST(core_cpp_pubsub, TimeoutAcknowledgment)
{
  // initialize eCAL API
  EXPECT_EQ(true, eCAL::Initialize("TimeoutAcknowledgment", eCAL::Init::All));

  // create publisher config
  eCAL::Publisher::Configuration pub_config;
  pub_config.layer.shm.acknowledge_timeout_ms = 500;

  // create publisher
  eCAL::CPublisher pub("topic", {}, pub_config);
  auto sub1 = std::make_shared< eCAL::CSubscriber>("topic");
  auto sleeper_variable_time = [](const eCAL::STopicId& /*topic_id_*/, const eCAL::SDataTypeInformation&  /*data_type_info_*/, const eCAL::SReceiveCallbackData& data_)
                                {
                                  std::string const sleep_time((const char*)data_.buffer, data_.buffer_size);
                                  int const sleep = std::stoi(sleep_time);
                                  std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
                                };

  sub1->SetReceiveCallback(sleeper_variable_time);

  // Registration activities
  std::this_thread::sleep_for(std::chrono::seconds(2));

  // Regular call with acknowledge should take between 9 and 12 ms.
  for (int i = 0; i < 5; ++i)
  {
    AssertOperationExecutionTimeInRange([&pub]()
                                        {
                                          auto send = pub.Send("100");
                                          EXPECT_TRUE(send);
                                        }
                                        , std::chrono::milliseconds(99)
                                        , std::chrono::milliseconds(140)
    );
  }

  AssertOperationExecutionTimeInRange([&pub]()
                                      {
                                        auto send = pub.Send("600");
                                        EXPECT_TRUE(send);
                                      }
                                      , std::chrono::milliseconds(499)
                                      , std::chrono::milliseconds(550)
  );

  for (int i = 0; i < 5; ++i)
  {
    auto now = std::chrono::steady_clock::now();
    AssertOperationExecutionTimeInRange([&pub]()
                                        {
                                          auto send = pub.Send("100");
                                          EXPECT_TRUE(send);
                                        }
                                        , std::chrono::milliseconds(0)
                                        , std::chrono::milliseconds(140)
    );
    std::this_thread::sleep_until(now + std::chrono::milliseconds(200));
  }

  // finalize eCAL API
  // without destroying any pub / sub
  EXPECT_EQ(true, eCAL::Finalize());
}
