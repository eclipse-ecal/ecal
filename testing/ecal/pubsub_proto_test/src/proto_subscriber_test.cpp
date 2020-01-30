/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2019 Continental Corporation
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
#include <thread>
// used libraries
#include <gtest/gtest.h>
// own project
#include <ecal/ecal.h>
#include <ecal/msg/protobuf/publisher.h>
#include <ecal/msg/protobuf/subscriber.h>

#include <person.pb.h>

// subscriber callback function

#define REGISTRATION_REFRESH_CYCLE 1000

class ProtoSubscriberTest : public ::testing::Test {
public:
  ProtoSubscriberTest() 
  : received_callbacks(0)
  {
    // Initialize eCAL
    eCAL::Initialize();
    // publish / subscribe match in the same process
    eCAL::Util::EnableLoopback(true);
  }

  virtual ~ProtoSubscriberTest() {
    // Finalize eCAL
    eCAL::Finalize();
  }

  void SendPerson(eCAL::protobuf::CPublisher<pb::People::Person>& pub)
  {
    pb::People::Person p;
    p.set_id(1);
    p.set_name("Max");
    pub.Send(p);
  }
  
  void OnPerson(const char*, const pb::People::Person&, long long, long long)
  {
    received_callbacks++;
  }

  std::atomic<int> received_callbacks;
};

TEST_F(ProtoSubscriberTest, SendReceive)
{
  // Assert that the Subscriber can be move constructed.
  eCAL::protobuf::CSubscriber<pb::People::Person> person_rec("ProtoSubscriberTest");
  auto person_callback = std::bind(&ProtoSubscriberTest::OnPerson, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
  person_rec.AddReceiveCallback(person_callback);
  ASSERT_TRUE(person_rec.IsCreated());

  eCAL::protobuf::CPublisher<pb::People::Person> person_pub("ProtoSubscriberTest");

  std::this_thread::sleep_for(std::chrono::milliseconds(2000));

  SendPerson(person_pub);
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  // assert that the OnPerson callback has been called once.
  ASSERT_EQ(1, received_callbacks);

}


TEST_F(ProtoSubscriberTest, MoveAssignment)
{
  eCAL::protobuf::CSubscriber<pb::People::Person> person_rec("ProtoSubscriberTest");
  auto person_callback = std::bind(&ProtoSubscriberTest::OnPerson, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
  person_rec.AddReceiveCallback(person_callback);

  ASSERT_TRUE(person_rec.IsCreated());

  eCAL::protobuf::CSubscriber<pb::People::Person>person_moved;

  ASSERT_FALSE(person_moved.IsCreated());

  person_moved = std::move(person_rec);

  // New Subscriber must be initialized
  ASSERT_TRUE(person_moved.IsCreated());
  // Old subscriber is not initialized
  ASSERT_FALSE(person_rec.IsCreated());
  ASSERT_EQ("ProtoSubscriberTest", person_moved.GetTopicName());

  // Assert that the move constructed object can receive something
  eCAL::protobuf::CPublisher<pb::People::Person> person_pub("ProtoSubscriberTest");

  std::this_thread::sleep_for(std::chrono::milliseconds(2000));

  SendPerson(person_pub);
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  // assert that the OnPerson callback has been called once.
  ASSERT_EQ(1, received_callbacks);
}

TEST_F(ProtoSubscriberTest, MoveConstruction)
{
  // Assert that the Subscriber can be move constructed.
  eCAL::protobuf::CSubscriber<pb::People::Person> person_rec("ProtoSubscriberTest");
  auto person_callback = std::bind(&ProtoSubscriberTest::OnPerson, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
  person_rec.AddReceiveCallback(person_callback);

  ASSERT_TRUE(person_rec.IsCreated());

  eCAL::protobuf::CSubscriber<pb::People::Person>person_moved{ std::move(person_rec) };

  ASSERT_TRUE(person_moved.IsCreated());
  ASSERT_FALSE(person_rec.IsCreated());
  ASSERT_EQ("ProtoSubscriberTest", person_moved.GetTopicName());

  // Assert that the move constructed object can receive something
  eCAL::protobuf::CPublisher<pb::People::Person> person_pub("ProtoSubscriberTest");

  std::this_thread::sleep_for(std::chrono::milliseconds(2000));

  SendPerson(person_pub);
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  // assert that the OnPerson callback has been called once.
  ASSERT_EQ(1, received_callbacks);

}
