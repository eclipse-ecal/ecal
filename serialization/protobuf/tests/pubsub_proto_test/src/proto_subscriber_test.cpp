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

// std headers
#include <atomic>
#include <chrono>
#include <cstddef>
#include <functional>
#include <thread>
// used libraries
#include <gtest/gtest.h>
// own project
#include <ecal/ecal.h>
#include <ecal/msg/protobuf/publisher.h>
#include <ecal/msg/protobuf/subscriber.h>

#include <person.pb.h>
#include <utility>

class ProtoSubscriberTest : public ::testing::Test {
public:
  ProtoSubscriberTest()
    : received_callbacks(0)
  {
    // Initialize eCAL
    eCAL::Initialize();
  }

  ~ProtoSubscriberTest() override {
    // Finalize eCAL
    eCAL::Finalize();
  }

  bool SendPerson(eCAL::protobuf::CPublisher<pb::People::Person>& pub)
  {
    p.set_id(1);
    p.set_name("Max");
    return pub.Send(p);
  }

  size_t GetPersonSize()
  {
#if GOOGLE_PROTOBUF_VERSION >= 3001000
    return static_cast<size_t>(p.ByteSizeLong());
#else
    return static_cast<size_t>(p.ByteSize());
#endif
  }

  void OnPerson()
  {
    received_callbacks++;
  }

  std::atomic<int> received_callbacks;

private:
  pb::People::Person p;
};
using core_cpp_pubsub_proto_sub = ProtoSubscriberTest;

TEST_F(core_cpp_pubsub_proto_sub, ProtoSubscriberTest_SendReceive)
{
  // Assert that the Subscriber can be move constructed.
  eCAL::protobuf::CSubscriber<pb::People::Person> person_rec("ProtoSubscriberTest");
  auto person_callback = std::bind(&ProtoSubscriberTest::OnPerson, this);
  person_rec.SetReceiveCallback(person_callback);

  eCAL::protobuf::CPublisher<pb::People::Person> person_pub("ProtoSubscriberTest");

  std::this_thread::sleep_for(std::chrono::milliseconds(2000));

  ASSERT_TRUE(SendPerson(person_pub));
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  // assert that the OnPerson callback has been called once.
  ASSERT_EQ(1, received_callbacks);
}

TEST_F(core_cpp_pubsub_proto_sub, ProtoSubscriberTest_MoveConstruction)
{

  // Assert that the Subscriber can be move constructed.
  std::vector<eCAL::protobuf::CSubscriber<pb::People::Person>> subscribers;
  subscribers.reserve(1000);

  subscribers.emplace_back("ProtoSubscriberTest");
  auto& person_rec = subscribers[0];
  auto person_callback = std::bind(&ProtoSubscriberTest::OnPerson, this);
  person_rec.SetReceiveCallback(person_callback);

  std::atomic<bool> stop_publishing(false);
  int numbers_of_sends = 0;
  // Assert that the move constructed object can receive something
  eCAL::Publisher::Configuration pub_config;
  pub_config.layer.shm.acknowledge_timeout_ms = 500;
  eCAL::protobuf::CPublisher<pb::People::Person> person_pub("ProtoSubscriberTest", pub_config);

  std::this_thread::sleep_for(std::chrono::milliseconds(2000));

  // Start publishing (With acknowledge?)
  std::thread person_pub_thread([this, &person_pub, &stop_publishing, &numbers_of_sends]() {
    while (!stop_publishing)
    {
      SendPerson(person_pub);
      ++numbers_of_sends;
    }
    });

  for (int i = 0; i < 999; ++i)
  {
    subscribers.emplace_back(std::move(subscribers[i]));
    std::this_thread::yield();
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(1000));

  stop_publishing = true;
  person_pub_thread.join();

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  ASSERT_EQ(numbers_of_sends, received_callbacks.load());
}
