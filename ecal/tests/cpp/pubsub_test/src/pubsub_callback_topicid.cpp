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

#include <gtest/gtest.h>
#include <configuration_helper.h>
#include <ostream>
#include <string>
#include <thread>
#include <vector>

enum {
  DATA_FLOW_TIME_MS = 100
};

TEST_P(TestFixture, OnePubSub)
{
  eCAL::SDataTypeInformation datatype_info;
  datatype_info.name       = "mytype";
  datatype_info.encoding   = "test";
  datatype_info.descriptor = "desc";

  eCAL::CPublisher  publisher ("foo", datatype_info);
  eCAL::CSubscriber subscriber("foo", datatype_info);

  eCAL::STopicId callback_topic_id;
  eCAL::SDataTypeInformation   callback_datatype_info;
  int                          callback_count{ 0 };

  subscriber.SetReceiveCallback([&callback_topic_id, &callback_datatype_info, &callback_count](const eCAL::STopicId& topic_id_, const eCAL::SDataTypeInformation& datatype_info_, const eCAL::SReceiveCallbackData&)
    {
      ++callback_count;
      callback_topic_id = topic_id_;
      callback_datatype_info = datatype_info_;
    }
  );
  const auto pub_id = publisher.GetTopicId();

  // let them match
  eCAL::Process::SleepMS(2 * config.registration.registration_refresh);
  // send data
  publisher.Send("abc");
  // make sure data was received
  std::this_thread::sleep_for(std::chrono::milliseconds(DATA_FLOW_TIME_MS));

  EXPECT_EQ(callback_topic_id, pub_id);
  EXPECT_EQ(callback_datatype_info, datatype_info);
}

TEST_P(TestFixture, MultiplePubSub)
{
  const int num_publishers = 4;

  std::vector<eCAL::CPublisher> publishers;
  for (int i = 0; i < num_publishers; ++i)
  {
    eCAL::SDataTypeInformation datatype_info;
    datatype_info.name = "mytype";
    datatype_info.encoding = "test";
    datatype_info.descriptor = "desc" + std::to_string(i);

    publishers.emplace_back("foo", datatype_info);
  }
  eCAL::CSubscriber subscriber("foo");

  eCAL::STopicId callback_topic_id;
  eCAL::SDataTypeInformation   callback_datatype_info;
  int                          callback_count{ 0 };

  subscriber.SetReceiveCallback([&callback_topic_id, &callback_datatype_info, &callback_count](const eCAL::STopicId& topic_id_, const eCAL::SDataTypeInformation& datatype_info_, const eCAL::SReceiveCallbackData&)
    {
      ++callback_count;
      callback_topic_id = topic_id_;
      callback_datatype_info = datatype_info_;
    }
  );

  // let them match
  eCAL::Process::SleepMS(2 * config.registration.registration_refresh);

  for (int i = 0; i < num_publishers; ++i)
  {
    auto& publisher = publishers[i];
    const auto pub_id = publisher.GetTopicId();
    const auto pub_datatype_info = publisher.GetDataTypeInformation();

    // send data
    publisher.Send("abc");
    // make sure data was received
    std::this_thread::sleep_for(std::chrono::milliseconds(DATA_FLOW_TIME_MS));

    EXPECT_EQ(callback_topic_id, pub_id);
    EXPECT_EQ(callback_datatype_info, pub_datatype_info);
  }
}

TEST_P(TestFixture, DestroyOneOfTwoSubscribers)
{
  int callback_count{ 0 };
  int callback_count_destroyable{ 0 };
  eCAL::CPublisher publisher("foo");
  eCAL::CSubscriber subscriber("foo");
  subscriber.SetReceiveCallback([&callback_count](...) {++callback_count; });
  std::unique_ptr<eCAL::CSubscriber> destroyable_subscriber = std::make_unique<eCAL::CSubscriber>("foo");
  destroyable_subscriber->SetReceiveCallback([&callback_count_destroyable](...) {++callback_count_destroyable; });

  // let them match
  eCAL::Process::SleepMS(2 * config.registration.registration_refresh);
  
  int count = 0;
  while (publisher.GetSubscriberCount() < 2)
  {
    ++count;
    if (count > 50)
    {
      FAIL() << "Couldn't match subscribers, current count" << publisher.GetSubscriberCount();
      break;
    }
    eCAL::Process::SleepMS(100);
  }


  publisher.Send("abc");
  std::this_thread::sleep_for(std::chrono::milliseconds(DATA_FLOW_TIME_MS));
  publisher.Send("abc");
  std::this_thread::sleep_for(std::chrono::milliseconds(DATA_FLOW_TIME_MS));

  // destroy one subscriber
  destroyable_subscriber.reset();
  eCAL::Process::SleepMS(2 * config.registration.registration_refresh);

  publisher.Send("abc");
  std::this_thread::sleep_for(std::chrono::milliseconds(DATA_FLOW_TIME_MS));
  publisher.Send("abc");
  std::this_thread::sleep_for(std::chrono::milliseconds(DATA_FLOW_TIME_MS));

  EXPECT_EQ(callback_count, 4) << "Destroyed subscriber should receive 4 messages";
  EXPECT_EQ(callback_count_destroyable, 2) << "Destroyed subscriber should receive 2 messages";
}

// Define the test parameters
INSTANTIATE_TEST_SUITE_P(
  core_cpp_pubsub_callback_topid_id,
  TestFixture,
  ::testing::Values(
    TestParams{ EnableSHM(GetTestingConfig()) , "Callback TopicID SHM" },
    TestParams{ EnableUDP(GetTestingConfig()) , "Callback TopicID UDP" },
    TestParams{ EnableTCP(GetTestingConfig()) , "Callback TopicID TCP" }
  )
);
