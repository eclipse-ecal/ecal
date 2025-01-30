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

#include <cstddef>
#include <ecal/ecal.h>

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <map>
#include <memory>
#include <string>
#include <thread>

enum {
  CMN_MONITORING_TIMEOUT_MS   = (5000 + 100),
  CMN_REGISTRATION_REFRESH_MS = (1000)
};

TEST(core_cpp_registration_public, GetTopics)
{
  // initialize eCAL API
  eCAL::Initialize("core_cpp_registration_public");

  std::set<eCAL::STopicId> topic_id_pub_set;
  std::set<eCAL::STopicId> topic_id_sub_set;
  bool get_publisher_ids_succeeded;
  bool get_subscriber_ids_succeeded;

  // create and check a few pub/sub entities
  {
    eCAL::SDataTypeInformation info_A1  { "typeA1"  ,"",  "descA1"   };
    eCAL::SDataTypeInformation info_A2  { "typeA2"  ,"",  "descA2"   };
    eCAL::SDataTypeInformation info_A3  { "typeA3"  ,"",  "descA3"   };

    eCAL::SDataTypeInformation info_B1  { "typeB1"  ,"",  "descB1"   };
    eCAL::SDataTypeInformation info_B2  { "typeB2"  ,"",  "descB2"   };

    // create 3 publisher
    auto pub1 = std::make_shared<eCAL::CPublisher>("A1", info_A1);
    auto pub2 = std::make_shared<eCAL::CPublisher>("A2", info_A2);
    auto pub3 = std::make_shared<eCAL::CPublisher>("A3", info_A3);

    // create 2 subscriber
    auto sub1 = std::make_shared<eCAL::CSubscriber>("B1", info_B1);
    auto sub2 = std::make_shared<eCAL::CSubscriber>("B2", info_B2);

    // let's register
    eCAL::Process::SleepMS(2 * CMN_REGISTRATION_REFRESH_MS);

    // get all publisher topics and check size
    get_publisher_ids_succeeded = eCAL::Registration::GetPublisherIDs(topic_id_pub_set);
    EXPECT_TRUE(get_publisher_ids_succeeded) << "GetPublisherIDs call failed";
    EXPECT_EQ(topic_id_pub_set.size(), 3);

    // check publisher types and descriptions
    for (auto& topic_id_pub : topic_id_pub_set)
    {
      eCAL::SDataTypeInformation registration_topic_info;
      eCAL::Registration::GetPublisherInfo(topic_id_pub, registration_topic_info);
      eCAL::SDataTypeInformation expected_topic_info{ "type" + topic_id_pub.topic_name, "", "desc" + topic_id_pub.topic_name };
      EXPECT_EQ(registration_topic_info, expected_topic_info);
    }

    // get all subscriber topics and check size
    get_publisher_ids_succeeded = eCAL::Registration::GetSubscriberIDs(topic_id_sub_set);
    EXPECT_TRUE(get_publisher_ids_succeeded) << "GetSubscriberIDs call failed";
    EXPECT_EQ(topic_id_sub_set.size(), 2);

    // check subscriber types and descriptions
    for (auto& topic_id_sub : topic_id_sub_set)
    {
      eCAL::SDataTypeInformation registration_topic_info;
      eCAL::Registration::GetSubscriberInfo(topic_id_sub, registration_topic_info);
      eCAL::SDataTypeInformation expected_topic_info{ "type" + topic_id_sub.topic_name, "", "desc" + topic_id_sub.topic_name };
      EXPECT_EQ(registration_topic_info, expected_topic_info);
    }

    // wait a monitoring timeout long,
    eCAL::Process::SleepMS(CMN_MONITORING_TIMEOUT_MS);

    // get all publisher topics and check size (should not be expired)
    get_publisher_ids_succeeded = eCAL::Registration::GetPublisherIDs(topic_id_pub_set);
    EXPECT_TRUE(get_publisher_ids_succeeded) << "GetPublisherIDs call failed";
    EXPECT_EQ(topic_id_pub_set.size(), 3);

    // get all subscriber topics and check size (should not be expired)
    get_subscriber_ids_succeeded = eCAL::Registration::GetSubscriberIDs(topic_id_sub_set);
    EXPECT_TRUE(get_subscriber_ids_succeeded) << "GetSubscriberIDs call failed";
    EXPECT_EQ(topic_id_sub_set.size(), 2);

    // now destroy publisher pub1 and subscriber sub1
    // the entities pub12 and sub12 should replace them
    // by overwriting their type names and descriptions
    pub1.reset();
    sub1.reset();

    // let's register
    eCAL::Process::SleepMS(2 * CMN_REGISTRATION_REFRESH_MS);

    // get all publisher topics and check size (pub1 should be expired)
    get_publisher_ids_succeeded = eCAL::Registration::GetPublisherIDs(topic_id_pub_set);
    EXPECT_TRUE(get_publisher_ids_succeeded) << "GetPublisherIDs call failed";
    EXPECT_EQ(topic_id_pub_set.size(), 2);

    // get all subscriber topics and check size (sub1 should be expired)
    get_subscriber_ids_succeeded = eCAL::Registration::GetSubscriberIDs(topic_id_sub_set);
    EXPECT_TRUE(get_subscriber_ids_succeeded) << "GetSubscriberIDs call failed";
    EXPECT_EQ(topic_id_sub_set.size(), 1);
  }

  // let's unregister
  eCAL::Process::SleepMS(2 * CMN_REGISTRATION_REFRESH_MS);

  // get all publisher topics and check size (all publisher should be expired)
  get_publisher_ids_succeeded = eCAL::Registration::GetPublisherIDs(topic_id_pub_set);
  EXPECT_TRUE(get_publisher_ids_succeeded) << "GetPublisherIDs call failed";
  EXPECT_EQ(topic_id_pub_set.size(), 0);

  // get all subscriber topics and check size (all subscriber should be expired)
  get_subscriber_ids_succeeded = eCAL::Registration::GetSubscriberIDs(topic_id_sub_set);
  EXPECT_TRUE(get_subscriber_ids_succeeded) << "GetSubscriberIDs call failed";
  EXPECT_EQ(topic_id_sub_set.size(), 0);

  // finalize eCAL API
  eCAL::Finalize();
}

