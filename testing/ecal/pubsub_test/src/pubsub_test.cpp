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

#include <ecal/ecal.h>

#include <atomic>
#include <thread>

#include <gtest/gtest.h>

#define CMN_REGISTRATION_REFRESH   1000
#define CMN_MONITORING_TIMEOUT     5000
#define DATA_FLOW_TIME               50
#define PAYLOAD_SIZE               1024

// subscriber callback function
std::atomic<size_t> g_callback_received_bytes;
std::atomic<size_t> g_callback_received_count;
void OnReceive(const char* /*topic_name_*/, const struct eCAL::SReceiveCallbackData* data_)
{
  g_callback_received_bytes += data_->size;
  g_callback_received_count++;
}

static std::string CreatePayLoad(size_t payload_size_)
{
  std::string s = "Hello World ";
  while(s.size() < payload_size_)
  {
    s += s;
  }
  s.resize(payload_size_);
  return(s);
}

TEST(IO, InitializeFinalize)
{ 
  // Is eCAL API initialized ?
  EXPECT_EQ(0, eCAL::IsInitialized());

  // initialize eCAL API
  EXPECT_EQ(0, eCAL::Initialize(0, nullptr, "initialize_test"));

  // Is eCAL API initialized ?
  EXPECT_EQ(1, eCAL::IsInitialized());

  // initialize eCAL API again we expect return value 1 for yet initialized
  EXPECT_EQ(1, eCAL::Initialize(0, nullptr, "initialize_test"));

  // post initialize eCAL API monitoring API we expect return value 0 for success
  EXPECT_EQ(0, eCAL::Initialize(0, nullptr, "initialize_test", eCAL::Init::Monitoring));

  // post initialize eCAL API monitoring API again we expect return value 1 for yet initialized
  EXPECT_EQ(1, eCAL::Initialize(0, nullptr, "initialize_test", eCAL::Init::Monitoring));

  // finalize eCAL API 2 times for the two monitoring post calls, so we decrease the reference
  // counter and expect 0 for success
  EXPECT_EQ(0, eCAL::Finalize());
  EXPECT_EQ(0, eCAL::Finalize());

  // Is eCAL API initialized ? yes ..
  EXPECT_EQ(1, eCAL::IsInitialized());

  // finalize first time eCAL API we still expect 0 because 
  // reference counter still greater 0
  EXPECT_EQ(0, eCAL::Finalize());

  // finalize second time eCAL API we still expect 0 
  // but now reference counter is 0 and destruction should be succeeded
  EXPECT_EQ(0, eCAL::Finalize());

  // finalize eCAL API again we expect 1 because yet finalized
  EXPECT_EQ(1, eCAL::Finalize());
}

TEST(IO, CreateDestroy)
{ 
  // initialize eCAL API
  eCAL::Initialize(0, nullptr, "pubsub_test");

  // create publisher for topic "A"
  eCAL::CPublisher pub;

  // check state
  EXPECT_EQ(false, pub.IsCreated());

  // create
  EXPECT_EQ(true, pub.Create("A"));

  // check state
  EXPECT_EQ(true, pub.IsCreated());

  // create subscriber for topic "A"
  eCAL::CSubscriber sub;

  // check state
  EXPECT_EQ(false, sub.IsCreated());

  // create
  EXPECT_EQ(true, sub.Create("A"));

  // check state
  EXPECT_EQ(true, sub.IsCreated());

  // destroy publisher
  EXPECT_EQ(true, pub.Destroy());

  // destroy subscriber
  EXPECT_EQ(true, sub.Destroy());

  // finalize eCAL API
  eCAL::Finalize();
}

TEST(IO, TypeDescriptionStatic)
{
  // initialize eCAL API
  eCAL::Initialize(0, nullptr, "pubsub_test");

  // create publisher without type and description
  eCAL::CPublisher pub("A");

  eCAL::SDataTypeInformation tdatatype = pub.GetDataTypeInformation();
  EXPECT_EQ("", tdatatype.encoding);
  EXPECT_EQ("", tdatatype.name);
  EXPECT_EQ("", tdatatype.descriptor);

  // check type name
  eCAL::Util::GetTopicDataTypeInformation("A", tdatatype);
  EXPECT_EQ("", tdatatype.encoding);
  EXPECT_EQ("", tdatatype.name);
  EXPECT_EQ("", tdatatype.descriptor);

  // finalize eCAL API
  eCAL::Finalize();
}

