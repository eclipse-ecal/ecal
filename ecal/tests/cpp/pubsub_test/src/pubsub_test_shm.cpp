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
#include <ecal/ecal_publisher.h>
#include <ecal/ecal_subscriber.h>

#include <atomic>
#include <functional>
#include <string>

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
  auto save_data = [&last_received_msg, &last_received_timestamp](const eCAL::Registration::STopicId& /*topic_id_*/, const eCAL::SDataTypeInformation& /*data_type_info_*/, const eCAL::SReceiveCallbackData& data_)
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
