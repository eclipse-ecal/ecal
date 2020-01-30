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
#include <chrono>
#include <thread>
// used libraries
#include <gtest/gtest.h>
// own project
#include <ecal/ecal.h>
#include <ecal/msg/protobuf/publisher.h>

#include <person.pb.h>


// subscriber callback function


class ProtoPublisherTest : public ::testing::Test {
public:

  ProtoPublisherTest() {
    eCAL::Initialize();
  }

  virtual ~ProtoPublisherTest() {
    eCAL::Finalize();
  }

};

TEST_F(ProtoPublisherTest, MoveAssignment)
{
  eCAL::protobuf::CPublisher<pb::People::Person> person_pub("ProtoPublisherTest");

  ASSERT_TRUE(person_pub.IsCreated());

  eCAL::protobuf::CPublisher<pb::People::Person>person_moved;

  ASSERT_FALSE(person_moved.IsCreated());

  person_moved = std::move(person_pub);

  // New Subscriber must be initialized
  ASSERT_TRUE(person_moved.IsCreated());
  // Old subscriber is not initialized
  ASSERT_FALSE(person_pub.IsCreated());
}

TEST_F(ProtoPublisherTest, MoveConstruction)
{
  eCAL::protobuf::CPublisher<pb::People::Person> person_pub("ProtoPublisherTest");

  ASSERT_TRUE(person_pub.IsCreated());

  eCAL::protobuf::CPublisher<pb::People::Person>person_moved{ std::move(person_pub) };

  ASSERT_TRUE(person_moved.IsCreated());
  ASSERT_FALSE(person_pub.IsCreated());
}
