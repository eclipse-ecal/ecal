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
#include <ecal/pubsub/publisher.h>
#include <ecal/pubsub/subscriber.h>

#include <atomic>
#include <functional>
#include <string>
#include <thread>

#include <gtest/gtest.h>
#include <vector>

enum {
  CMN_REGISTRATION_REFRESH_MS = 1000,
  DATA_FLOW_TIME_MS = 50,
};

namespace
{
  // subscriber callback function
  std::atomic<size_t> g_callback_received_bytes;
  std::atomic<size_t> g_callback_received_count;
  void OnReceive(const struct eCAL::SReceiveCallbackData& data_)
  {
    g_callback_received_bytes += data_.size;
    g_callback_received_count++;
  }
}

TEST(core_cpp_pubsub, ZeroPayloadMessageSHM)
{
  // default send string
  const std::string send_s;

  // initialize eCAL API
  eCAL::Initialize("pubsub_test");

  // create subscriber for topic "A"
  eCAL::CSubscriber sub("A");

  // create publisher config
  eCAL::Publisher::Configuration pub_config;
  // set transport layer
  pub_config.layer.shm.enable = true;
  pub_config.layer.udp.enable = false;
  pub_config.layer.tcp.enable = false;

  // create publisher for topic "A" (no zero copy)
  eCAL::CPublisher pub1("A", eCAL::SDataTypeInformation(), pub_config);

  // switch on zero copy
  pub_config.layer.shm.zero_copy_mode = true;
  eCAL::CPublisher pub2("A", eCAL::SDataTypeInformation(), pub_config);


  // add callback
  EXPECT_TRUE(sub.SetReceiveCallback(std::bind(OnReceive, std::placeholders::_3)));

  // let's match them
  eCAL::Process::SleepMS(2 * CMN_REGISTRATION_REFRESH_MS);

  g_callback_received_bytes = 0;
  g_callback_received_count = 0;

  // send without zero copy
  EXPECT_TRUE(pub1.Send(send_s));
  eCAL::Process::SleepMS(DATA_FLOW_TIME_MS);

  EXPECT_TRUE(pub1.Send(nullptr, 0));
  eCAL::Process::SleepMS(DATA_FLOW_TIME_MS);

  // send with zero copy
  EXPECT_TRUE(pub2.Send(send_s));
  eCAL::Process::SleepMS(DATA_FLOW_TIME_MS);

  EXPECT_TRUE(pub2.Send(nullptr, 0));
  eCAL::Process::SleepMS(DATA_FLOW_TIME_MS);

  // check callback receive
  EXPECT_EQ(send_s.size(), g_callback_received_bytes);
  EXPECT_EQ(4, g_callback_received_count);

  // finalize eCAL API
  eCAL::Finalize();
}

TEST(core_cpp_pubsub, MultipleSendsSHM)
{
  // default send string
  const std::vector<std::string> send_vector{ "this", "is", "a", "", "testtest" };
  std::string last_received_msg;
  long long   last_received_timestamp(0);

  // initialize eCAL API
  eCAL::Initialize("pubsub_test");

  // create subscriber for topic "A"
  eCAL::CSubscriber sub("A");

  // create publisher config
  eCAL::Publisher::Configuration pub_config;
  // set transport layer
  pub_config.layer.shm.enable = true;
  pub_config.layer.udp.enable = false;
  pub_config.layer.tcp.enable = false;

  // create publisher for topic "A"
  eCAL::CPublisher pub("A", {}, pub_config);

  // add callback
  auto save_data = [&last_received_msg, &last_received_timestamp](const eCAL::STopicId& /*topic_id_*/, const eCAL::SDataTypeInformation& /*data_type_info_*/, const eCAL::SReceiveCallbackData& data_)
  {
    last_received_msg = std::string{ (const char*)data_.buf, (size_t)data_.size};
    last_received_timestamp = data_.time;
  };
  EXPECT_TRUE(sub.SetReceiveCallback(save_data));

  // let's match them
  eCAL::Process::SleepMS(2 * CMN_REGISTRATION_REFRESH_MS);
  long long timestamp = 1;
  for (const auto& elem : send_vector)
  {
    pub.Send(elem, timestamp);
    eCAL::Process::SleepMS(DATA_FLOW_TIME_MS);
    EXPECT_EQ(last_received_msg, elem);
    EXPECT_EQ(last_received_timestamp, timestamp);
    ++timestamp;
  }

  // finalize eCAL API
  eCAL::Finalize();
}