TEST(IO, TypeDescriptionDynamic)
{
  // initialize eCAL API
  eCAL::Initialize(0, nullptr, "pubsub_test");

  {
    eCAL::SDataTypeInformation tdatatype{ "encoding_A", "type_A", "desc_A" };

    // create publisher with type and description
    eCAL::CPublisher pub("A", tdatatype);

    // check topic information
    eCAL::SDataTypeInformation retrieved_info = pub.GetDataTypeInformation();
    EXPECT_EQ(retrieved_info, tdatatype);

    // check topic information from eCAL Utils
    eCAL::SDataTypeInformation retrieved_util_info;
    eCAL::Util::GetTopicDataTypeInformation("A", retrieved_util_info);
    EXPECT_EQ(retrieved_util_info, tdatatype);

    // set topic info
    eCAL::SDataTypeInformation tinfo_new(tdatatype);
    tinfo_new.descriptor = "desc_A_new";
    pub.SetDataTypeInformation(tinfo_new);  // Already set descriptions are not overwritten in the database

    // check type name (should not be influenced by SetDescription)
    eCAL::SDataTypeInformation retrieved_util_info_new;
    eCAL::Util::GetTopicDataTypeInformation("A", retrieved_util_info_new);
    EXPECT_EQ(retrieved_util_info_new, tdatatype);

    // check description of publisher
    EXPECT_EQ(pub.GetDataTypeInformation(), tinfo_new);
  }

  // Test replace empty description
  {
    eCAL::SDataTypeInformation tinfo_no_desc{  "my_type", "", "" };
    eCAL::SDataTypeInformation tinfo_with_desc{ "my_type", "", "my_description" };
    eCAL::SDataTypeInformation tinfo_with_other_desc{ "my_type", "", "my_description_2" };

    auto pub2 = eCAL::CPublisher("B", tinfo_no_desc);

    {
      eCAL::SDataTypeInformation retrieved_util_info;
      eCAL::Util::GetTopicDataTypeInformation("B", retrieved_util_info);
      EXPECT_EQ(retrieved_util_info, tinfo_no_desc);
    }

    pub2.SetDataTypeInformation(tinfo_with_desc);

    {
      eCAL::SDataTypeInformation retrieved_util_info;
      eCAL::Util::GetTopicDataTypeInformation("B", retrieved_util_info);
      EXPECT_EQ(retrieved_util_info, tinfo_with_desc);
    }

    // A new publisher cannot change the description any more
    auto pub3 = eCAL::CPublisher("B", tinfo_with_other_desc);

    {
      eCAL::SDataTypeInformation retrieved_util_info;
      eCAL::Util::GetTopicDataTypeInformation("B", retrieved_util_info);
      EXPECT_EQ(retrieved_util_info, tinfo_with_desc);
    }
  }

  // Test Publisher replaces subscriber's description
  {
    eCAL::SDataTypeInformation tinfo_C1{ "type1", "", "desc" };
    eCAL::SDataTypeInformation tinfo_C2{ "type2", "", "" };
    eCAL::SDataTypeInformation tinfo_C3{ "type3", "", "desc3" };


    auto sub = eCAL::CSubscriber("C", tinfo_C1);

    {
      eCAL::SDataTypeInformation retrieved_util_info;
      eCAL::Util::GetTopicDataTypeInformation("C", retrieved_util_info);
      EXPECT_EQ(retrieved_util_info, tinfo_C1);
    }

    // A publisher without a description will NOT replace the subscribers data
    auto pub1 = eCAL::CPublisher("C", tinfo_C2);
    
    {
      eCAL::SDataTypeInformation retrieved_util_info;
      eCAL::Util::GetTopicDataTypeInformation("C", retrieved_util_info);
      EXPECT_EQ(retrieved_util_info, tinfo_C1);
    }

    // A publisher with a description will replace the subscribers data
    auto pub2 = eCAL::CPublisher("C", tinfo_C3);

    {
      eCAL::SDataTypeInformation retrieved_util_info;
      eCAL::Util::GetTopicDataTypeInformation("C", retrieved_util_info);
      EXPECT_EQ(retrieved_util_info, tinfo_C3);
    }
  }

  // finalize eCAL API
  eCAL::Finalize();
}

