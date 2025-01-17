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
#include <cstddef>
#include <ecal/ecal.h>
#include <ecal/ecal_publisher.h>
#include <ecal/ecal_subscriber.h>

#include <atomic>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include <gtest/gtest.h>

enum {
  CMN_REGISTRATION_REFRESH_MS = 1000,
  DATA_FLOW_TIME_MS = 50,
  PAYLOAD_SIZE_BYTE = 1024
};

namespace
{
  // subscriber callback function
  std::atomic<size_t> g_callback_received_bytes;
  std::atomic<size_t> g_callback_received_count;
  void OnReceive(const struct eCAL::SReceiveCallbackData& data_)
  {
    g_callback_received_bytes += data_.size;
    g_callback_received_count++;
  }

  std::string CreatePayLoad(size_t payload_size_)
  {
    std::string s = "Hello World ";
    while(s.size() < payload_size_)
    {
      s += s;
    }
    s.resize(payload_size_);
    return(s);
  }
}

TEST(core_cpp_pubsub, LeakedPubSub)
{
  // initialize eCAL API
  EXPECT_EQ(true, eCAL::Initialize("leaked pub/sub"));

  // create subscriber and register a callback
  eCAL::CSubscriber sub("foo");
  sub.SetReceiveCallback(std::bind(OnReceive, std::placeholders::_3));

  // create publisher
  eCAL::CPublisher pub("foo");

  // let's match them
  eCAL::Process::SleepMS(2 * CMN_REGISTRATION_REFRESH_MS);

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
  EXPECT_EQ(true, eCAL::Finalize());

  // stop publishing thread
  pub_stop = true; pub_t.join();
}

TEST(core_cpp_pubsub, CallbackDestruction)
{
  // initialize eCAL API
  EXPECT_EQ(true, eCAL::Initialize("callback destruction"));

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
      sub = std::make_shared<eCAL::CSubscriber>("foo");
      sub->SetReceiveCallback(std::bind(OnReceive, std::placeholders::_3));
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
  EXPECT_EQ(true, eCAL::Finalize());
}

TEST(core_cpp_pubsub, SimpleMessageCB)
{ 
  // default send string
  const std::string send_s = CreatePayLoad(PAYLOAD_SIZE_BYTE);

  // initialize eCAL API
  eCAL::Initialize("pubsub_test");

  // create subscriber for topic "foo"
  auto sub = std::make_shared<eCAL::CSubscriber>("foo");

  // create publisher for topic "foo"
  auto pub = std::make_shared<eCAL::CPublisher>("foo");

  // add callback
  EXPECT_TRUE(sub->SetReceiveCallback(std::bind(OnReceive, std::placeholders::_3)));

  // let's match them
  eCAL::Process::SleepMS(2 * CMN_REGISTRATION_REFRESH_MS);

  // send content
  g_callback_received_bytes = 0;
  EXPECT_TRUE(pub->Send(send_s));

  // let the data flow
  eCAL::Process::SleepMS(DATA_FLOW_TIME_MS);

  // check callback receive
  EXPECT_EQ(send_s.size(), g_callback_received_bytes);

  // remove receive callback
  sub->RemoveReceiveCallback();

  // send content
  g_callback_received_bytes = 0;
  EXPECT_TRUE(pub->Send(send_s));

  // let the data flow
  eCAL::Process::SleepMS(DATA_FLOW_TIME_MS);

  // check callback receive
  EXPECT_EQ(0, g_callback_received_bytes);

  // add callback again
  EXPECT_TRUE(sub->SetReceiveCallback(std::bind(OnReceive, std::placeholders::_3)));

  // send content
  g_callback_received_bytes = 0;
  EXPECT_TRUE(pub->Send(send_s));

  // let the data flow
  eCAL::Process::SleepMS(DATA_FLOW_TIME_MS);

  // check callback receive
  EXPECT_EQ(send_s.size(), g_callback_received_bytes);

  // destroy subscriber
  sub.reset();

  // send content
  g_callback_received_bytes = 0;
  EXPECT_TRUE(pub->Send(send_s));

  // let the data flow
  eCAL::Process::SleepMS(DATA_FLOW_TIME_MS);

  // check callback receive
  EXPECT_EQ(0, g_callback_received_bytes);

  // finalize eCAL API
  eCAL::Finalize();
}

