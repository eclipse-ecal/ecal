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

#include <gtest/gtest.h>

#include "registration/ecal_registration_timeout_provider.h"
#include "serialization/ecal_struct_sample_registration.h"

eCAL::Registration::Sample pub_foo_process_a_unregister;
eCAL::Registration::Sample pub_foo_process_a_register_1;
eCAL::Registration::Sample pub_foo_process_a_register_2;

eCAL::Registration::Sample sub_foo_process_a_unregister;
eCAL::Registration::Sample sub_foo_process_a_register_1;
eCAL::Registration::Sample sub_foo_process_a_register_2;

eCAL::Registration::Sample sub_foo_process_b_unregister;
eCAL::Registration::Sample sub_foo_process_b_register_1;
eCAL::Registration::Sample sub_foo_process_b_register_2;

// make sure we create unique topic IDs for our testcases
uint64_t getUniqueId()
{
  static uint64_t topic_id = 1;
  return topic_id++;
}

eCAL::Registration::Sample UpdateTopicSample(const eCAL::Registration::Sample& input_)
{
  // vary statistical data
  eCAL::Registration::Sample updated = input_;
  updated.topic.registration_clock = input_.topic.registration_clock + 1;
  updated.topic.data_clock = input_.topic.data_clock + 10;
  return updated;
}

void InitializeAllSamples()
{
  // Publisher 1
  pub_foo_process_a_unregister.cmd_type = eCAL::bct_unreg_publisher;
  pub_foo_process_a_unregister.identifier.host_name = "host0";
  pub_foo_process_a_unregister.identifier.process_id = 1000;
  pub_foo_process_a_unregister.identifier.entity_id = getUniqueId();
  pub_foo_process_a_unregister.topic.shm_transport_domain = "host0";
  pub_foo_process_a_unregister.topic.process_name = "process_a";
  pub_foo_process_a_unregister.topic.topic_name = "foo";
  pub_foo_process_a_register_1.topic.unit_name = "abc";

  pub_foo_process_a_register_1 = pub_foo_process_a_unregister;
  pub_foo_process_a_register_1.cmd_type = eCAL::bct_reg_publisher;
  pub_foo_process_a_register_1.topic.registration_clock = 1;
  pub_foo_process_a_register_1.topic.direction = "publisher";
  pub_foo_process_a_register_1.topic.datatype_information = { "a", "b", "c" };
  pub_foo_process_a_register_1.topic.topic_size = 100;
  pub_foo_process_a_register_1.topic.connections_local = 2;
  pub_foo_process_a_register_1.topic.connections_external = 2;
  pub_foo_process_a_register_1.topic.message_drops = 0;
  pub_foo_process_a_register_1.topic.data_id = 0;
  pub_foo_process_a_register_1.topic.data_clock = 1;
  pub_foo_process_a_register_1.topic.data_frequency = 10;

  pub_foo_process_a_register_2 = UpdateTopicSample(pub_foo_process_a_register_1);

  // Subscriber 1
  sub_foo_process_a_unregister.cmd_type = eCAL::bct_unreg_subscriber;
  sub_foo_process_a_unregister.identifier.host_name = "host0";
  sub_foo_process_a_unregister.identifier.process_id = 1000;
  sub_foo_process_a_unregister.identifier.entity_id = getUniqueId();
  sub_foo_process_a_unregister.topic.shm_transport_domain = "host0";
  sub_foo_process_a_unregister.topic.process_name = "process_a";
  sub_foo_process_a_unregister.topic.topic_name = "foo";
  sub_foo_process_a_register_1.topic.unit_name = "abc";

  sub_foo_process_a_register_1 = sub_foo_process_a_unregister;
  sub_foo_process_a_register_1.cmd_type = eCAL::bct_reg_subscriber;
  sub_foo_process_a_register_1.topic.registration_clock = 1;
  sub_foo_process_a_register_1.topic.direction = "subscriber";
  sub_foo_process_a_register_1.topic.datatype_information = { "a", "b", "c" };
  sub_foo_process_a_register_1.topic.topic_size = 100;
  sub_foo_process_a_register_1.topic.connections_local = 2;
  sub_foo_process_a_register_1.topic.connections_external = 2;
  sub_foo_process_a_register_1.topic.message_drops = 0;
  sub_foo_process_a_register_1.topic.data_id = 0;
  sub_foo_process_a_register_1.topic.data_clock = 1;
  sub_foo_process_a_register_1.topic.data_frequency = 10;

  sub_foo_process_a_register_2 = UpdateTopicSample(sub_foo_process_a_register_1);

  // Subscriber 2
  sub_foo_process_b_unregister.cmd_type = eCAL::bct_unreg_subscriber;
  sub_foo_process_b_unregister.identifier.host_name = "host0";
  sub_foo_process_b_unregister.identifier.process_id = 1000;
  sub_foo_process_b_unregister.identifier.entity_id = getUniqueId();
  sub_foo_process_b_unregister.topic.shm_transport_domain = "host0";
  sub_foo_process_b_unregister.topic.process_name = "process_b";
  sub_foo_process_b_unregister.topic.topic_name = "foo";
  sub_foo_process_b_register_1.topic.unit_name = "abc";

  sub_foo_process_b_register_1 = sub_foo_process_b_unregister;
  sub_foo_process_b_register_1.cmd_type = eCAL::bct_reg_subscriber;
  sub_foo_process_b_register_1.topic.registration_clock = 1;
  sub_foo_process_b_register_1.topic.direction = "subscriber";
  sub_foo_process_b_register_1.topic.datatype_information = { "a", "b", "c" };
  sub_foo_process_b_register_1.topic.topic_size = 100;
  sub_foo_process_b_register_1.topic.connections_local = 2;
  sub_foo_process_b_register_1.topic.connections_external = 2;
  sub_foo_process_b_register_1.topic.message_drops = 0;
  sub_foo_process_b_register_1.topic.data_id = 0;
  sub_foo_process_b_register_1.topic.data_clock = 1;
  sub_foo_process_b_register_1.topic.data_frequency = 10;

  sub_foo_process_b_register_2 = UpdateTopicSample(sub_foo_process_b_register_1);
}

