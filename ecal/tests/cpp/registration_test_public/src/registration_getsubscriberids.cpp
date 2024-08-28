/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2024 Continental Corporation
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

#include <gtest/gtest.h>

#include <vector>
#include <string>
#include <atomic>
#include <set>

enum {
  CMN_REGISTRATION_REFRESH_MS = (1000)
};

// Struct to hold the test parameters
struct TestParams
{
  int subscriber_count = 0;
  eCAL::Configuration configuration;
};

// Test class that accepts TestParams as a parameter
class EcalTest : public ::testing::TestWithParam<TestParams>
{
protected:
  void SetUp() override
  {
    // Set configuration from the test parameters
    auto params = GetParam();
    eCAL::Initialize(params.configuration, "core_cpp_registration_subscriberids");
  }

  void TearDown() override
  {
    // Clean up
    eCAL::Finalize();
  }
};

TEST_P(EcalTest, GetSubscriberIDsReturnsCorrectNumber)
{
  {
    // Create subscribers for testing
    std::vector<eCAL::CSubscriber> subscriber_vec;
    for (int i = 0; i < GetParam().subscriber_count; ++i)
    {
      std::stringstream tname;
      tname << "topic_" << i;

      subscriber_vec.emplace_back(tname.str());
    }

    // Register and wait
    eCAL::Process::SleepMS(2 * GetParam().configuration.registration.registration_refresh);

    // Get the list of subscriber IDs
    const auto sub_ids1 = eCAL::Registration::GetSubscriberIDs();

    // Verify the number of subscribers created
    ASSERT_EQ(sub_ids1.size(), GetParam().subscriber_count);
  }

  // Wait for timeout
  eCAL::Process::SleepMS(2 * GetParam().configuration.registration.registration_timeout);

  // Get the list of subscriber IDs again
  const auto sub_ids2 = eCAL::Registration::GetSubscriberIDs();

  // Verify the number of subscribers still existing
  ASSERT_EQ(sub_ids2.size(), 0);
}

TEST_P(EcalTest, SubscriberEventCallbackIsTriggered)
{
  std::atomic<size_t> created_subscriber_num(0);
  std::atomic<size_t> deleted_subscriber_num(0);
  std::set<eCAL::Registration::STopicId> created_subscriber_ids;
  std::set<eCAL::Registration::STopicId> deleted_subscriber_ids;

  // Register the callback
  auto callback_token = eCAL::Registration::AddSubscriberEventCallback(
    [&](const eCAL::Registration::STopicId& id, eCAL::Registration::RegistrationEventType event_type)
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
    // Create subscribers for testing
    std::vector<eCAL::CSubscriber> subscriber_vec;
    for (int i = 0; i < GetParam().subscriber_count; ++i)
    {
      std::stringstream tname;
      tname << "topic_" << i;

      subscriber_vec.emplace_back(tname.str());
    }

    // Register and wait
    eCAL::Process::SleepMS(2 * GetParam().configuration.registration.registration_refresh);

    // Verify the number of subscribers created through the callback
    ASSERT_EQ(created_subscriber_num.load(), GetParam().subscriber_count);

    // Clear subscribers to trigger deletion events
    subscriber_vec.clear();

    // Register the deletion events and wait
    eCAL::Process::SleepMS(2 * GetParam().configuration.registration.registration_refresh);

    // Verify the number of subscribers deleted through the callback
    ASSERT_EQ(deleted_subscriber_num.load(), GetParam().subscriber_count);
  }

  // Unregister the callback
  eCAL::Registration::RemSubscriberEventCallback(callback_token);
}

// Instantiate the test suite with different configurations and subscriber counts
INSTANTIATE_TEST_SUITE_P(
  GetSubscriberIDTests,
  EcalTest,
  ::testing::Values(
    TestParams{ 10, []() {
      // Shared Memory (shm)
      eCAL::Configuration config;
      config.registration.registration_timeout = 2000;
      config.registration.layer.shm.enable = true;
      config.registration.layer.udp.enable = false;
      return config;
    }() },
    TestParams{ 10, []() {
      // UDP
      eCAL::Configuration config;
      config.registration.registration_timeout = 2000;
      config.registration.layer.shm.enable = false;
      config.registration.layer.udp.enable = true;
      return config;
    }() }
  )
);