TEST(IO, SimpleMessage1)
{ 
  // default send / receive strings
  std::string send_s = CreatePayLoad(PAYLOAD_SIZE);
  std::string recv_s;

  // initialize eCAL API
  eCAL::Initialize(0, nullptr, "pubsub_test");

  // publish / subscribe match in the same process
  eCAL::Util::EnableLoopback(true);

  // create publisher for topic "A"
  eCAL::CPublisher pub("A");

  // create subscriber for topic "A"
  eCAL::CSubscriber sub("A");

  // let's match them
  eCAL::Process::SleepMS(2 * CMN_REGISTRATION_REFRESH);

  // send content
  EXPECT_EQ(send_s.size(), pub.Send(send_s));

  // receive content with DATA_FLOW_TIME ms timeout
  recv_s.clear();
  EXPECT_EQ(true, sub.ReceiveBuffer(recv_s, nullptr, DATA_FLOW_TIME));
  EXPECT_EQ(send_s.size(), recv_s.size());

  // receive content with DATA_FLOW_TIME ms timeout
  // should return because no new publishing
  recv_s.clear();
  EXPECT_EQ(false, sub.ReceiveBuffer(recv_s, nullptr, DATA_FLOW_TIME));
  EXPECT_EQ(0, recv_s.size());

  // destroy publisher
  pub.Destroy();

  // destroy subscriber
  sub.Destroy();

  // finalize eCAL API
  eCAL::Finalize();
}

TEST(IO, SimpleMessage2)
{ 
  // default send / receive strings
  std::string send_s = CreatePayLoad(PAYLOAD_SIZE);
  std::string recv_s;

  // initialize eCAL API
  eCAL::Initialize(0, nullptr, "pubsub_test");

  // publish / subscribe match in the same process
  eCAL::Util::EnableLoopback(true);

  // create subscriber for topic "A"
  eCAL::CSubscriber sub("A");

  // create publisher for topic "A"
  eCAL::CPublisher pub("A");

  // let's match them
  eCAL::Process::SleepMS(2 * CMN_REGISTRATION_REFRESH);

  // send content
  EXPECT_EQ(send_s.size(), pub.Send(send_s));

  // receive content with DATA_FLOW_TIME ms timeout
  recv_s.clear();
  EXPECT_EQ(true, sub.ReceiveBuffer(recv_s, nullptr, DATA_FLOW_TIME));
  EXPECT_EQ(send_s.size(), recv_s.size());

  // destroy publisher
  pub.Destroy();

  // destroy subscriber
  sub.Destroy();

  // finalize eCAL API
  eCAL::Finalize();
}