class TestingClock {
public:
  // Define the required types for TrivialClock
  using duration = std::chrono::milliseconds;
  using rep = duration::rep;
  using period = duration::period;
  using time_point = std::chrono::time_point<TestingClock>;
  static const bool is_steady = false;

  // Function to get the current time
  static time_point now() noexcept {
    return time_point(current_time);
  }

  // Function to manually set the current time
  static void set_time(const time_point& tp) {
    current_time = tp.time_since_epoch();
  }

  // Function to manually increment the current time by a given duration
  static void increment_time(const duration& d) {
    current_time += d;
  }

private:
  static duration current_time;
};

// Initialize the static member
TestingClock::duration TestingClock::current_time{ 0 };

// Create a test fixture class
class core_cpp_registration : public ::testing::Test {
protected:
  // Override the SetUp method to initialize the global variable
  void SetUp() override {
    InitializeAllSamples();
    TestingClock::set_time(std::chrono::time_point<TestingClock>(std::chrono::milliseconds(0)));
  }

  // You can also override the TearDown method if needed
  void TearDown() override {
    // Clean up if necessary
  }
};

TEST_F(core_cpp_registration, IsUnregistrationSamples)
{
  EXPECT_EQ(eCAL::Registration::IsUnregistrationSample(pub_foo_process_a_unregister), true);
  EXPECT_EQ(eCAL::Registration::IsUnregistrationSample(pub_foo_process_a_register_1), false);
  EXPECT_EQ(eCAL::Registration::IsUnregistrationSample(pub_foo_process_a_register_2), false);
  EXPECT_EQ(eCAL::Registration::IsUnregistrationSample(sub_foo_process_a_unregister), true);
  EXPECT_EQ(eCAL::Registration::IsUnregistrationSample(sub_foo_process_a_register_1), false);
  EXPECT_EQ(eCAL::Registration::IsUnregistrationSample(sub_foo_process_a_register_2), false);
  EXPECT_EQ(eCAL::Registration::IsUnregistrationSample(sub_foo_process_b_unregister), true);
  EXPECT_EQ(eCAL::Registration::IsUnregistrationSample(sub_foo_process_b_register_1), false);
  EXPECT_EQ(eCAL::Registration::IsUnregistrationSample(sub_foo_process_b_register_2), false);
}


