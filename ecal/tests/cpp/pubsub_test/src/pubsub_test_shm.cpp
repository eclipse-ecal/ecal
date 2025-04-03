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
    g_callback_received_bytes += data_.buffer_size;
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
    last_received_msg = std::string{ (const char*)data_.buffer, (size_t)data_.buffer_size};
    last_received_timestamp = data_.send_timestamp;
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

TEST(core_cpp_pubsub, SubscriberFastReconnectionSHM) {
  /* Test setup :
   * publisher runs permanently in a thread
   * subscriber A start reading topic
   * subscriber A gets out of scope (destruction)
   * Small delay less than the registration refresh
   * subscriber B start reading topic again
   * subscriber B gets out of scope (destruction)
   * Test ensures that subscriber is reconnecting and all sync mechanism are
   * working properly again. Previously the test suite was not catching a case
   * where a delay between the destruction of the topic A subscriber and its
   * recreation would create a subscriber that was not receiving messages.
   */

  constexpr auto REGISTRATION_REFRESH_MS = 100;
  constexpr auto REGISTRATION_TIMEOUT_MS = 10 * REGISTRATION_REFRESH_MS;

  constexpr auto MESSAGE_SEND_DELAY =
      std::chrono::milliseconds(REGISTRATION_REFRESH_MS);

  // The delay between the destruction of the subscriber and its recreation
  // must be between the registration refresh and the registration timeout
  constexpr auto RESUBSCRIBE_DELAY =
      std::chrono::milliseconds(3 * REGISTRATION_REFRESH_MS);

  // Ensure we wait at least one timeout period
  constexpr auto RECEIVE_TIMEOUT =
      std::chrono::milliseconds(2 * REGISTRATION_TIMEOUT_MS);

  constexpr auto TOPIC = "shm_reconnect_test";

  // Prepare config
  auto config = eCAL::Init::Configuration();
  config.publisher.layer.shm.enable = true;
  config.publisher.layer.tcp.enable = false;
  config.publisher.layer.udp.enable = false;
  config.subscriber.layer.shm.enable = true;
  config.subscriber.layer.tcp.enable = false;
  config.subscriber.layer.udp.enable = false;
  config.registration.registration_refresh = REGISTRATION_REFRESH_MS;
  config.registration.registration_timeout = REGISTRATION_TIMEOUT_MS;

  eCAL::Initialize(config, "SubscriberReconnectionSHM");

  std::atomic<bool> stop_publishing(false);
  const auto send_messages = [&](eCAL::CPublisher &pub) {
    while (!stop_publishing) {
      pub.Send("Hello World");
      std::this_thread::sleep_for(MESSAGE_SEND_DELAY);
    }
    std::cerr << "Stopped publishing\n";
  };

  eCAL::CPublisher publisher(TOPIC);
  std::thread message_publishing_thread(send_messages, std::ref(publisher));

  std::condition_variable cv;
  std::mutex cv_m;
  bool data_received(false);
  std::unique_lock<std::mutex> cv_lk(cv_m);

  const auto create_subscriber = [&]() -> eCAL::CSubscriber {
    eCAL::CSubscriber subscriber(TOPIC);

    const auto receive_lambda =
        [&](const eCAL::STopicId & /*topic_id_*/,
            const eCAL::SDataTypeInformation & /*data_type_info_*/,
            const eCAL::SReceiveCallbackData & /*data_*/) {
          {
            std::lock_guard<std::mutex> lk(cv_m);
            data_received = true;
          }
          // Mutex should be unlocked before notifying the condition variable
          cv.notify_all();
        };
    subscriber.SetReceiveCallback(receive_lambda);
    return subscriber;
  };

  const auto receive_one_message = [&]() {
    // We should receive something within the timeout period
    cv.wait_for(cv_lk, RECEIVE_TIMEOUT,
                [&data_received]() { return data_received; });

    EXPECT_TRUE(data_received) << "No messages received within the timeout";
  };

  // 1 - Subscribe to topic and receive a message
  {
    const auto subscriber = create_subscriber();
    receive_one_message();
    std::cerr << "Closing first subscriber scope\n";
  }

  // 2 - Small delay to unlink the SHM file but not timeout the observer
  std::this_thread::sleep_for(RESUBSCRIBE_DELAY);
  data_received = false; // Reset for next scope

  // 3 - Subscribe to topic again and try to receive a message
  {
    const auto subscriber = create_subscriber();
    receive_one_message();
    std::cerr << "Closing second subscriber scope\n";
  }

  stop_publishing = true;
  message_publishing_thread.join();

  eCAL::Finalize();
}