TEST(IO, SimpleMessageCB)
{ 
  // default send string
  std::string send_s = CreatePayLoad(PAYLOAD_SIZE);

  // initialize eCAL API
  eCAL::Initialize(0, nullptr, "pubsub_test");

  // publish / subscribe match in the same process
  eCAL::Util::EnableLoopback(true);

  // create subscriber for topic "A"
  eCAL::CSubscriber sub("A");

  // create publisher for topic "A"
  eCAL::CPublisher pub("A");

  // add callback
  EXPECT_EQ(true, sub.AddReceiveCallback(std::bind(OnReceive, std::placeholders::_1, std::placeholders::_2)));

  // let's match them
  eCAL::Process::SleepMS(2 * CMN_REGISTRATION_REFRESH);

  // send content
  g_callback_received_bytes = 0;
  EXPECT_EQ(send_s.size(), pub.Send(send_s));

  // let the data flow
  eCAL::Process::SleepMS(DATA_FLOW_TIME);

  // check callback receive
  EXPECT_EQ(send_s.size(), g_callback_received_bytes);

  // remove receive callback
  sub.RemReceiveCallback();

  // send content
  g_callback_received_bytes = 0;
  EXPECT_EQ(send_s.size(), pub.Send(send_s));

  // let the data flow
  eCAL::Process::SleepMS(DATA_FLOW_TIME);

  // check callback receive
  EXPECT_EQ(0, g_callback_received_bytes);

  // add callback again
  EXPECT_EQ(true, sub.AddReceiveCallback(std::bind(OnReceive, std::placeholders::_1, std::placeholders::_2)));

  // send content
  g_callback_received_bytes = 0;
  EXPECT_EQ(send_s.size(), pub.Send(send_s));

  // let the data flow
  eCAL::Process::SleepMS(DATA_FLOW_TIME);

  // check callback receive
  EXPECT_EQ(send_s.size(), g_callback_received_bytes);

  // destroy subscriber
  sub.Destroy();

  // send content
  g_callback_received_bytes = 0;
  EXPECT_EQ(send_s.size(), pub.Send(send_s));

  // let the data flow
  eCAL::Process::SleepMS(DATA_FLOW_TIME);

  // check callback receive
  EXPECT_EQ(0, g_callback_received_bytes);

  // destroy publisher
  pub.Destroy();

  // finalize eCAL API
  eCAL::Finalize();
}

TEST(IO, DynamicSizeCB)
{ 
  // default send string
  std::string send_s = CreatePayLoad(PAYLOAD_SIZE);

  // initialize eCAL API
  eCAL::Initialize(0, nullptr, "pubsub_test");

  // publish / subscribe match in the same process
  eCAL::Util::EnableLoopback(true);

  // create subscriber for topic "A"
  eCAL::CSubscriber sub("A");

  // create publisher for topic "A"
  eCAL::CPublisher pub("A");

  // add callback
  EXPECT_EQ(true, sub.AddReceiveCallback(std::bind(OnReceive, std::placeholders::_1, std::placeholders::_2)));

  // let's match them
  eCAL::Process::SleepMS(2 * CMN_REGISTRATION_REFRESH);

  // send content
  g_callback_received_bytes = 0;
  EXPECT_EQ(send_s.size(), pub.Send(send_s));

  // let the data flow
  eCAL::Process::SleepMS(DATA_FLOW_TIME);

  // check callback receive
  EXPECT_EQ(send_s.size(), g_callback_received_bytes);

  // increase payload size
  send_s = CreatePayLoad(PAYLOAD_SIZE*10);

  // send content
  g_callback_received_bytes = 0;
  EXPECT_EQ(send_s.size(), pub.Send(send_s));

  // let the data flow
  eCAL::Process::SleepMS(DATA_FLOW_TIME);

  // check callback receive
  EXPECT_EQ(send_s.size(), g_callback_received_bytes);

  // destroy publisher
  pub.Destroy();

  // destroy subscriber
  sub.Destroy();

  // finalize eCAL API
  eCAL::Finalize();
}

