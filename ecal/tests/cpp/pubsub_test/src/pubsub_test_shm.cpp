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
#include <ecal/msg/string/publisher.h>
#include <ecal/msg/string/subscriber.h>

#include <atomic>
#include <thread>

#include <gtest/gtest.h>

enum {
  CMN_REGISTRATION_REFRESH_MS = 1000,
  DATA_FLOW_TIME_MS = 50,
};

namespace
{
  // subscriber callback function
  std::atomic<size_t> g_callback_received_bytes;
  std::atomic<size_t> g_callback_received_count;
  void OnReceive(const char* /*topic_name_*/, const struct eCAL::SReceiveCallbackData* data_)
  {
    g_callback_received_bytes += data_->size;
    g_callback_received_count++;
  }
}

TEST(core_cpp_pubsub, ZeroPayloadMessageSHM)
{
  // default send string
  const std::string send_s;

  // initialize eCAL API
  eCAL::Initialize(0, nullptr, "pubsub_test");

  // publish / subscribe match in the same process
  eCAL::Util::EnableLoopback(true);

  // create subscriber for topic "A"
  eCAL::CSubscriber sub("A");

  // create publisher config
  eCAL::Publisher::Configuration pub_config;
  // set transport layer
  pub_config.layer.shm.enable = true;
  pub_config.layer.udp.enable = false;
  pub_config.layer.tcp.enable = false;

  // create publisher for topic "A" (no zero copy)
  eCAL::CPublisher pub1("A", pub_config);

  // switch on zero copy
  pub_config.layer.shm.zero_copy_mode = true;
  eCAL::CPublisher pub2("A", pub_config);


  // add callback
  EXPECT_EQ(true, sub.AddReceiveCallback(std::bind(OnReceive, std::placeholders::_1, std::placeholders::_2)));

  // let's match them
  eCAL::Process::SleepMS(2 * CMN_REGISTRATION_REFRESH_MS);

  g_callback_received_bytes = 0;
  g_callback_received_count = 0;

  // send without zero copy
  EXPECT_EQ(send_s.size(), pub1.Send(send_s));
  eCAL::Process::SleepMS(DATA_FLOW_TIME_MS);

  EXPECT_EQ(send_s.size(), pub1.Send(nullptr, 0));
  eCAL::Process::SleepMS(DATA_FLOW_TIME_MS);

  // send with zero copy
  EXPECT_EQ(send_s.size(), pub2.Send(send_s));
  eCAL::Process::SleepMS(DATA_FLOW_TIME_MS);

  EXPECT_EQ(send_s.size(), pub2.Send(nullptr, 0));
  eCAL::Process::SleepMS(DATA_FLOW_TIME_MS);

  // check callback receive
  EXPECT_EQ(send_s.size(), g_callback_received_bytes);
  EXPECT_EQ(4, g_callback_received_count);

  // destroy subscriber
  sub.Destroy();

  // destroy publisher
  pub1.Destroy();
  pub2.Destroy();

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
  eCAL::Initialize(0, nullptr, "pubsub_test");

  // publish / subscribe match in the same process
  eCAL::Util::EnableLoopback(true);

  // create subscriber for topic "A"
  eCAL::string::CSubscriber<std::string> sub("A");

  // create publisher config
  eCAL::Publisher::Configuration pub_config;
  // set transport layer
  pub_config.layer.shm.enable = true;
  pub_config.layer.udp.enable = false;
  pub_config.layer.tcp.enable = false;

  // create publisher for topic "A"
  eCAL::string::CPublisher<std::string> pub("A", pub_config);

  // add callback
  auto save_data = [&last_received_msg, &last_received_timestamp](const char* /*topic_name_*/, const std::string& msg_, long long time_, long long /*clock_*/, long long /*id_*/)
    {
      last_received_msg = msg_;
      last_received_timestamp = time_;
    };
  EXPECT_TRUE(sub.AddReceiveCallback(save_data));

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

  // destroy subscriber
  sub.Destroy();

  // destroy publisher
  pub.Destroy();

  // finalize eCAL API
  eCAL::Finalize();
}