TEST(core_cpp_pubsub, SubscriberReconnectionSHM) {
  /* Test setup :
   * publishers runs permanently in a thread
   * subscriber A start reading topic A
   * subscriber A gets out of scope (destruction)
   * subscriber B start reading topic B
   * subscriber B gets out of scope (destruction)
   * subscriber A starts again in a new scope
   * Test ensures that subscriber is reconnecting and all sync mechanism are
   * working properly again. Previously the test suite was not catching a case
   * where a delay between the destruction of the topic A subscriber and its
   * recreation would create a subscriber that was not receiving messages.
   */

  // Prepare config
  auto config = eCAL::Init::Configuration();
  config.publisher.layer.shm.enable = true;
  config.publisher.layer.tcp.enable = false;
  config.publisher.layer.udp.enable = false;
  config.subscriber.layer.shm.enable = true;
  config.subscriber.layer.tcp.enable = false;
  config.subscriber.layer.udp.enable = false;

  // initialize eCAL API
  eCAL::Initialize(config, "SubscriberReconnectionSHM");

  constexpr auto RECEIVE_TIMEOUT = std::chrono::seconds(5);
  constexpr auto TOPIC_A = "shm_reconnect_test_A";
  constexpr auto TOPIC_B = "shm_reconnect_test_B";

  // start publishing thread
  std::atomic<bool> stop_publishing(false);
  const auto publish_messages = [&stop_publishing](eCAL::CPublisher &pub) {
    while (!stop_publishing) {
      pub.Send("Hello World");
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    std::cout << "Stopped publishing" << std::endl;
  };

  eCAL::CPublisher pub_foo(TOPIC_A);
  std::thread pub_foo_t(publish_messages, std::ref(pub_foo));

  eCAL::CPublisher pub_bar(TOPIC_B);
  std::thread pub_bar_t(publish_messages, std::ref(pub_bar));

  std::condition_variable cv;
  std::mutex cv_m;
  bool data_received(false);

  const auto receive_lambda =
      [&cv_m, &cv,
       &data_received](const eCAL::Registration::STopicId & /*topic_id_*/,
                       const eCAL::SDataTypeInformation & /*data_type_info_*/,
                       const eCAL::SReceiveCallbackData & /*data_*/) {
        {
          std::cout << "Callback received message" << std::endl;
          std::lock_guard<std::mutex> lk(cv_m);
          data_received = true;
        }
        cv.notify_all();
      };
  std::unique_lock<std::mutex> cv_lk(cv_m);

  // 1 - Subscribe to topic A and receive a message
  {
    eCAL::CSubscriber sub_foo(TOPIC_A);
    sub_foo.SetReceiveCallback(receive_lambda);

    // We should receive something within the timeout period
    cv.wait_for(cv_lk, RECEIVE_TIMEOUT,
                [&data_received]() { return data_received; });

    EXPECT_TRUE(data_received);
    std::cout << "Closing first subscriber scope (A)" << std::endl;
  }

  data_received = false; // Reset for next scope

  // 2 - Subscribe to topic B and receive a message
  {
    eCAL::CSubscriber sub_foo(TOPIC_B);
    sub_foo.SetReceiveCallback(receive_lambda);

    // We should receive something within the timeout period
    cv.wait_for(cv_lk, RECEIVE_TIMEOUT,
                [&data_received]() { return data_received; });

    EXPECT_TRUE(data_received);
    std::cout << "Closing second subscriber scope (B)" << std::endl;
  }

  data_received = false; // Reset for next scope

  // 3 - Subscribe to topic A again and receive a message
  // TODO: Figure out why this now fails
  {
    eCAL::CSubscriber sub_foo(TOPIC_A);
    sub_foo.SetReceiveCallback(receive_lambda);

    // We should receive something within the timeout period
    cv.wait_for(cv_lk, RECEIVE_TIMEOUT,
                [&data_received]() { return data_received; });

    EXPECT_TRUE(data_received);
    std::cout << "Closing third subscriber scope (A again)" << std::endl;
  }

  // stop publishing and join thread
  stop_publishing = true;
  pub_foo_t.join();
  pub_bar_t.join();

  // finalize eCAL API
  // without destroying any pub / sub
  eCAL::Finalize();
}