TEST(core_cpp_pubsub, DynamicSizeCB)
{ 
  // default send string
  std::string send_s = CreatePayLoad(PAYLOAD_SIZE_BYTE);

  // initialize eCAL API
  eCAL::Initialize("pubsub_test");

  // create subscriber for topic "foo"
  auto sub = std::make_shared<eCAL::CSubscriber>("foo");

  // create publisher for topic "foo"
  auto pub = std::make_shared<eCAL::CPublisher>("foo");

  // add callback
  EXPECT_TRUE(sub->SetReceiveCallback(std::bind(OnReceive, std::placeholders::_3)));

  // let's match them
  eCAL::Process::SleepMS(2 * CMN_REGISTRATION_REFRESH_MS);

  // send content
  g_callback_received_bytes = 0;
  EXPECT_TRUE(pub->Send(send_s));

  // let the data flow
  eCAL::Process::SleepMS(DATA_FLOW_TIME_MS);

  // check callback receive
  EXPECT_EQ(send_s.size(), g_callback_received_bytes);

  // increase payload size
  send_s = CreatePayLoad(PAYLOAD_SIZE_BYTE*10);

  // send content
  g_callback_received_bytes = 0;
  EXPECT_TRUE(pub->Send(send_s));

  // let the data flow
  eCAL::Process::SleepMS(DATA_FLOW_TIME_MS);

  // check callback receive
  EXPECT_EQ(send_s.size(), g_callback_received_bytes);

  // destroy subscriber
  sub.reset();

  // finalize eCAL API
  eCAL::Finalize();
}

