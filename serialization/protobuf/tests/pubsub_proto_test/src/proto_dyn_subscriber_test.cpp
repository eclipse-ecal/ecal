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
#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <thread>

// used libraries
#include <gtest/gtest.h>

// own project
#include <ecal/ecal.h>
#include <ecal/msg/protobuf/publisher.h>
#include <ecal/msg/protobuf/dynamic_subscriber.h>

#include <person.pb.h>

// subscriber callback function

class ProtoDynSubscriberTest : public ::testing::Test {
public:
  ProtoDynSubscriberTest()
    : received_callbacks(0)
  {
    // Initialize eCAL
    eCAL::Initialize();
  }

  ~ProtoDynSubscriberTest() override {
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

  void OnPerson(const std::shared_ptr<google::protobuf::Message>& /*message*/, long long /*time*/)
  {
    received_callbacks++;
  }

  std::atomic<int> received_callbacks;
};
using core_cpp_pubsub_proto_dyn = ProtoDynSubscriberTest;

int extract_id(const google::protobuf::Message& msg_)
{
  int count = msg_.GetDescriptor()->field_count();
  const google::protobuf::Reflection* ref_ptr = msg_.GetReflection();

  if (ref_ptr)
  {
    for (int i = 0; i < count; ++i)
    {
      auto field = msg_.GetDescriptor()->field(i);

      const google::protobuf::FieldDescriptor::CppType fdt = field->cpp_type();
      if (fdt == google::protobuf::FieldDescriptor::CPPTYPE_INT32 && !field->is_repeated())
      {
        if (field->name() == "id")
        {
          return ref_ptr->GetInt32(msg_, field);
        }
      }
    }
  }
  return 0;
}

TEST_F(core_cpp_pubsub_proto_dyn, ProtoDynSubscriberTest_SendReceiveCB)
{
  // Assert that the Subscriber can be move constructed.
  eCAL::protobuf::CDynamicSubscriber person_dyn_rec("ProtoSubscriberTest");
  auto person_callback = std::bind(&ProtoDynSubscriberTest::OnPerson, this, std::placeholders::_2, std::placeholders::_3);
  person_dyn_rec.SetReceiveCallback(person_callback);

  eCAL::protobuf::CPublisher<pb::People::Person> person_pub("ProtoSubscriberTest");

  std::this_thread::sleep_for(std::chrono::milliseconds(2000));

  SendPerson(person_pub);
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  // assert that the OnPerson callback has been called once.
  ASSERT_EQ(1, received_callbacks);
}
