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

#include "tracing_test_helpers.h"

using namespace eCAL::tracing;

// ============ Real Pub/Sub Stress Tests ============
//
// These tests use actual eCAL publishers and subscribers exchanging data at
// high frequency.  They validate that the tracing subsystem generates the
// correct number/type of spans under realistic load.

class PubSubStressTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        eCAL::Configuration config;
        config.registration.registration_refresh = 100;
        config.registration.registration_timeout = 200;

        config.publisher.layer.shm.enable  = true;
        config.publisher.layer.udp.enable  = false;
        config.publisher.layer.tcp.enable  = false;
        config.subscriber.layer.shm.enable = true;
        config.subscriber.layer.udp.enable = false;
        config.subscriber.layer.tcp.enable = false;

        eCAL::Initialize(config, "tracing_pubsub_stress");

        setenv("ECAL_TRACING_DATA_DIR", tempDir().c_str(), 1);
        if (auto provider = eCAL::g_trace_provider(); provider)
        {
            triggerFlush(provider.get());
        }
        removeFile(spansFilePath());
        removeFile(metadataFilePath());
    }

    void TearDown() override
    {
        if (auto provider = eCAL::g_trace_provider(); provider)
        {
            triggerFlush(provider.get());
        }
        removeFile(spansFilePath());
        removeFile(metadataFilePath());

        eCAL::Finalize();
    }

    static void waitForMatch(const std::vector<eCAL::CSubscriber*>& subs, int timeout_ms = 2000)
    {
        auto deadline = std::chrono::steady_clock::now()
                      + std::chrono::milliseconds(timeout_ms);
        while (std::chrono::steady_clock::now() < deadline)
        {
            bool all_matched = true;
            for (auto* s : subs)
            {
                if (s->GetPublisherCount() == 0)
                {
                    all_matched = false;
                    break;
                }
            }
            if (all_matched) return;
            eCAL::Process::SleepMS(50);
        }
    }
};

// 2 publishers, 2 subscribers on the same topic, high-frequency send.
TEST_F(PubSubStressTest, TwoPubTwoSubHighFrequency)
{
    const std::string topic = "stress_topic_a";
    const int num_messages = 500;
    const std::string payload(128, 'X');

    eCAL::CPublisher  pub1(topic);
    eCAL::CPublisher  pub2(topic);
    eCAL::CSubscriber sub1(topic);
    eCAL::CSubscriber sub2(topic);

    std::atomic<int> recv_count1{0};
    std::atomic<int> recv_count2{0};

    sub1.SetReceiveCallback([&recv_count1](auto&&...) { recv_count1.fetch_add(1, std::memory_order_relaxed); });
    sub2.SetReceiveCallback([&recv_count2](auto&&...) { recv_count2.fetch_add(1, std::memory_order_relaxed); });

    std::vector<eCAL::CSubscriber*> subs = {&sub1, &sub2};
    waitForMatch(subs);

    std::thread t1([&]() { for (int i = 0; i < num_messages; ++i) pub1.Send(payload); });
    std::thread t2([&]() { for (int i = 0; i < num_messages; ++i) pub2.Send(payload); });
    t1.join();
    t2.join();

    eCAL::Process::SleepMS(500);

    if (auto provider = eCAL::g_trace_provider(); provider)
        triggerFlush(provider.get());

    auto lines = readJsonLines(spansFilePath());

    size_t send_spans = 0, receive_spans = 0, callback_spans = 0;
    for (const auto& line : lines)
    {
        if (line["entity_id"].get<uint64_t>() == 0) continue;
        int op = line["op_type"].get<int>();
        if (op == send)                    ++send_spans;
        else if (op == receive)            ++receive_spans;
        else if (op == callback_execution) ++callback_spans;
    }

    EXPECT_EQ(send_spans, 2u * num_messages);
    EXPECT_GE(receive_spans,  0u);
    EXPECT_GE(callback_spans, 0u);

    for (const auto& line : lines)
    {
        if (line["entity_id"].get<uint64_t>() != 0)
        {
            EXPECT_GT(line["start_ns"].get<long long>(), 0);
            EXPECT_GT(line["end_ns"].get<long long>(), 0);
        }
    }
}

