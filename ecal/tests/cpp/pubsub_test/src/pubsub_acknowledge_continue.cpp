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
#include <ecal/ecal.h>
#include <ecal/pubsub/publisher.h>
#include <ecal/pubsub/subscriber.h>

#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include <gtest/gtest.h>

namespace
{
  void createSubscriber(std::unique_ptr<eCAL::CSubscriber>& sub_ptr_)
  {
    eCAL::SDataTypeInformation topic_info_sub;
    topic_info_sub.descriptor = "Next simulation framework event type";
    topic_info_sub.name = "EventType";
    topic_info_sub.encoding = "";
    
    eCAL::SubEventCallbackT event_callback = [](const eCAL::STopicId &,
                                                    const eCAL::SSubEventCallbackData & data_) {
      std::cout << "Event called. Type: " << to_string(data_.event_type) << "\n";
    };

    sub_ptr_ = std::make_unique<eCAL::CSubscriber>("test", topic_info_sub, event_callback);

    eCAL::ReceiveCallbackT callb = [](const eCAL::STopicId &, const eCAL::SDataTypeInformation &,const eCAL::SReceiveCallbackData & data_) {
      std::cout << "Receive CB called. Clock: " << data_.send_clock << "\n";
    };

    sub_ptr_->SetReceiveCallback(callb);
  }

  void subscriberThread()
  {
    std::unique_ptr<eCAL::CSubscriber> sub_ptr;

    createSubscriber(sub_ptr);

    while(eCAL::Ok())
    {
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  }

  
}

TEST(core_cpp_pubsub, TimeoutAcknowledgmentContinuation)
{
  auto config = eCAL::Configuration();
  config.registration.registration_timeout = 6000;

  // auto config = eCAL::Init::Configuration();
  
  // initialize eCAL API
  EXPECT_EQ(true, eCAL::Initialize(config, "TimeoutAcknowledgmentContinuation", eCAL::Init::All));


  eCAL::SDataTypeInformation topic_info_pub;
  topic_info_pub.name = "EventType";
  topic_info_pub.descriptor = "Next simulation framework event type";
  topic_info_pub.encoding = "";

  // create publisher config
  eCAL::Publisher::Configuration pub_config;
  pub_config.layer.shm.acknowledge_timeout_ms = 2000;

  // create publisher
  eCAL::CPublisher pub("test", topic_info_pub, pub_config);
  
  // std::thread sub_thread(subscriberThread);

  unsigned int send_counter = 0;

  while (++send_counter < 100)
  {
    auto start = std::chrono::high_resolution_clock::now();
    pub.Send("Hello", 5);
    auto end = std::chrono::high_resolution_clock::now();

    std::cout << "Send took: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "us\n";

    //if (send_counter == 10)
      //sub_ptr = nullptr;

    //if (send_counter == 20)
      //createSubscriber(sub_ptr);

    std::this_thread::sleep_for(std::chrono::seconds(1));
  }


  // finalize eCAL API
  // without destroying any pub / sub
  EXPECT_EQ(true, eCAL::Finalize());
}

TEST(core_cpp_pubsub, sub)
{
  auto config = eCAL::Configuration();
  config.registration.registration_timeout = 6000;

  // auto config = eCAL::Init::Configuration();
  
  // initialize eCAL API
  EXPECT_EQ(true, eCAL::Initialize(config, "TimeoutAcknowledgmentContinuation-sub", eCAL::Init::All));
  
  std::unique_ptr<eCAL::CSubscriber> sub_ptr;
  createSubscriber(sub_ptr);

  unsigned int counter = 0;
  while(eCAL::Ok() && counter < 12)
  {
    if (++counter % 2 == 0)
    {
      // sub_ptr.reset();
    }
    else
    {
      
    };

    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  eCAL::Finalize();
}