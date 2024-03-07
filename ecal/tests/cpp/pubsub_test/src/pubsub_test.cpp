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
#include <ecal/msg/string/publisher.h>
#include <ecal/msg/string/subscriber.h>

#include <atomic>
#include <thread>

#include <gtest/gtest.h>

#define CMN_REGISTRATION_REFRESH   1000
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

TEST(core_cpp_pubsub, LeakedPubSub)
{
  // initialize eCAL API
  EXPECT_EQ(0, eCAL::Initialize(0, nullptr, "leaked pub/sub"));

  // enable loop back communication in the same thread
  eCAL::Util::EnableLoopback(true);

  // create subscriber and register a callback
  eCAL::CSubscriber sub("foo");
  sub.AddReceiveCallback(std::bind(OnReceive, std::placeholders::_1, std::placeholders::_2));

  // create publisher
  eCAL::CPublisher pub("foo");

  // let's match them
  eCAL::Process::SleepMS(2 * CMN_REGISTRATION_REFRESH);

  // start publishing thread
  std::atomic<bool> pub_stop(false);
  std::thread pub_t([&]() {
    while (!pub_stop)
    {
      pub.Send("Hello World");
#if 0
      // some kind of busy waiting....
      int y = 0;
      for (int i = 0; i < 100000; i++)
      {
        y += i;
      }
#else
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
#endif
    }
    });

  // let them work together
  std::this_thread::sleep_for(std::chrono::seconds(2));

  // finalize eCAL API
  // without destroying any pub / sub
  EXPECT_EQ(0, eCAL::Finalize());

  // stop publishing thread
  pub_stop = true; pub_t.join();
}

TEST(core_cpp_pubsub, CallbackDestruction)
{
  // initialize eCAL API
  EXPECT_EQ(0, eCAL::Initialize(0, nullptr, "callback destruction"));

  // enable loop back communication in the same thread
  eCAL::Util::EnableLoopback(true);

  // create subscriber and register a callback
  std::shared_ptr<eCAL::CSubscriber> sub;

  // create publisher
  eCAL::CPublisher pub("foo");

  // start publishing thread
  std::atomic<bool> pub_stop(false);
  std::thread pub_t([&]() {
    while (!pub_stop)
    {
      pub.Send("Hello World");
#if 0
      // some kind of busy waiting....
      int y = 0;
      for (int i = 0; i < 100000; i++)
      {
        y += i;
      }
#else
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
#endif
    }
    });

  std::atomic<bool> sub_stop(false);
  std::thread sub_t([&]() {
    while (!sub_stop)
    {
      sub = std::make_shared<eCAL::string::CSubscriber<std::string>>("foo");
      sub->AddReceiveCallback(std::bind(OnReceive, std::placeholders::_1, std::placeholders::_2));
      std::this_thread::sleep_for(std::chrono::seconds(2));
    }
    });

  // let them work together
  std::this_thread::sleep_for(std::chrono::seconds(10));

  // stop publishing thread
  pub_stop = true;
  pub_t.join();

  sub_stop = true;
  sub_t.join();

  // finalize eCAL API
  // without destroying any pub / sub
  EXPECT_EQ(0, eCAL::Finalize());
}

TEST(core_cpp_pubsub, CreateDestroy)
{ 
  // initialize eCAL API
  eCAL::Initialize(0, nullptr, "pubsub_test");

  // create publisher for topic "foo"
  eCAL::CPublisher pub;

  // check state
  EXPECT_EQ(false, pub.IsCreated());

  // create
  EXPECT_EQ(true, pub.Create("foo"));

  // check state
  EXPECT_EQ(true, pub.IsCreated());

  // create subscriber for topic "foo"
  eCAL::CSubscriber sub;

  // check state
  EXPECT_EQ(false, sub.IsCreated());

  // create
  EXPECT_EQ(true, sub.Create("foo"));

  // check state
  EXPECT_EQ(true, sub.IsCreated());

  // destroy publisher
  EXPECT_EQ(true, pub.Destroy());

  // destroy subscriber
  EXPECT_EQ(true, sub.Destroy());

  // finalize eCAL API
  eCAL::Finalize();
}

