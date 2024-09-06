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

TEST(core_cpp_pubsub, TestSubscriberIsPublishedTiming)
{
  // initialize eCAL API
  EXPECT_EQ(0, eCAL::Initialize(0, nullptr, "subscriber_is_published"));

  // enable data loopback
  eCAL::Util::EnableLoopback(true);

  std::atomic<bool> do_start_publication(false);
  std::atomic<bool> publication_finished(false);

  // publishing thread
  std::atomic<bool> subscriber_seen_at_publication_start(false);
  auto publisher_thread = [&]() {
    eCAL::Publisher::Configuration pub_config;
    pub_config.layer.shm.acknowledge_timeout_ms = 500;
    eCAL::CPublisher pub("blob", pub_config);

    int pub_count(0);
    const auto max_pub_count(1000);
    while (eCAL::Ok())
    {
      if (do_start_publication && pub_count < max_pub_count)
      {
        if (pub_count == 0)
        {
          subscriber_seen_at_publication_start = pub.IsSubscribed();
        }

        pub.Send(std::to_string(pub_count));
        pub_count++;

        if (pub_count == max_pub_count)
        {
          publication_finished = true;
          break;
        }
      }
    }
  };

  // subscribing thread
  std::atomic<bool> publisher_seen_at_subscription_start(false);
  std::string first_received_sample;
  auto subscriber_thread = [&]() {
    eCAL::CSubscriber sub("blob");
    bool received(false);
    const auto max_sub_count(10);
    auto sub_count(0);
    auto receive_lambda = [&](const char* /*topic_name_*/, const struct eCAL::SReceiveCallbackData* data_)
      {
        if (sub_count == 0)
        {
          publisher_seen_at_subscription_start = sub.IsPublished();
          first_received_sample = std::string(static_cast<const char*>(data_->buf), data_->size);
        }

        if (sub_count < max_sub_count)
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
          sub_count++;
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

  // the first received sample should be "0"
  EXPECT_TRUE(first_received_sample == "0");

  // check if the publisher has seen the subscriber before publishing
  EXPECT_TRUE(subscriber_seen_at_publication_start);

  // check if the subscriber has seen the publisher on first receive
  EXPECT_TRUE(publisher_seen_at_subscription_start);

  // finalize eCAL API
  eCAL::Finalize();
}

TEST(core_cpp_pubsub, TestPublisherIsSubscribedTiming)
{
  // initialize eCAL API
  EXPECT_EQ(0, eCAL::Initialize(0, nullptr, "publisher_is_subscribed"));

  // enable data loopback
  eCAL::Util::EnableLoopback(true);

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
      if (pub.IsSubscribed())
      {
        do_start_publication = true;
      }

      if (do_start_publication && cnt < max_runs)
      {
        pub.Send(std::to_string(cnt));
        cnt++;

        if (cnt == max_runs)
        {
          publication_finished = true;
          break;
        }
      }
    }
  };

  // subscribing thread
  std::atomic<bool> publisher_seen_at_subscription_start(false);
  std::string first_received_sample;
  auto subscriber_thread = [&]() {
    eCAL::CSubscriber sub("blob");
    bool received(false);
    const auto max_sub_count(10);
    auto sub_count(0);
    auto receive_lambda = [&](const char* /*topic_name_*/, const struct eCAL::SReceiveCallbackData* data_)
      {
        if (sub_count == 0)
        {
          publisher_seen_at_subscription_start = sub.IsPublished();
          first_received_sample = std::string(static_cast<const char*>(data_->buf), data_->size);
        }

        if (sub_count < max_sub_count)
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
          sub_count++;
        }
      };
    sub.AddReceiveCallback(receive_lambda);

    while (eCAL::Ok() && !publication_finished)
    {
    }
  };

  // create threads for publisher and subscriber
  std::thread pub_thread(publisher_thread);
  std::thread sub_thread(subscriber_thread);

  // join threads to the main thread
  pub_thread.join();
  sub_thread.join();

  // the first received sample should be "0"
  EXPECT_TRUE(first_received_sample == "0");

  // check if the subscriber has seen the publisher on first receive
  EXPECT_TRUE(publisher_seen_at_subscription_start);

  // finalize eCAL API
  eCAL::Finalize();
}
