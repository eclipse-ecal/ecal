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

#include <ecal/ecal.h>
#include <ecal/pubsub/publisher.h>
#include <ecal/pubsub/subscriber.h>

#include <atomic>
#include <chrono>
#include <string>
#include <thread>

#include <gtest/gtest.h>

TEST(core_cpp_pubsub, TestSubscriberIsPublishedTiming)
{
  // initialize eCAL API
  EXPECT_EQ(true, eCAL::Initialize("subscriber_is_published"));

  std::atomic<bool> do_start_publication(false);
  std::atomic<bool> publication_finished(false);

  // publishing thread
  std::atomic<bool> subscriber_seen_at_publication_start(false);
  auto publisher_function = [&do_start_publication, &publication_finished, &subscriber_seen_at_publication_start]() {
    eCAL::Publisher::Configuration pub_config;
    pub_config.layer.shm.acknowledge_timeout_ms = 500;
    eCAL::CPublisher pub("blob", eCAL::SDataTypeInformation(), pub_config);

    int pub_count(0);
    const auto max_pub_count(1000);
    while (eCAL::Ok())
    {
      if (do_start_publication && pub_count < max_pub_count)
      {
        if (pub_count == 0)
        {
          subscriber_seen_at_publication_start = pub.GetSubscriberCount() > 0;
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
  auto subscriber_function = [&do_start_publication, &publication_finished, &publisher_seen_at_subscription_start, &first_received_sample]() {
    eCAL::CSubscriber sub("blob");
    const auto max_sub_count(10);
    auto sub_count(0);
    auto receive_lambda = [&max_sub_count, &sub_count, &publisher_seen_at_subscription_start, &first_received_sample, &sub](const eCAL::STopicId& /*topic_id_*/, const eCAL::SDataTypeInformation& /*data_type_info_*/, const eCAL::SReceiveCallbackData& data_) {
      if (sub_count == 0)
      {
        publisher_seen_at_subscription_start = sub.GetPublisherCount() > 0;
        first_received_sample = std::string(static_cast<const char*>(data_.buffer), data_.buffer_size);
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
          //std::cout << "Receiving " << std::string(static_cast<const char*>(data_->buf), data_->size) << std::endl;
          sub_count++;
        }
      };
    sub.SetReceiveCallback(receive_lambda);

    while (eCAL::Ok() && !publication_finished)
    {
      if (sub.GetPublisherCount() > 0) do_start_publication = true;
    }
  };

  // create threads for publisher and subscriber
  std::thread pub_thread(publisher_function);
  std::thread sub_thread(subscriber_function);

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
  EXPECT_EQ(true, eCAL::Initialize("publisher_is_subscribed"));

  std::atomic<bool> do_start_publication(false);
  std::atomic<bool> publication_finished(false);

  // publishing thread
  auto publisher_function = [&do_start_publication, &publication_finished]() {
    eCAL::Publisher::Configuration pub_config;
    pub_config.layer.shm.acknowledge_timeout_ms = 500;
    eCAL::CPublisher pub("blob", eCAL::SDataTypeInformation(), pub_config);

    int cnt(0);
    const auto max_runs(1000);
    while (eCAL::Ok())
    {
      if (pub.GetSubscriberCount() > 0)
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
  auto subscriber_function = [&publication_finished, &publisher_seen_at_subscription_start, &first_received_sample]() {
    eCAL::CSubscriber sub("blob");
    const auto max_sub_count(10);
    auto sub_count(0);
    auto receive_lambda = [&max_sub_count, &sub_count, &publisher_seen_at_subscription_start, &first_received_sample, &sub](const eCAL::STopicId& /*topic_id_*/, const eCAL::SDataTypeInformation& /*data_type_info_*/, const eCAL::SReceiveCallbackData& data_) {
      if (sub_count == 0)
      {
        publisher_seen_at_subscription_start = sub.GetPublisherCount() > 0;
        first_received_sample = std::string(static_cast<const char*>(data_.buffer), data_.buffer_size);
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
          //std::cout << "Receiving " << std::string(static_cast<const char*>(data_->buf), data_->size) << std::endl;
          sub_count++;
        }
      };
    sub.SetReceiveCallback(receive_lambda);

    while (eCAL::Ok() && !publication_finished)
    {
    }
  };

  // create threads for publisher and subscriber
  std::thread pub_thread(publisher_function);
  std::thread sub_thread(subscriber_function);

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

TEST(core_cpp_pubsub, TestChainedPublisherSubscriberCallback)
{
  // initialize eCAL API
  EXPECT_EQ(true, eCAL::Initialize("chained_publisher_subscriber"));

  // Set up counters for sent and received messages
  const int message_count = 10;
  std::atomic<int> publisher1_sent_count(0);
  std::atomic<int> subscriber2_received_count(0);

  // Publisher1 in thread 1
  auto publisher1_function = [&publisher1_sent_count, &message_count]() {
    eCAL::CPublisher pub1("topic1");
    while (pub1.GetSubscriberCount() == 0)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    for (int i = 0; i < message_count; ++i)
    {
      pub1.Send(std::to_string(i));
      publisher1_sent_count++;
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    };

  // Publisher2
  eCAL::CPublisher pub2("topic2");

  // Subscriber1 with callback that triggers Publisher2
  eCAL::CSubscriber sub1("topic1");
  auto subscriber1_callback = [&pub2](const eCAL::STopicId& /*topic_id_*/, const eCAL::SDataTypeInformation& /*data_type_info_*/, const eCAL::SReceiveCallbackData& data_) {
    // On receiving data from Publisher1, Publisher2 sends the same data
    const std::string received_data(static_cast<const char*>(data_.buffer), data_.buffer_size);
    pub2.Send(received_data);
    };
  sub1.SetReceiveCallback(subscriber1_callback);

  // Subscriber2 that receives data from Publisher2
  eCAL::CSubscriber sub2("topic2");
  auto subscriber2_callback = [&subscriber2_received_count](const eCAL::STopicId& /*topic_id_*/, const eCAL::SDataTypeInformation& /*data_type_info_*/, const eCAL::SReceiveCallbackData& /*data_*/) {
    // Count each received message from Publisher2
    subscriber2_received_count++;
    //std::cout << "Subscriber2 Receiving " << std::string(static_cast<const char*>(data_.buf), data_.size) << std::endl;
    };
  sub2.SetReceiveCallback(subscriber2_callback);

  // Start publisher1 thread
  std::thread pub1_thread(publisher1_function);

  // Wait until Publisher1 has sent all messages and Subscriber2 has received them
  pub1_thread.join();
  int max_message_loops(100);
  while ((subscriber2_received_count < message_count) && (max_message_loops-- > 0))
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  // Validate that Subscriber2 received all messages sent by Publisher1
  EXPECT_EQ(publisher1_sent_count, subscriber2_received_count);

  // finalize eCAL API
  eCAL::Finalize();
}
