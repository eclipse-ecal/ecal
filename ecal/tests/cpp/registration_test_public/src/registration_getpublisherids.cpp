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
#include <set>
#include <sstream>
#include <string>
#include <vector>

// struct to hold the test parameters
struct TestParams
{
  int publisher_count = 0;
  eCAL::Configuration configuration;
};

// test class that accepts TestParams as a parameter
class TestFixture : public ::testing::TestWithParam<TestParams>
{
protected:
  void SetUp() override
  {
    // set configuration from the test parameters
    auto params = GetParam();
    eCAL::Initialize(params.configuration, "core_cpp_registration_publisherids");
  }

  void TearDown() override
  {
    // clean up
    eCAL::Finalize();
  }
};

TEST_P(TestFixture, GetPublisherIDsReturnsCorrectNumber)
{
  {
    // create publishers for testing
    std::vector<eCAL::CPublisher> publisher_vec;
    for (int i = 0; i < GetParam().publisher_count; ++i)
    {
      std::stringstream topic_name;
      topic_name << "topic_" << i;

      eCAL::SDataTypeInformation data_type_info;
      data_type_info.name       = topic_name.str() + "_type_name";
      data_type_info.encoding   = topic_name.str() + "_type_encoding";
      data_type_info.descriptor = topic_name.str() + "_type_descriptor";

      publisher_vec.emplace_back(topic_name.str(), data_type_info);
    }

    // let's register
    eCAL::Process::SleepMS(2 * GetParam().configuration.registration.registration_refresh);

    // get the list of publisher IDs
    std::set<eCAL::STopicId> pub_ids1;
    const auto get_publisher_ids_succeeded = eCAL::Registration::GetPublisherIDs(pub_ids1);

    // verify the number of publishers created
    EXPECT_TRUE(get_publisher_ids_succeeded) << "GetPublisherIDs call failed";
    ASSERT_EQ(pub_ids1.size(), GetParam().publisher_count);
  }

  // let's finally timeout
  eCAL::Process::SleepMS(GetParam().configuration.registration.registration_timeout);

  // get the list of publisher IDs
  std::set<eCAL::STopicId> pub_ids2;
  const auto get_publisher_ids_succeeded = eCAL::Registration::GetPublisherIDs(pub_ids2);

  // all publisher should be timeouted
  EXPECT_TRUE(get_publisher_ids_succeeded) << "GetPublisherIDs call failed";
  ASSERT_EQ(pub_ids2.size(), 0);
}

TEST_P(TestFixture, PublisherEventCallbackIsTriggered)
{
  std::atomic<size_t> created_publisher_num(0);
  std::atomic<size_t> deleted_publisher_num(0);
  std::set<eCAL::STopicId> created_publisher_ids;
  std::set<eCAL::STopicId> deleted_publisher_ids;

  // register the callback
  auto callback_token = eCAL::Registration::AddPublisherEventCallback(
    [&](const eCAL::STopicId& id, eCAL::Registration::RegistrationEventType event_type)
    {
      if (event_type == eCAL::Registration::RegistrationEventType::new_entity)
      {
        created_publisher_num++;
        created_publisher_ids.insert(id);
      }
      else if (event_type == eCAL::Registration::RegistrationEventType::deleted_entity)
      {
        deleted_publisher_num++;
        deleted_publisher_ids.insert(id);
      }
    });

  {
    // create publishers for testing
    std::vector<eCAL::CPublisher> publisher_vec;
    for (int i = 0; i < GetParam().publisher_count; ++i)
    {
      std::stringstream topic_name;
      topic_name << "topic_" << i;

      eCAL::SDataTypeInformation data_type_info;
      data_type_info.name       = topic_name.str() + "_type_name";
      data_type_info.encoding   = topic_name.str() + "_type_encoding";
      data_type_info.descriptor = topic_name.str() + "_type_descriptor";

      publisher_vec.emplace_back(topic_name.str(), data_type_info);
    }

    // let's register
    eCAL::Process::SleepMS(2 * GetParam().configuration.registration.registration_refresh);

    // verify the number of publishers created through the callback
    ASSERT_EQ(created_publisher_num.load(), GetParam().publisher_count);

    // clear publishers to trigger deletion events
    publisher_vec.clear();

    // let's register the deletion events
    eCAL::Process::SleepMS(2 * GetParam().configuration.registration.registration_refresh);

    // verify the number of publishers deleted through the callback
    ASSERT_EQ(deleted_publisher_num.load(), GetParam().publisher_count);
  }

  // unregister the callback
  eCAL::Registration::RemPublisherEventCallback(callback_token);
}

// instantiate the test suite with different configurations and publisher counts
INSTANTIATE_TEST_SUITE_P(
  core_cpp_registration_public,
  TestFixture,
  ::testing::Values(
    TestParams{ 10, []() {
      // shm
      eCAL::Configuration config;
      config.registration.registration_refresh = 100;
      config.registration.registration_timeout = 200;
      config.registration.layer.shm.enable     = true;
      config.registration.layer.udp.enable     = false;
      return config;
    }() },
    TestParams{ 10, []() {
      // shm + shm transport domain
      eCAL::Configuration config;
      config.registration.layer.shm.enable     = true;
      config.registration.layer.udp.enable     = false;
      config.registration.shm_transport_domain = "abc";
      return config;
    }() },
    TestParams{ 10, []() {
      // udp
      eCAL::Configuration config;
      config.registration.registration_refresh = 100;
      config.registration.registration_timeout = 200;
      config.registration.layer.shm.enable     = false;
      config.registration.layer.udp.enable     = true;
      return config;
    }() }
  )
);
