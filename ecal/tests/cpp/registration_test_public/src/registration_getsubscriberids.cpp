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
  int subscriber_count = 0;
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
    eCAL::Initialize(params.configuration, "core_cpp_registration_subscriberids");
  }

  void TearDown() override
  {
    // clean up
    eCAL::Finalize();
  }
};

TEST_P(TestFixture, GetSubscriberIDsReturnsCorrectNumber)
{
  {
    // create subscribers for testing
    std::vector<eCAL::CSubscriber> subscriber_vec;
    for (int i = 0; i < GetParam().subscriber_count; ++i)
    {
      std::stringstream topic_name;
      topic_name << "topic_" << i;

      eCAL::SDataTypeInformation data_type_info;
      data_type_info.name       = topic_name.str() + "_type_name";
      data_type_info.encoding   = topic_name.str() + "_type_encoding";
      data_type_info.descriptor = topic_name.str() + "_type_descriptor";

      subscriber_vec.emplace_back(topic_name.str(), data_type_info);
    }

    // let's register
    eCAL::Process::SleepMS(2 * GetParam().configuration.registration.registration_refresh);

    // get the list of subscriber IDs
    std::set<eCAL::STopicId> sub_ids1;
    const bool call_successful = eCAL::Registration::GetSubscriberIDs(sub_ids1);

    // verify the number of subscribers created
    ASSERT_TRUE(call_successful) << "GetSubscriberIDs returned false";
    ASSERT_EQ(sub_ids1.size(), GetParam().subscriber_count);
  }

  // let's finally timeout
  eCAL::Process::SleepMS(2 * GetParam().configuration.registration.registration_timeout);

  // get the list of subscriber IDs
  std::set<eCAL::STopicId> sub_ids2;
  const bool call_successful = eCAL::Registration::GetSubscriberIDs(sub_ids2);

  // all subscriber should be timeouted
  ASSERT_TRUE(call_successful) << "GetSubscriberIDs returned false";
  ASSERT_EQ(sub_ids2.size(), 0);
}

TEST_P(TestFixture, SubscriberEventCallbackIsTriggered)
{
  std::atomic<size_t> created_subscriber_num(0);
  std::atomic<size_t> deleted_subscriber_num(0);
  std::set<eCAL::STopicId> created_subscriber_ids;
  std::set<eCAL::STopicId> deleted_subscriber_ids;

  // register the callback
  auto callback_token = eCAL::Registration::AddSubscriberEventCallback(
    [&](const eCAL::STopicId& id, eCAL::Registration::RegistrationEventType event_type)
    {
      if (event_type == eCAL::Registration::RegistrationEventType::new_entity)
      {
        created_subscriber_num++;
        created_subscriber_ids.insert(id);
      }
      else if (event_type == eCAL::Registration::RegistrationEventType::deleted_entity)
      {
        deleted_subscriber_num++;
        deleted_subscriber_ids.insert(id);
      }
    });

  {
    // create subscribers for testing
    std::vector<eCAL::CSubscriber> subscriber_vec;
    for (int i = 0; i < GetParam().subscriber_count; ++i)
    {
      std::stringstream topic_name;
      topic_name << "topic_" << i;

      eCAL::SDataTypeInformation data_type_info;
      data_type_info.name       = topic_name.str() + "_type_name";
      data_type_info.encoding   = topic_name.str() + "_type_encoding";
      data_type_info.descriptor = topic_name.str() + "_type_descriptor";

      subscriber_vec.emplace_back(topic_name.str(), data_type_info);
    }

    // let's register
    eCAL::Process::SleepMS(2 * GetParam().configuration.registration.registration_refresh);

    // verify the number of subscribers created through the callback
    ASSERT_EQ(created_subscriber_num.load(), GetParam().subscriber_count);

    // clear subscribers to trigger deletion events
    subscriber_vec.clear();

    // let's register the deletion events
    eCAL::Process::SleepMS(2 * GetParam().configuration.registration.registration_refresh);

    // verify the number of subscribers deleted through the callback
    ASSERT_EQ(deleted_subscriber_num.load(), GetParam().subscriber_count);
  }

  // unregister the callback
  eCAL::Registration::RemSubscriberEventCallback(callback_token);
}

// instantiate the test suite with different configurations and subscriber counts
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
    TestParams{10, []() {
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
