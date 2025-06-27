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

#include <gtest/gtest.h>
#include <ecal/ecal.h>
#include <ecal/pubsub/publisher.h>
#include <ecal/pubsub/subscriber.h>
#include <ecal/pubsub/types.h>

#include <vector>
#include <unordered_set>
#include <thread>
#include <chrono>
#include <algorithm>

using namespace eCAL;
using namespace std::chrono_literals;

// store just the topic ID and event type
struct PubEvent {
  STopicId          id;
  ePublisherEvent   type;
};

struct SubEvent {
  STopicId           id;
  eSubscriberEvent   type;
};

class PubSubEventTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    eCAL::Initialize("PubSubEventTest");
  }

  void TearDown() override
  {
    eCAL::Finalize();
  }

  void wait_disc() const
  {
    std::this_thread::sleep_for(2s);
  }
};

TEST_F(PubSubEventTest, PublisherCallback_TopicIdsMatchSubscribers)
{
  std::vector<PubEvent> events;
  std::unordered_set<STopicId> expected_ids;

  CPublisher pub(
    "MyTopic",
    SDataTypeInformation(),
    [&events](const STopicId& tid, const SPubEventCallbackData& d)
    {
      events.push_back({ tid, d.event_type });
    });
  wait_disc();

  {
    CSubscriber sub1("MyTopic");  expected_ids.insert(sub1.GetTopicId());
    CSubscriber sub2("MyTopic");  expected_ids.insert(sub2.GetTopicId());
    wait_disc();
    CSubscriber sub3("MyTopic");  expected_ids.insert(sub3.GetTopicId());
    wait_disc();
  }
  wait_disc();

  std::unordered_set<STopicId> conn_ids, disc_ids;
  for (auto& e : events)
  {
    if (e.type == ePublisherEvent::connected)    conn_ids.insert(e.id);
    if (e.type == ePublisherEvent::disconnected) disc_ids.insert(e.id);
  }

  EXPECT_EQ(conn_ids, expected_ids);
  EXPECT_EQ(disc_ids, expected_ids);
}

TEST_F(PubSubEventTest, SubscriberCallback_TopicIdsMatchPublishers)
{
  std::vector<SubEvent> events;
  std::unordered_set<STopicId> expected_ids;

  CSubscriber sub(
    "MyTopic",
    SDataTypeInformation(),
    [&events](const STopicId& tid, const SSubEventCallbackData& d)
    {
      events.push_back({ tid, d.event_type });
    });
  wait_disc();

  {
    CPublisher pub1("MyTopic");  expected_ids.insert(pub1.GetTopicId());
    CPublisher pub2("MyTopic");  expected_ids.insert(pub2.GetTopicId());
    wait_disc();
    CPublisher pub3("MyTopic");  expected_ids.insert(pub3.GetTopicId());
    wait_disc();
  }
  wait_disc();

  std::unordered_set<STopicId> conn_ids, disc_ids;
  for (auto& e : events)
  {
    if (e.type == eSubscriberEvent::connected)    conn_ids.insert(e.id);
    if (e.type == eSubscriberEvent::disconnected) disc_ids.insert(e.id);
  }

  EXPECT_EQ(conn_ids, expected_ids);
  EXPECT_EQ(disc_ids, expected_ids);
}