TEST_F(core_cpp_registration, CreateUnregistrationSamples)
{
  EXPECT_EQ(eCAL::Registration::CreateUnregisterSample(pub_foo_process_a_register_1), pub_foo_process_a_unregister);
  EXPECT_EQ(eCAL::Registration::CreateUnregisterSample(pub_foo_process_a_register_2), pub_foo_process_a_unregister);
  EXPECT_EQ(eCAL::Registration::CreateUnregisterSample(sub_foo_process_a_register_1), sub_foo_process_a_unregister);
  EXPECT_EQ(eCAL::Registration::CreateUnregisterSample(sub_foo_process_a_register_2), sub_foo_process_a_unregister);
  EXPECT_EQ(eCAL::Registration::CreateUnregisterSample(sub_foo_process_b_register_1), sub_foo_process_b_unregister);
  EXPECT_EQ(eCAL::Registration::CreateUnregisterSample(sub_foo_process_b_register_2), sub_foo_process_b_unregister);
}

// we apply samples and then unregistration samples
// we need to veryfy no callback is called
TEST_F(core_cpp_registration, TimeOutProviderApplyUnregistration)
{
  int callbacks_called = 0;
  eCAL::Registration::CTimeoutProvider<TestingClock> timout_provider(std::chrono::seconds(5), [&callbacks_called](const eCAL::Registration::Sample&) {callbacks_called++; return true; });

  timout_provider.ApplySample(pub_foo_process_a_register_1);
  TestingClock::increment_time(std::chrono::seconds(1));
  timout_provider.CheckForTimeouts();
  EXPECT_EQ(callbacks_called, 0);
  
  TestingClock::increment_time(std::chrono::seconds(1));
  timout_provider.ApplySample(pub_foo_process_a_register_2);
  TestingClock::increment_time(std::chrono::seconds(1));
  timout_provider.CheckForTimeouts();
  EXPECT_EQ(callbacks_called, 0);

  TestingClock::increment_time(std::chrono::seconds(1));
  timout_provider.ApplySample(pub_foo_process_a_unregister);
  TestingClock::increment_time(std::chrono::seconds(1));
  timout_provider.CheckForTimeouts();
  EXPECT_EQ(callbacks_called, 0);
}

// we apply samples and then unregistration samples
// we need to veryfy no callback is called
TEST_F(core_cpp_registration, TimeOutProviderApplyTimeout)
{
  int callbacks_called = 0;
  eCAL::Registration::Sample sample_from_callback;
  eCAL::Registration::CTimeoutProvider<TestingClock> timout_provider(std::chrono::seconds(5), 
    [&sample_from_callback, &callbacks_called](const eCAL::Registration::Sample& s)
      {
        sample_from_callback = s; 
        ++callbacks_called;  
        return true; 
    });

  timout_provider.ApplySample(pub_foo_process_a_register_1);
  TestingClock::increment_time(std::chrono::seconds(6));
  timout_provider.CheckForTimeouts();
  EXPECT_EQ(sample_from_callback, pub_foo_process_a_unregister);
  EXPECT_EQ(callbacks_called, 1);

  // reset sample
  sample_from_callback = eCAL::Registration::Sample{};
  callbacks_called = 0;
  TestingClock::increment_time(std::chrono::seconds(6));
  timout_provider.ApplySample(pub_foo_process_a_register_2);
  TestingClock::increment_time(std::chrono::seconds(6));
  timout_provider.CheckForTimeouts();
  EXPECT_EQ(sample_from_callback, pub_foo_process_a_unregister);
  EXPECT_EQ(callbacks_called, 1);
}