// This test creates a real big number of publishers.
// It then checks, if they have all been seen using GetTopics()
// And the count is back to 0 upon completion.
TEST(core_cpp_registration_public, GetTopicsParallel)
{
  constexpr const int max_publisher_count(2000);
  constexpr const int waiting_time_thread(4000);
  constexpr const int parallel_threads(1);

  std::atomic<bool> testing_completed{ false };

  // initialize eCAL API
  eCAL::Initialize("core_cpp_registration_public");

  auto create_publishers = [&]() {
    std::string topic_name = "Test.ParallelUtilFunctions";
    std::atomic<int> call_back_count{ 0 };
    {
      std::vector<std::unique_ptr<eCAL::CPublisher>> publishers;
      for (int pub_count = 0; pub_count < max_publisher_count; pub_count++) {
        std::unique_ptr<eCAL::CPublisher> publisher = std::make_unique<eCAL::CPublisher>(topic_name + std::to_string(pub_count));
        publishers.push_back(std::move(publisher));
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(waiting_time_thread));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(waiting_time_thread));
    testing_completed = true;
    };

  auto get_topics_from_ecal = [&]() {
    size_t number_publishers_seen = 0;
    size_t max_number_publishers_seen = 0;

    std::set<eCAL::STopicId> tmp_publisher_ids;

    do {
      std::set<eCAL::STopicId> tmp_publisher_ids;
      eCAL::Registration::GetPublisherIDs(tmp_publisher_ids);

      number_publishers_seen = tmp_publisher_ids.size();
      max_number_publishers_seen = std::max(max_number_publishers_seen, number_publishers_seen);
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
    } while (!testing_completed);

    EXPECT_EQ(number_publishers_seen, 0);
    EXPECT_EQ(max_number_publishers_seen, max_publisher_count);
    };

  std::vector<std::thread> threads_container;
  threads_container.push_back(std::thread(create_publishers));

  for (size_t i = 0; i < parallel_threads; i++) {
    threads_container.push_back(std::thread(get_topics_from_ecal));
  }

  for (auto& th : threads_container) {
    th.join();
  }

  // finalize eCAL API
  eCAL::Finalize();
}
