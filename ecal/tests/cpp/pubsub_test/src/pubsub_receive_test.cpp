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

#include <ecal/ecal.h>
#include <ecal/msg/string/publisher.h>
#include <ecal/msg/string/subscriber.h>

#include <atomic>
#include <chrono>
#include <string>
#include <thread>

#include <gtest/gtest.h>

enum {
  CMN_REGISTRATION_REFRESH_MS = 1000
};

using namespace std::chrono_literals;

// define std::chrono::abs for < c++17
#if (__cpp_lib_chrono < 201510L)
namespace std
{
  namespace chrono
  {
      template <class T>
      T abs(T d)
      {
        return d >= d.zero() ? d : -d;
      }
  }
}
#endif

void measure_execution_within_range(const std::string& description, std::function<void()> func, std::chrono::milliseconds expected_runtime, std::chrono::microseconds epsilon)
{
  auto start = std::chrono::steady_clock::now();

  func();

  auto end = std::chrono::steady_clock::now();

  auto duration = end - start;
  auto divergence = std::chrono::abs(expected_runtime - duration);
  auto within_epsilon = divergence < epsilon;

  // How to do this nicely?
  EXPECT_TRUE(within_epsilon) << "Execution of " << description << " took " << duration.count() << " ns but expected was " << expected_runtime.count()*1000000 << "ns";
}

TEST(core_cpp_pubsub, TimingSubscriberReceive)
{
  // initialize eCAL API
  EXPECT_EQ(0, eCAL::Initialize(0, nullptr, "subscriber_receive_timing"));

  // publish / subscribe match in the same process
  eCAL::Util::EnableLoopback(true);

  // create simple string publisher
  eCAL::string::CPublisher<std::string> pub("CLOCK");
  eCAL::string::CSubscriber<std::string> sub("CLOCK");

  // let's match them
  eCAL::Process::SleepMS(2 * CMN_REGISTRATION_REFRESH_MS);

  // Send nothing and make sure the functions return as specified
  std::string received;
  // return with immediately
  measure_execution_within_range(
    "ReturnImmediate",
    [&sub, &received]() {sub.Receive(received); },
    0ms,
    10ms
  );

  // return with timeout
  // lets give a few more milliseconds delta, at least for windows
  measure_execution_within_range(
    "Return500ms",
    [&sub, &received]() {
      auto res = sub.Receive(received, nullptr, 500); 
      EXPECT_FALSE(res);
    },
    500ms,
    50ms
  );

  // All three send functions should return immediately even though timeouts were given, since data is available.
  pub.Send("Hi");
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  measure_execution_within_range(
    "ReceiveImmediate",
    [&sub, &received]() {
      auto res = sub.Receive(received); 
      EXPECT_TRUE(res);
    },
    0ms,
    10ms
  );

  measure_execution_within_range(
    "Return500ms_2",
    [&sub, &received]() {
      auto res = sub.Receive(received, nullptr, 500);
      EXPECT_FALSE(res);
    },
    500ms,
    50ms
   );

  pub.Send("There");
  measure_execution_within_range(
    "ReceiveImmediateInfinite",
    [&sub, &received]() {
      auto res = sub.Receive(received, nullptr, -1); 
      EXPECT_TRUE(res);
    },
    0ms,
    10ms
  );

  measure_execution_within_range(
    "Return500ms_3",
    [&sub, &received]() {
      auto res = sub.Receive(received, nullptr, 500);
      EXPECT_FALSE(res);
    },
    500ms,
    50ms
  );

  pub.Send("Person");
  measure_execution_within_range(
    "ReceiveImmediate500ms",
    [&sub, &received]() {
      auto res = sub.Receive(received, nullptr, 500);
      EXPECT_TRUE(res);
    },
    0ms,
    10ms
  );
  
  // finalize eCAL API
  EXPECT_EQ(0, eCAL::Finalize());
}

