/* ========================= eCAL LICENSE =================================
 *
 * Copyright 2026 AUMOVIO and subsidiaries. All rights reserved.
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
#include <condition_variable>
#include <functional>
#include <mutex>
#include <string>
#include <thread>

#include <gtest/gtest.h>
#include <vector>

/*
* eCAL may send data via different transport layers.
* When this happens, we need to make sure that a subscriber which has multiple layers activated
* does not receive duplicate messages.
*/
enum {
  CMN_REGISTRATION_REFRESH_MS = 1000,
  DATA_FLOW_TIME_MS = 50,
};

namespace
{
  class ReceiveCounter
  {
  public:
    void OnData()
    {
      received_count_.fetch_add(1, std::memory_order_relaxed);
    }

    void Reset()
    {
      received_count_.store(0, std::memory_order_relaxed);
    }

    std::size_t Count() const { return received_count_.load(std::memory_order_relaxed); }
    
  private:
    std::atomic<std::size_t> received_count_{ 0 };
  };
}

TEST(core_cpp_pubsub_multilayer, MultiLayerNoDuplicateMessages)
{
  // default send string
  const std::string send_s;

  // initialize eCAL API
  eCAL::Initialize("pubsub_test");

  // create publisher config
  eCAL::Publisher::Configuration pub_config;
  // set transport layer
  pub_config.layer.shm.enable = true;
  pub_config.layer.udp.enable = true;
  pub_config.layer.tcp.enable = true;

  eCAL::Subscriber::Configuration sub_shm_config;
  sub_shm_config.layer.shm.enable = true;
  sub_shm_config.layer.udp.enable = false;
  sub_shm_config.layer.tcp.enable = false;

  eCAL::Subscriber::Configuration sub_udp_config;
  sub_udp_config.layer.shm.enable = false;
  sub_udp_config.layer.udp.enable = true;
  sub_udp_config.layer.tcp.enable = false;

  eCAL::Subscriber::Configuration sub_tcp_config;
  sub_tcp_config.layer.shm.enable = false;
  sub_tcp_config.layer.udp.enable = false;
  sub_tcp_config.layer.tcp.enable = true;

  eCAL::Subscriber::Configuration sub_all_config;
  sub_all_config.layer.shm.enable = true;
  sub_all_config.layer.udp.enable = true;
  sub_all_config.layer.tcp.enable = true;


  // create publisher for topic "A" (no zero copy)
  eCAL::CPublisher pub("A", eCAL::SDataTypeInformation(), pub_config);
  eCAL::CSubscriber sub_shm("A", eCAL::SDataTypeInformation(), sub_shm_config);
  eCAL::CSubscriber sub_udp("A", eCAL::SDataTypeInformation(), sub_udp_config);
  eCAL::CSubscriber sub_tcp("A", eCAL::SDataTypeInformation(), sub_tcp_config);
  eCAL::CSubscriber sub_all("A", eCAL::SDataTypeInformation(), sub_all_config);

  ReceiveCounter counter_shm;
  sub_shm.SetReceiveCallback([&counter_shm](auto&&...) {counter_shm.OnData();});
  ReceiveCounter counter_udp;
  sub_udp.SetReceiveCallback([&counter_udp](auto&&...) {counter_udp.OnData(); });
  ReceiveCounter counter_tcp;
  sub_tcp.SetReceiveCallback([&counter_tcp](auto&&...) {counter_tcp.OnData(); });
  ReceiveCounter counter_all;
  sub_all.SetReceiveCallback([&counter_all](auto&&...) {counter_all.OnData(); });

  // let's match them
  eCAL::Process::SleepMS(3 * CMN_REGISTRATION_REFRESH_MS);

  const int number_messages_sent = 10;
  for (int i = 0; i < number_messages_sent; ++i)
  {
    EXPECT_TRUE(pub.Send(send_s));
    eCAL::Process::SleepMS(DATA_FLOW_TIME_MS);
  }
  
  EXPECT_EQ(counter_shm.Count(), number_messages_sent);
  EXPECT_EQ(counter_udp.Count(), number_messages_sent);
  EXPECT_EQ(counter_tcp.Count(), number_messages_sent);
  EXPECT_EQ(counter_all.Count(), number_messages_sent);
  
  // finalize eCAL API
  eCAL::Finalize();
}