TEST(IO, DynamicCreate)
{ 
  // default send string
  std::string send_s = CreatePayLoad(PAYLOAD_SIZE);

  // initialize eCAL API
  eCAL::Initialize(0, nullptr, "pubsub_test");

  // publish / subscribe match in the same process
  eCAL::Util::EnableLoopback(true);

  // create subscriber for topic "A"
  eCAL::CSubscriber* sub;
  sub = new eCAL::CSubscriber("A");

  // create publisher for topic "A"
  eCAL::CPublisher* pub;
  pub = new eCAL::CPublisher("A");

  // add callback
  EXPECT_EQ(true, sub->AddReceiveCallback(std::bind(OnReceive, std::placeholders::_1, std::placeholders::_2)));

  // let's match them
  eCAL::Process::SleepMS(2 * CMN_REGISTRATION_REFRESH);

  // send content
  g_callback_received_bytes = 0;
  EXPECT_EQ(send_s.size(), pub->Send(send_s));

  // let the data flow
  eCAL::Process::SleepMS(DATA_FLOW_TIME);

  // check callback receive
  EXPECT_EQ(send_s.size(), g_callback_received_bytes);

  // destroy subscriber
  delete sub;
  sub = nullptr;

  // create subscriber for topic "A"
  sub = new eCAL::CSubscriber("A");

  // add callback
  EXPECT_EQ(true, sub->AddReceiveCallback(std::bind(OnReceive, std::placeholders::_1, std::placeholders::_2)));

  // let's match them
  eCAL::Process::SleepMS(2 * CMN_REGISTRATION_REFRESH);

  // send content
  g_callback_received_bytes = 0;
  EXPECT_EQ(send_s.size(), pub->Send(send_s));

  // let the data flow
  eCAL::Process::SleepMS(DATA_FLOW_TIME);

  // check callback receive
  EXPECT_EQ(send_s.size(), g_callback_received_bytes);

  // destroy subscriber
  sub->Destroy();

  // create subscriber for topic "A"
  sub->Create("A");

  // add callback
  EXPECT_EQ(true, sub->AddReceiveCallback(std::bind(OnReceive, std::placeholders::_1, std::placeholders::_2)));

  // let's match them
  eCAL::Process::SleepMS(2 * CMN_REGISTRATION_REFRESH);

  // send content
  g_callback_received_bytes = 0;
  EXPECT_EQ(send_s.size(), pub->Send(send_s));

  // let the data flow
  eCAL::Process::SleepMS(DATA_FLOW_TIME);

  // check callback receive
  EXPECT_EQ(send_s.size(), g_callback_received_bytes);

  // destroy publisher
  delete pub;
  pub = nullptr;

  // destroy subscriber
  delete sub;
  sub = nullptr;

  // finalize eCAL API
  eCAL::Finalize();
}

TEST(IO, SimpleMessageCBSHMBufferCount)
{
  // default send string
  std::string send_s = CreatePayLoad(PAYLOAD_SIZE);

  // initialize eCAL API
  eCAL::Initialize(0, nullptr, "pubsub_test");

  // publish / subscribe match in the same process
  eCAL::Util::EnableLoopback(true);

  // create subscriber for topic "A"
  eCAL::CSubscriber sub("A");

  // create publisher for topic "A"
  eCAL::CPublisher pub("A");
  pub.SetLayerMode(eCAL::TLayer::tlayer_all, eCAL::TLayer::smode_off);
  pub.SetLayerMode(eCAL::TLayer::tlayer_shm, eCAL::TLayer::smode_on);
  pub.ShmSetBufferCount(2);

  std::atomic<size_t> received_count{ 0 };
  std::atomic<size_t> received_bytes{ 0 };

  // add callback
  auto lambda = [&received_count, &received_bytes](const char* /*topic_name_*/, const eCAL::SReceiveCallbackData* data_) {
    received_bytes += data_->size;
    ++received_count;
  };
  EXPECT_EQ(true, sub.AddReceiveCallback(lambda));

  // let's match them
  eCAL::Process::SleepMS(2 * CMN_REGISTRATION_REFRESH);

  int iterations = 50;
  for (int i = 0; i < iterations; ++i)
  {
    // send content
    EXPECT_EQ(send_s.size(), pub.Send(send_s));

    // let the data flow
    eCAL::Process::SleepMS(DATA_FLOW_TIME);
  }

  // check callback receive
  EXPECT_EQ(send_s.size() * iterations, received_bytes);
  EXPECT_EQ(iterations, received_count);

  // finalize eCAL API
  eCAL::Finalize();
}