// This tests test for sporadically received empty messages which were a problem.
TEST(core_cpp_pubsub, SporadicEmptyReceives)
{ 
  // initialize eCAL API
  EXPECT_EQ(0, eCAL::Initialize(0, nullptr, "sporadic_empty_receives"));

  // publish / subscribe match in the same process
  eCAL::Util::EnableLoopback(true);

  // create simple string publisher
  eCAL::string::CPublisher<std::string> pub("CLOCK");
  eCAL::string::CSubscriber<std::string> sub("CLOCK");

  // let's match them
  eCAL::Process::SleepMS(2 * CMN_REGISTRATION_REFRESH_MS);

  // start publishing thread
  std::atomic<bool> pub_stop(false);
  std::thread pub_t([&pub, &pub_stop]() {
    const std::string abc{ "abc" };
    while (!pub_stop)
    {
      pub.Send(abc);
      std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    });

  std::atomic<bool> sub_stop(false);
  std::thread sub_t([&]() {
    std::string received;
    while (!sub_stop)
    {
      // we define a maximum timeout of 10 sec to not get locked forever here in worst case
      const bool got_data = sub.Receive(received, nullptr, 10*1000);
      if (got_data && received.empty())
      {
        FAIL() << "received empty string";
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
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
  EXPECT_EQ(0, eCAL::Finalize());
}

TEST(PubSub, TestSubscriberSeen)
{
  // initialize eCAL API
  EXPECT_EQ(0, eCAL::Initialize(0, nullptr, "subscriber_seen"));

  // enable data loopback
  eCAL::Util::EnableLoopback(true);

  std::atomic<bool> subscriber_seen_at_publication_start(false);
  std::atomic<bool> subscriber_seen_at_publication_end(false);

  std::atomic<bool> do_start_publication(false);
  std::atomic<bool> publication_finished(false);

  // publishing thread
  auto publisher_thread = [&]() {
    eCAL::Publisher::Configuration pub_config;
    pub_config.layer.shm.acknowledge_timeout_ms = 500;
    eCAL::CPublisher pub("blob", pub_config);

    int cnt(0);
    const auto max_runs(1000);
    while (eCAL::Ok())
    {
      if (do_start_publication && cnt < max_runs)
      {
        if (cnt == 0)
        {
          subscriber_seen_at_publication_start = pub.IsSubscribed();
        }

        pub.Send(std::to_string(cnt));
        cnt++;

        if (cnt == max_runs)
        {
          subscriber_seen_at_publication_end = pub.IsSubscribed();
          publication_finished = true;
          break;
        }
      }
    }
    };

  // subscribing thread
  auto subscriber_thread = [&]() {
    eCAL::CSubscriber sub("blob");
    bool received(false);
    auto max_lines(10);
    auto receive_lambda = [&received, &max_lines](const char* /*topic_name_*/, const struct eCAL::SReceiveCallbackData* data_)
      {
        if (max_lines)
        {
          // the final log should look like this
          // -----------------------------------
          //  Receiving 0
          //  Receiving 1
          //  Receiving 2
          //  Receiving 3
          //  Receiving 4
          //  Receiving 5
          //  Receiving 6
          //  Receiving 7
          //  Receiving 8
          //  Receiving 9
          // -----------------------------------
          std::cout << "Receiving " << std::string(static_cast<const char*>(data_->buf), data_->size) << std::endl;
          max_lines--;
        }
      };
    sub.AddReceiveCallback(receive_lambda);

    while (eCAL::Ok() && !publication_finished)
    {
      if (sub.IsPublished()) do_start_publication = true;
    }
    };

  // create threads for publisher and subscriber
  std::thread pub_thread(publisher_thread);
  std::thread sub_thread(subscriber_thread);

  // join threads to the main thread
  pub_thread.join();
  sub_thread.join();

  // finalize eCAL API
  eCAL::Finalize();

  // check if the publisher has seen the subscriber
  EXPECT_TRUE(subscriber_seen_at_publication_start);
  EXPECT_TRUE(subscriber_seen_at_publication_end);
}
