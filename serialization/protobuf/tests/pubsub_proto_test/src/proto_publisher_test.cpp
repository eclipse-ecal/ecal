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

// std headers
// used libraries
#include <gtest/gtest.h>
// own project
#include <ecal/ecal.h>
#include <ecal/msg/protobuf/publisher.h>

#include <person.pb.h>
#include <utility>

class ProtoPublisherTest : public ::testing::Test {
public:

  ProtoPublisherTest() {
    eCAL::Initialize();
  }

  ~ProtoPublisherTest() override {
    eCAL::Finalize();
  }

};
using core_cpp_pubsub_proto_pub = ProtoPublisherTest;

TEST_F(core_cpp_pubsub_proto_pub, ProtoPublisherTest_MoveAssignment)
{
  eCAL::protobuf::CPublisher<pb::People::Person> person_pub("ProtoPublisherTest");
  auto topic_id = person_pub.GetTopicId();

  eCAL::protobuf::CPublisher<pb::People::Person>person_moved("ProtoPublisherTest");
  person_moved = std::move(person_pub);
  // New Publisher must have the same topic id
  ASSERT_EQ(person_moved.GetTopicId(), topic_id);
  // Old Publisher must have no topic id
  ASSERT_EQ(person_pub.GetTopicId(), eCAL::STopicId());
}

TEST_F(core_cpp_pubsub_proto_pub, ProtoPublisherTest_MoveConstruction)
{
  eCAL::protobuf::CPublisher<pb::People::Person> person_pub("ProtoPublisherTest");
  auto topic_id = person_pub.GetTopicId();

  eCAL::protobuf::CPublisher<pb::People::Person>person_moved{ std::move(person_pub) };
  // New Publisher must have the same topic id
  ASSERT_EQ(person_moved.GetTopicId(), topic_id);
  // Old Publisher must have no topic id
  ASSERT_EQ(person_pub.GetTopicId(), eCAL::STopicId());
}