TEST(core_cpp_pubsub, DynamicCreate)
{ 
  // default send string
  const std::string send_s = CreatePayLoad(PAYLOAD_SIZE_BYTE);

  // initialize eCAL API
  eCAL::Initialize("pubsub_test");

  // create subscriber for topic "foo"
  eCAL::CSubscriber* sub;
  sub = new eCAL::CSubscriber("foo");

  // create publisher for topic "foo"
  eCAL::CPublisher* pub;
  pub = new eCAL::CPublisher("foo");

  // add callback
  EXPECT_TRUE(sub->SetReceiveCallback(std::bind(OnReceive, std::placeholders::_3)));

  // let's match them
  eCAL::Process::SleepMS(2 * CMN_REGISTRATION_REFRESH_MS);

  // send content
  g_callback_received_bytes = 0;
  EXPECT_TRUE(pub->Send(send_s));

  // let the data flow
  eCAL::Process::SleepMS(DATA_FLOW_TIME_MS);

  // check callback receive
  EXPECT_EQ(send_s.size(), g_callback_received_bytes);

  // destroy subscriber
  delete sub;
  sub = nullptr;

  // create subscriber for topic "foo"
  sub = new eCAL::CSubscriber("foo");

  // add callback
  EXPECT_TRUE(sub->SetReceiveCallback(std::bind(OnReceive, std::placeholders::_3)));

  // let's match them
  eCAL::Process::SleepMS(2 * CMN_REGISTRATION_REFRESH_MS);

  // send content
  g_callback_received_bytes = 0;
  EXPECT_TRUE(pub->Send(send_s));

  // let the data flow
  eCAL::Process::SleepMS(DATA_FLOW_TIME_MS);

  // check callback receive
  EXPECT_EQ(send_s.size(), g_callback_received_bytes);

  // destroy subscriber
  delete sub;

  // create subscriber for topic "foo"
  sub = new eCAL::CSubscriber("foo");

  // add callback
  EXPECT_TRUE(sub->SetReceiveCallback(std::bind(OnReceive, std::placeholders::_3)));

  // let's match them
  eCAL::Process::SleepMS(2 * CMN_REGISTRATION_REFRESH_MS);

  // send content
  g_callback_received_bytes = 0;
  EXPECT_TRUE(pub->Send(send_s));

  // let the data flow
  eCAL::Process::SleepMS(DATA_FLOW_TIME_MS);

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

// ATM, this test does no longer make sense, as V6 subscribers / publishers do not have a destroy function
// maybe this test needs to be rewritten to test the same by using pointers.
TEST(core_cpp_pubsub /*unused*/, DISABLED_DestroyInCallback /*unused*/)
{
  /* Test setup :
   * 2 pair of pub_sub connections ("foo" and "destroy")
   * "foo" publisher sends message every 100ms
   * "destroy" publisher sends destroy message after 2 seconds, which will destroy foo subscriber in its callback
   * Test ensures that this does not create a deadlock or other unintended situation.
  */

  // initialize eCAL API
  eCAL::Initialize("New Publisher in Callback");

  // start publishing thread
  eCAL::CPublisher pub_foo("foo");
  eCAL::CSubscriber sub_foo("foo");

  eCAL::CPublisher pub_destroy("destroy");
  eCAL::CSubscriber sub_destroy("destroy");
  std::atomic<bool> destroyed(false);

  auto destroy_lambda = [&sub_foo, &destroyed](const eCAL::Registration::STopicId& /*topic_id_*/, const eCAL::SDataTypeInformation& /*data_type_info_*/, const eCAL::SReceiveCallbackData& /*data_*/) {
    std::cout << "Receive destroy command" << std::endl;
    //sub_foo.Destroy();
    destroyed = true;
    std::cout << "Finnished destroying" << std::endl;
  };
  sub_destroy.SetReceiveCallback(destroy_lambda);

  auto receive_lambda = [](const eCAL::Registration::STopicId& /*topic_id_*/, const eCAL::SDataTypeInformation& /*data_type_info_*/, const eCAL::SReceiveCallbackData& /*data_*/) {
    std::cout << "Hello" << std::endl;
  };
  sub_foo.SetReceiveCallback(receive_lambda);

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
   * publishers runs permanently in a thread
   * subscriber A start reading topic A
   * subscriber A gets out of scope (destruction)
   * subscriber B start reading topic B
   * subscriber B gets out of scope (destruction)
   * subscriber A starts again in a new scope
   * Test ensures that subscriber is reconnecting and all sync mechanism are working properly again.
   * Previously this test was not catching a case where a delay between the destruction of the
   * topic A subscriber and its recreation would create a subscriber that was not receiving messages.
  */

  // initialize eCAL API
  eCAL::Initialize("SubscriberReconnection");

  constexpr auto RECEIVE_TIMEOUT = std::chrono::seconds(5);

  // start publishing thread
  std::atomic<bool> stop_publishing(false);
  const auto publish_messages = [&stop_publishing](eCAL::CPublisher &pub) {
    while (!stop_publishing) {
      pub.Send("Hello World");
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    std::cout << "Stopped publishing" << std::endl;
  };

  eCAL::CPublisher pub_foo("foo");
  std::thread pub_foo_t(publish_messages, std::ref(pub_foo));

  eCAL::CPublisher pub_bar("bar");
  std::thread pub_bar_t(publish_messages, std::ref(pub_bar));

  std::condition_variable cv;
  std::mutex cv_m;
  bool data_received(false);

  const auto receive_lambda =
      [&cv_m, &cv,
       &data_received](const eCAL::Registration::STopicId & /*topic_id_*/,
                       const eCAL::SDataTypeInformation & /*data_type_info_*/,
                       const eCAL::SReceiveCallbackData & /*data_*/) {
        {
          std::cout << "Callback received message" << std::endl;
          std::lock_guard<std::mutex> lk(cv_m);
          data_received = true;
        }
        cv.notify_all();
      };
  std::unique_lock<std::mutex> cv_lk(cv_m);

  // 1 - Subscribe to topic A and receive a message
  {
    eCAL::CSubscriber sub_foo("foo");
    sub_foo.SetReceiveCallback(receive_lambda);

    // We should receive something within the timeout period
    cv.wait_for(cv_lk,RECEIVE_TIMEOUT, [&data_received]() { return data_received; });

    EXPECT_TRUE(data_received);
    std::cout << "Closing first subscriber scope (foo)" << std::endl;
  }

  data_received = false; // Reset for next scope

  // 2 - Subscribe to topic B and receive a message
  {
    eCAL::CSubscriber sub_foo("bar");
    sub_foo.SetReceiveCallback(receive_lambda);

    // We should receive something within the timeout period
    cv.wait_for(cv_lk,RECEIVE_TIMEOUT, [&data_received]() { return data_received; });

    EXPECT_TRUE(data_received);
    std::cout << "Closing second subscriber scope (bar)" << std::endl;
  }

  data_received = false; // Reset for next scope

  // 3 - Subscribe to topic A again and receive a message
  // TODO: Figure out why this now fails
  {
    eCAL::CSubscriber sub_foo("foo");
    sub_foo.SetReceiveCallback(receive_lambda);

    // We should receive something within the timeout period
    cv.wait_for(cv_lk,RECEIVE_TIMEOUT, [&data_received]() { return data_received; });

    EXPECT_TRUE(data_received);
    std::cout << "Closing third subscriber scope (foo again)" << std::endl;
  }

  // stop publishing and join thread
  stop_publishing = true;
  pub_foo_t.join();
  pub_bar_t.join();

  // finalize eCAL API
  // without destroying any pub / sub
  eCAL::Finalize();
}