TEST(core_cpp_pubsub, SimpleMessage1)
{ 
  // default send / receive strings
  std::string send_s = CreatePayLoad(PAYLOAD_SIZE);
  std::string recv_s;

  // initialize eCAL API
  eCAL::Initialize(0, nullptr, "pubsub_test");

  // publish / subscribe match in the same process
  eCAL::Util::EnableLoopback(true);

  // create publisher for topic "foo"
  eCAL::CPublisher pub("foo");

  // create subscriber for topic "foo"
  eCAL::CSubscriber sub("foo");

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

TEST(core_cpp_pubsub, SimpleMessage2)
{ 
  // default send / receive strings
  std::string send_s = CreatePayLoad(PAYLOAD_SIZE);
  std::string recv_s;

  // initialize eCAL API
  eCAL::Initialize(0, nullptr, "pubsub_test");

  // publish / subscribe match in the same process
  eCAL::Util::EnableLoopback(true);

  // create subscriber for topic "foo"
  eCAL::CSubscriber sub("foo");

  // create publisher for topic "foo"
  eCAL::CPublisher pub("foo");

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

TEST(core_cpp_pubsub, SimpleMessageCB)
{ 
  // default send string
  std::string send_s = CreatePayLoad(PAYLOAD_SIZE);

  // initialize eCAL API
  eCAL::Initialize(0, nullptr, "pubsub_test");

  // publish / subscribe match in the same process
  eCAL::Util::EnableLoopback(true);

  // create subscriber for topic "foo"
  eCAL::CSubscriber sub("foo");

  // create publisher for topic "foo"
  eCAL::CPublisher pub("foo");

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

TEST(core_cpp_pubsub, DynamicSizeCB)
{ 
  // default send string
  std::string send_s = CreatePayLoad(PAYLOAD_SIZE);

  // initialize eCAL API
  eCAL::Initialize(0, nullptr, "pubsub_test");

  // publish / subscribe match in the same process
  eCAL::Util::EnableLoopback(true);

  // create subscriber for topic "foo"
  eCAL::CSubscriber sub("foo");

  // create publisher for topic "foo"
  eCAL::CPublisher pub("foo");

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

TEST(core_cpp_pubsub, DynamicCreate)
{ 
  // default send string
  std::string send_s = CreatePayLoad(PAYLOAD_SIZE);

  // initialize eCAL API
  eCAL::Initialize(0, nullptr, "pubsub_test");

  // publish / subscribe match in the same process
  eCAL::Util::EnableLoopback(true);

  // create subscriber for topic "foo"
  eCAL::CSubscriber* sub;
  sub = new eCAL::CSubscriber("foo");

  // create publisher for topic "foo"
  eCAL::CPublisher* pub;
  pub = new eCAL::CPublisher("foo");

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

  // create subscriber for topic "foo"
  sub = new eCAL::CSubscriber("foo");

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

  // create subscriber for topic "foo"
  sub->Create("foo");

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

TEST(core_cpp_pubsub, ZeroPayloadMessageUDP)
{
  // default send string
  std::string send_s;

  // initialize eCAL API
  eCAL::Initialize(0, nullptr, "pubsub_test");

  // publish / subscribe match in the same process
  eCAL::Util::EnableLoopback(true);

  // create subscriber for topic "foo"
  eCAL::CSubscriber sub("foo");

  // create publisher for topic "foo"
  eCAL::CPublisher pub("foo");

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

TEST(core_cpp_pubsub, MultipleSendsUDP)
{
  // default send string
  std::vector<std::string> send_vector{ "this", "is", "a", "", "testtest" };
  std::string last_received_msg;
  long long   last_received_timestamp;

  // initialize eCAL API
  eCAL::Initialize(0, nullptr, "pubsub_test");

  // publish / subscribe match in the same process
  eCAL::Util::EnableLoopback(true);

  // create subscriber for topic "foo"
  eCAL::string::CSubscriber<std::string> sub("foo");

  // create publisher for topic "foo"
  eCAL::string::CPublisher<std::string> pub("foo");

  // add callback
  auto save_data = [&last_received_msg, &last_received_timestamp](const char* /*topic_name_*/, const std::string& msg_, long long time_, long long /*clock_*/, long long /*id_*/)
  {
    last_received_msg = msg_;
    last_received_timestamp = time_;
  };
  EXPECT_TRUE(sub.AddReceiveCallback(save_data));

  // let's match them
  eCAL::Process::SleepMS(2 * CMN_REGISTRATION_REFRESH);
  long long timestamp = 1;
  for (const auto& elem : send_vector)
  {
    pub.Send(elem, timestamp);
    eCAL::Process::SleepMS(DATA_FLOW_TIME);
    EXPECT_EQ(last_received_msg, elem);
    EXPECT_EQ(last_received_timestamp, timestamp);
    ++timestamp;
  }

  // destroy subscriber
  sub.Destroy();

  // destroy publisher
  pub.Destroy();

  // finalize eCAL API
  eCAL::Finalize();
}

TEST(core_cpp_pubsub, DestroyInCallback)
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

TEST(core_cpp_pubsub, SubscriberReconnection)
{
  /* Test setup :
   * publisher runs permanently in a thread
   * subscriber start reading
   * subscriber gets out of scope (destruction)
   * subscriber starts again in a new scope
   * Test ensures that subscriber is reconnecting and all sync mechanism are working properly again.
  */

  // initialize eCAL API
  eCAL::Initialize(0, nullptr, "SubscriberReconnection");

  // enable loop back communication in the same thread
  eCAL::Util::EnableLoopback(true);

  // start publishing thread
  std::atomic<bool> stop_publishing(false);
  eCAL::string::CPublisher<std::string> pub_foo("foo");
  std::thread pub_foo_t([&pub_foo, &stop_publishing]() {
    while (!stop_publishing)
    {
      pub_foo.Send("Hello World");
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    std::cout << "Stopped publishing" << std::endl;
  });

  // scope 1
  {
    size_t callback_received_count(0);

    eCAL::string::CSubscriber<std::string> sub_foo("foo");
    auto receive_lambda = [&sub_foo, &callback_received_count](const char* /*topic_*/, const std::string& /*msg*/, long long /*time_*/, long long /*clock_*/, long long /*id_*/) {
      std::cout << "Receiving in scope 1" << std::endl;
      callback_received_count++;
    };
    sub_foo.AddReceiveCallback(receive_lambda);

    // sleep for 2 seconds, we should receive something
    std::this_thread::sleep_for(std::chrono::seconds(2));

    EXPECT_TRUE(callback_received_count > 0);
  }

  // scope 2
  {
    size_t callback_received_count(0);

    eCAL::string::CSubscriber<std::string> sub_foo("foo");
    auto receive_lambda = [&sub_foo, &callback_received_count](const char* /*topic_*/, const std::string& /*msg*/, long long /*time_*/, long long /*clock_*/, long long /*id_*/) {
      std::cout << "Receiving in scope 2" << std::endl;
      callback_received_count++;
    };
    sub_foo.AddReceiveCallback(receive_lambda);

    // sleep for 2 seconds, we should receive something
    std::this_thread::sleep_for(std::chrono::seconds(2));

    EXPECT_TRUE(callback_received_count > 0);
  }

  // stop publishing and join thread
  stop_publishing = true;
  pub_foo_t.join();

  // finalize eCAL API
  // without destroying any pub / sub
  eCAL::Finalize();
}