TEST(IO, ZeroPayloadMessageInProc)
{
  // default send string
  std::string send_s;

  // initialize eCAL API
  eCAL::Initialize(0, nullptr, "pubsub_test");

  // publish / subscribe match in the same process
  eCAL::Util::EnableLoopback(true);

  // create subscriber for topic "A"
  eCAL::CSubscriber sub("A");

  // create publisher for topic "A"
  eCAL::CPublisher pub("A");
  pub.SetLayerMode(eCAL::TLayer::tlayer_all,    eCAL::TLayer::smode_off);
  pub.SetLayerMode(eCAL::TLayer::tlayer_inproc, eCAL::TLayer::smode_on);

  // add callback
  EXPECT_EQ(true, sub.AddReceiveCallback(std::bind(OnReceive, std::placeholders::_1, std::placeholders::_2)));

  // let's match them
  eCAL::Process::SleepMS(2 * CMN_REGISTRATION_REFRESH);

  g_callback_received_bytes = 0;
  g_callback_received_count = 0;

  EXPECT_EQ(send_s.size(), pub.Send(send_s));
  eCAL::Process::SleepMS(DATA_FLOW_TIME);

  EXPECT_EQ(send_s.size(), pub.Send(nullptr, 0));
  eCAL::Process::SleepMS(DATA_FLOW_TIME);

  // check callback receive
  EXPECT_EQ(send_s.size(), g_callback_received_bytes);
  EXPECT_EQ(2,             g_callback_received_count);

  // destroy subscriber
  sub.Destroy();

  // destroy publisher
  pub.Destroy();

  // finalize eCAL API
  eCAL::Finalize();
}

TEST(IO, ZeroPayloadMessageSHM)
{
  // default send string
  std::string send_s;

  // initialize eCAL API
  eCAL::Initialize(0, nullptr, "pubsub_test");

  // publish / subscribe match in the same process
  eCAL::Util::EnableLoopback(true);

  // create subscriber for topic "A"
  eCAL::CSubscriber sub("A");

  // create publisher for topic "A"
  eCAL::CPublisher pub("A");
  pub.SetLayerMode(eCAL::TLayer::tlayer_all, eCAL::TLayer::smode_off);
  pub.SetLayerMode(eCAL::TLayer::tlayer_shm, eCAL::TLayer::smode_on);

  // add callback
  EXPECT_EQ(true, sub.AddReceiveCallback(std::bind(OnReceive, std::placeholders::_1, std::placeholders::_2)));

  // let's match them
  eCAL::Process::SleepMS(2 * CMN_REGISTRATION_REFRESH);

  g_callback_received_bytes = 0;
  g_callback_received_count = 0;

  EXPECT_EQ(send_s.size(), pub.Send(send_s));
  eCAL::Process::SleepMS(DATA_FLOW_TIME);

  EXPECT_EQ(send_s.size(), pub.Send(nullptr, 0));
  eCAL::Process::SleepMS(DATA_FLOW_TIME);

  // check callback receive
  EXPECT_EQ(send_s.size(), g_callback_received_bytes);
  EXPECT_EQ(2,             g_callback_received_count);

  // destroy subscriber
  sub.Destroy();

  // destroy publisher
  pub.Destroy();

  // finalize eCAL API
  eCAL::Finalize();
}

TEST(IO, ZeroPayloadMessageUDP)
{
  // default send string
  std::string send_s;

  // initialize eCAL API
  eCAL::Initialize(0, nullptr, "pubsub_test");

  // publish / subscribe match in the same process
  eCAL::Util::EnableLoopback(true);

  // create subscriber for topic "A"
  eCAL::CSubscriber sub("A");

  // create publisher for topic "A"
  eCAL::CPublisher pub("A");
  pub.SetLayerMode(eCAL::TLayer::tlayer_all, eCAL::TLayer::smode_off);
  pub.SetLayerMode(eCAL::TLayer::tlayer_udp_mc, eCAL::TLayer::smode_on);

  // add callback
  EXPECT_EQ(true, sub.AddReceiveCallback(std::bind(OnReceive, std::placeholders::_1, std::placeholders::_2)));

  // let's match them
  eCAL::Process::SleepMS(2 * CMN_REGISTRATION_REFRESH);

  g_callback_received_bytes = 0;
  g_callback_received_count = 0;

  EXPECT_EQ(send_s.size(), pub.Send(send_s));
  eCAL::Process::SleepMS(DATA_FLOW_TIME);

  EXPECT_EQ(send_s.size(), pub.Send(nullptr, 0));
  eCAL::Process::SleepMS(DATA_FLOW_TIME);

  // check callback receive
  EXPECT_EQ(send_s.size(), g_callback_received_bytes);
  EXPECT_EQ(2,             g_callback_received_count);

  // destroy subscriber
  sub.Destroy();

  // destroy publisher
  pub.Destroy();

  // finalize eCAL API
  eCAL::Finalize();
}