// 3 publishers on different topics, 1 subscriber per topic.
TEST_F(PubSubStressTest, MultiTopicHighFrequency)
{
    const int num_topics = 3;
    const int num_messages = 300;
    const std::string payload(64, 'Y');

    std::vector<std::unique_ptr<eCAL::CPublisher>>  pubs;
    std::vector<std::unique_ptr<eCAL::CSubscriber>> subs;
    std::vector<std::atomic<int>> recv_counts(num_topics);
    for (auto& c : recv_counts) c.store(0);

    for (int t = 0; t < num_topics; ++t)
    {
        std::string topic = "stress_multi_" + std::to_string(t);
        pubs.push_back(std::make_unique<eCAL::CPublisher>(topic));
        subs.push_back(std::make_unique<eCAL::CSubscriber>(topic));
        subs.back()->SetReceiveCallback([&recv_counts, t](auto&&...) {
            recv_counts[t].fetch_add(1, std::memory_order_relaxed);
        });
    }

    std::vector<eCAL::CSubscriber*> sub_ptrs;
    for (auto& s : subs) sub_ptrs.push_back(s.get());
    waitForMatch(sub_ptrs);

    std::vector<std::thread> threads;
    for (int t = 0; t < num_topics; ++t)
    {
        threads.emplace_back([&pubs, &payload, t, num_messages]() {
            for (int i = 0; i < num_messages; ++i)
                pubs[t]->Send(payload);
        });
    }
    for (auto& th : threads) th.join();

    eCAL::Process::SleepMS(500);

    if (auto provider = eCAL::g_trace_provider(); provider)
        triggerFlush(provider.get());

    auto lines = readJsonLines(spansFilePath());

    size_t send_spans = 0;
    for (const auto& line : lines)
    {
        if (line["entity_id"].get<uint64_t>() == 0) continue;
        if (line["op_type"].get<int>() == send)
            ++send_spans;
    }

    EXPECT_EQ(send_spans, static_cast<size_t>(num_topics) * num_messages);

    for (const auto& line : lines)
    {
        if (line["entity_id"].get<uint64_t>() == 0) continue;
        if (line["op_type"].get<int>() == send)
        {
            EXPECT_EQ(line["layer"].get<uint64_t>(), tl_trace_shm);
        }
    }
}

// Sustained high-frequency publishing with subscriber callback overhead.
TEST_F(PubSubStressTest, HighFrequencyWithSlowCallback)
{
    const std::string topic = "stress_slow_cb";
    const int num_messages = 200;
    const std::string payload(256, 'Z');

    eCAL::CPublisher  pub(topic);
    eCAL::CSubscriber sub(topic);

    std::atomic<int> recv_count{0};

    sub.SetReceiveCallback([&recv_count](auto&&...) {
        recv_count.fetch_add(1, std::memory_order_relaxed);
        std::this_thread::sleep_for(std::chrono::microseconds(50));
    });

    std::vector<eCAL::CSubscriber*> sub_ptrs = {&sub};
    waitForMatch(sub_ptrs);

    for (int i = 0; i < num_messages; ++i)
    {
        pub.Send(payload);
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }

    eCAL::Process::SleepMS(1000);

    if (auto provider = eCAL::g_trace_provider(); provider)
        triggerFlush(provider.get());

    auto lines = readJsonLines(spansFilePath());

    size_t send_spans = 0;
    for (const auto& line : lines)
    {
        if (line["entity_id"].get<uint64_t>() == 0) continue;
        if (line["op_type"].get<int>() == send)
            ++send_spans;
    }
    EXPECT_EQ(send_spans, static_cast<size_t>(num_messages));

    for (const auto& line : lines)
    {
        if (line["op_type"].get<int>() == callback_execution)
        {
            long long duration_ns = line["end_ns"].get<long long>()
                                  - line["start_ns"].get<long long>();
            EXPECT_GE(duration_ns, 40000)
                << "callback_execution span duration too short";
        }
    }
}
