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

#include <gtest/gtest.h>

#define CMN_REGISTRATION_REFRESH   1000
#define DATA_FLOW_TIME               50
#define PAYLOAD_SIZE               1024

// subscriber callback function
std::atomic<size_t> g_callback_received;
void OnReceive(const char* /*topic_name_*/, const struct eCAL::SReceiveCallbackData* data_)
{
  g_callback_received += data_->size;
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

TEST(IO, SimpleMessage1)
{ 
  // default send string
  std::string send_s = CreatePayLoad(PAYLOAD_SIZE);

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
  EXPECT_EQ(send_s.size(), sub.Receive(send_s, nullptr, DATA_FLOW_TIME));

  // receive content with DATA_FLOW_TIME ms timeout
  // should return because no new publishing
  EXPECT_EQ(0, sub.Receive(send_s, nullptr, DATA_FLOW_TIME));

  // destroy publisher
  pub.Destroy();

  // destroy subscriber
  sub.Destroy();

  // finalize eCAL API
  eCAL::Finalize();
}

TEST(IO, SimpleMessage2)
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

  // let's match them
  eCAL::Process::SleepMS(2 * CMN_REGISTRATION_REFRESH);

  // send content
  EXPECT_EQ(send_s.size(), pub.Send(send_s));

  // receive content with DATA_FLOW_TIME ms timeout
  EXPECT_EQ(send_s.size(), sub.Receive(send_s, nullptr, DATA_FLOW_TIME));

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
  g_callback_received = 0;
  EXPECT_EQ(send_s.size(), pub.Send(send_s));

  // let the data flow
  eCAL::Process::SleepMS(DATA_FLOW_TIME);

  // check callback receive
  EXPECT_EQ(send_s.size(), g_callback_received);

  // remove receive callback
  sub.RemReceiveCallback();

  // send content
  g_callback_received = 0;
  EXPECT_EQ(send_s.size(), pub.Send(send_s));

  // let the data flow
  eCAL::Process::SleepMS(DATA_FLOW_TIME);

  // check callback receive
  EXPECT_EQ(0, g_callback_received);

  // add callback again
  EXPECT_EQ(true, sub.AddReceiveCallback(std::bind(OnReceive, std::placeholders::_1, std::placeholders::_2)));

  // send content
  g_callback_received = 0;
  EXPECT_EQ(send_s.size(), pub.Send(send_s));

  // let the data flow
  eCAL::Process::SleepMS(DATA_FLOW_TIME);

  // check callback receive
  EXPECT_EQ(send_s.size(), g_callback_received);

  // destroy subscriber
  sub.Destroy();

  // send content
  g_callback_received = 0;
  EXPECT_EQ(send_s.size(), pub.Send(send_s));

  // let the data flow
  eCAL::Process::SleepMS(DATA_FLOW_TIME);

  // check callback receive
  EXPECT_EQ(0, g_callback_received);

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
  g_callback_received = 0;
  EXPECT_EQ(send_s.size(), pub.Send(send_s));

  // let the data flow
  eCAL::Process::SleepMS(DATA_FLOW_TIME);

  // check callback receive
  EXPECT_EQ(send_s.size(), g_callback_received);

  // increase payload size
  send_s = CreatePayLoad(PAYLOAD_SIZE*10);

  // send content
  g_callback_received = 0;
  EXPECT_EQ(send_s.size(), pub.Send(send_s));

  // let the data flow
  eCAL::Process::SleepMS(DATA_FLOW_TIME);

  // check callback receive
  EXPECT_EQ(send_s.size(), g_callback_received);

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
  g_callback_received = 0;
  EXPECT_EQ(send_s.size(), pub->Send(send_s));

  // let the data flow
  eCAL::Process::SleepMS(DATA_FLOW_TIME);

  // check callback receive
  EXPECT_EQ(send_s.size(), g_callback_received);

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
  g_callback_received = 0;
  EXPECT_EQ(send_s.size(), pub->Send(send_s));

  // let the data flow
  eCAL::Process::SleepMS(DATA_FLOW_TIME);

  // check callback receive
  EXPECT_EQ(send_s.size(), g_callback_received);

  // destroy subscriber
  sub->Destroy();

  // create subscriber for topic "A"
  sub->Create("A");

  // add callback
  EXPECT_EQ(true, sub->AddReceiveCallback(std::bind(OnReceive, std::placeholders::_1, std::placeholders::_2)));

  // let's match them
  eCAL::Process::SleepMS(2 * CMN_REGISTRATION_REFRESH);

  // send content
  g_callback_received = 0;
  EXPECT_EQ(send_s.size(), pub->Send(send_s));

  // let the data flow
  eCAL::Process::SleepMS(DATA_FLOW_TIME);

  // check callback receive
  EXPECT_EQ(send_s.size(), g_callback_received);

  // destroy publisher
  delete pub;
  pub = nullptr;

  // destroy subscriber
  delete sub;
  sub = nullptr;

  // finalize eCAL API
  eCAL::Finalize();
}