#if 0
TEST(IO, ZeroPayloadMessageTCP)
{
  // default send string
  std::string send_s;

  // initialize eCAL API
  eCAL::Initialize(0, nullptr, "pubsub_test");

  // publish / subscribe match in the same process
  eCAL::Util::EnableLoopback(true);

  // create subscriber for topic "A"
  eCAL::CSubscriber sub("A");

  // create publisher for topic "A"
  eCAL::CPublisher pub("A");
  pub.SetLayerMode(eCAL::TLayer::tlayer_all, eCAL::TLayer::smode_off);
  pub.SetLayerMode(eCAL::TLayer::tlayer_tcp, eCAL::TLayer::smode_on);

  // add callback
  EXPECT_EQ(true, sub.AddReceiveCallback(std::bind(OnReceive, std::placeholders::_1, std::placeholders::_2)));

  // let's match them
  eCAL::Process::SleepMS(2 * CMN_REGISTRATION_REFRESH);

  g_callback_received_bytes = 0;
  g_callback_received_count = 0;

  EXPECT_EQ(send_s.size(), pub.Send(send_s));
  eCAL::Process::SleepMS(DATA_FLOW_TIME);

  EXPECT_EQ(send_s.size(), pub.Send(nullptr, 0));
  eCAL::Process::SleepMS(DATA_FLOW_TIME);

  // check callback receive
  EXPECT_EQ(send_s.size(), g_callback_received_bytes);
  EXPECT_EQ(2,             g_callback_received_count);

  // destroy subscriber
  sub.Destroy();

  // destroy publisher
  pub.Destroy();

  // finalize eCAL API
  eCAL::Finalize();
}
#endif

#include <ecal/msg/string/publisher.h>
#include <ecal/msg/string/subscriber.h>
TEST(IO, DestroyInCallback)
{
  /* Test setup :
   * 2 pair of pub_sub connections ("foo" and "destroy")
   * "foo" publisher sends message every 100ms
   * "destroy" publisher sends destroy message after 2 seconds, which will destroy foo subscriber in its callback
   * Test ensures that this does not create a deadlock or other unintended situation.
  */

  // initialize eCAL API
  eCAL::Initialize(0, nullptr, "New Publisher in Callback");

  // enable loop back communication in the same thread
  eCAL::Util::EnableLoopback(true);

  // start publishing thread
  eCAL::string::CPublisher<std::string> pub_foo("foo");
  eCAL::string::CSubscriber<std::string> sub_foo("foo");

  eCAL::string::CPublisher<std::string> pub_destroy("destroy");
  eCAL::string::CSubscriber<std::string> sub_destroy("destroy");
  std::atomic<bool> destroyed(false);

  auto destroy_lambda = [&sub_foo, &destroyed](const char* /*topic_*/, const std::string& /*msg*/, long long /*time_*/, long long /*clock_*/, long long /*id_*/) {
    std::cout << "Receive destroy command" << std::endl;
    sub_foo.Destroy();
    destroyed = true;
    std::cout << "Finnished destroying" << std::endl;
  };
  sub_destroy.AddReceiveCallback(destroy_lambda);

  auto receive_lambda = [](const char* /*topic_*/, const std::string& /*msg*/, long long /*time_*/, long long /*clock_*/, long long /*id_*/) {
    std::cout << "Hello" << std::endl;
  };
  sub_foo.AddReceiveCallback(receive_lambda);

  // sleep for 2 seconds, registration should be good!
  std::this_thread::sleep_for(std::chrono::seconds(2));

  std::thread pub_foo_t([&pub_foo, &destroyed]() {
    while (!destroyed)
    {
      pub_foo.Send("Hello World");
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    std::cout << "Stopped sending foo" << std::endl;
    });

  std::thread pub_destroy_t([&pub_destroy]() {
    // sleep for two second, then send the destroy command
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "Sending destroy message" << std::endl;
    pub_destroy.Send("Destroy");
    std::cout << "Done sending destroy message" << std::endl;
    });


  pub_foo_t.join();
  pub_destroy_t.join();

  // finalize eCAL API
  // without destroying any pub / sub
  eCAL::Finalize();
}