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

// ============ Scale / Stress Tests ============

class ScaleTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
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
    }
};

// Simulate hundreds of publishers/subscribers buffering spans concurrently.
// Validates that no spans are lost under high contention.
TEST_F(ScaleTest, HighFanoutProducers)
{
    auto provider = eCAL::g_trace_provider();
    if (!provider) return;

    const int num_producers = 500;
    const int spans_per_producer = 200;
    std::vector<std::thread> threads;

    for (int p = 0; p < num_producers; ++p)
    {
        threads.emplace_back([&provider, p, spans_per_producer]()
        {
            for (int i = 0; i < spans_per_producer; ++i)
            {
                SSpanData span{};
                span.entity_id = static_cast<uint64_t>(p);
                span.clock = i;
                span.op_type = (p % 2 == 0) ? send : receive;
                provider->bufferSpan(span);
            }
        });
    }

    for (auto& t : threads) t.join();

    triggerFlush(provider.get());
    EXPECT_EQ(provider->getSpans().size(), 0);

    auto lines = readJsonLines(spansFilePath());
    EXPECT_GE(lines.size(), static_cast<size_t>(num_producers) * spans_per_producer);
}

// Same high fan-out but using CSpan RAII objects (the real API path).
TEST_F(ScaleTest, HighFanoutViaSpanRAII)
{
    auto provider = eCAL::g_trace_provider();
    if (!provider) return;

    const int num_publishers = 200;
    const int messages_per_pub = 100;
    std::vector<std::thread> threads;

    for (int p = 0; p < num_publishers; ++p)
    {
        threads.emplace_back([p, messages_per_pub]()
        {
            eCAL::STopicId topic_id;
            topic_id.topic_id.entity_id = static_cast<uint64_t>(p);
            topic_id.topic_id.process_id = static_cast<uint64_t>(eCAL::Process::GetProcessID());

            for (int i = 0; i < messages_per_pub; ++i)
            {
                CSpan span(topic_id, i, tl_trace_shm, 256, send);
            }
        });
    }

    for (auto& t : threads) t.join();

    triggerFlush(provider.get());

    auto lines = readJsonLines(spansFilePath());
    EXPECT_GE(lines.size(), static_cast<size_t>(num_publishers) * messages_per_pub);
}

// Simulate mixed publisher and subscriber registration at scale.
TEST_F(ScaleTest, MixedPubSubMetadataAndSpans)
{
    auto provider = eCAL::g_trace_provider();
    if (!provider) return;

    const int num_endpoints = 200;
    const int spans_per_endpoint = 50;
    std::vector<std::thread> threads;

    for (int e = 0; e < num_endpoints; ++e)
    {
        threads.emplace_back([&provider, e, spans_per_endpoint]()
        {
            STopicMetadata metadata;
            metadata.entity_id = static_cast<uint64_t>(e);
            metadata.process_id = eCAL::Process::GetProcessID();
            metadata.host_name = "scale-host";
            metadata.topic_name = "topic_" + std::to_string(e);
            metadata.encoding = "protobuf";
            metadata.type_name = "ScaleMsg";
            metadata.direction = (e % 2 == 0) ? publisher : subscriber;
            provider->addTopicMetadata(metadata);

            for (int i = 0; i < spans_per_endpoint; ++i)
            {
                SSpanData span{};
                span.entity_id = static_cast<uint64_t>(e);
                span.clock = i;
                span.op_type = (e % 2 == 0) ? send : receive;
                provider->bufferSpan(span);
            }
        });
    }

    for (auto& t : threads) t.join();

    triggerFlush(provider.get());

    auto meta_lines = readJsonLines(metadataFilePath());
    EXPECT_EQ(meta_lines.size(), static_cast<size_t>(num_endpoints));

    auto span_lines = readJsonLines(spansFilePath());
    EXPECT_GE(span_lines.size(), static_cast<size_t>(num_endpoints) * spans_per_endpoint);
}

// Vary batch sizes under constant load to verify correctness is independent of batch tuning.
TEST_F(ScaleTest, HighThroughputNoDataLoss)
{
    auto provider = eCAL::g_trace_provider();
    if (!provider) return;

    const int num_producers = 50;
    const int spans_per_producer = 100;
    const size_t expected_total = static_cast<size_t>(num_producers) * spans_per_producer;

    removeFile(spansFilePath());

    std::vector<std::thread> threads;
    for (int p = 0; p < num_producers; ++p)
    {
        threads.emplace_back([&provider, p, spans_per_producer]()
        {
            for (int i = 0; i < spans_per_producer; ++i)
            {
                SSpanData span{};
                span.entity_id = static_cast<uint64_t>(p);
                span.clock = i;
                span.op_type = send;
                provider->bufferSpan(span);
            }
        });
    }

    for (auto& t : threads) t.join();

    triggerFlush(provider.get());

    auto lines = readJsonLines(spansFilePath());
    EXPECT_GE(lines.size(), expected_total);
}

// Sustained burst: rapidly produce large volumes to stress the mutex + file I/O path.
TEST_F(ScaleTest, SustainedBurst)
{
    auto provider = eCAL::g_trace_provider();
    if (!provider) return;

    const int num_threads = 100;
    const int spans_per_thread = 500;
    std::vector<std::thread> threads;

    for (int t = 0; t < num_threads; ++t)
    {
        threads.emplace_back([&provider, t, spans_per_thread]()
        {
            for (int i = 0; i < spans_per_thread; ++i)
            {
                SSpanData span{};
                span.entity_id = static_cast<uint64_t>(t);
                span.topic_id = static_cast<uint64_t>(t % 50);
                span.process_id = static_cast<uint64_t>(eCAL::Process::GetProcessID());
                span.clock = i;
                span.layer = tl_trace_shm;
                span.op_type = send;
                provider->bufferSpan(span);
            }
        });
    }

    for (auto& t : threads) t.join();

    triggerFlush(provider.get());

    auto lines = readJsonLines(spansFilePath());
    EXPECT_GE(lines.size(), static_cast<size_t>(num_threads) * spans_per_thread);
}
