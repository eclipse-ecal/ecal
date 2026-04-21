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

#include <tracing/trace_provider.h>
#include <tracing/tracing.h>

#include <gtest/gtest.h>

#include <ecal_utils/barrier.h>

#include <atomic>
#include <thread>
#include <vector>

TEST(TestTraceProvider, ConcurrentSpanWrites)
{
    constexpr size_t num_threads      = 100;
    constexpr size_t spans_per_thread = 100;
    constexpr size_t total_spans      = num_threads * spans_per_thread;  // 10000
    constexpr size_t batch_size       = 500;  // Flush every 500 spans

    MockTracingWriter mock_writer;

    auto provider = eCAL::tracing::CTraceProvider::Create(
        std::make_unique<ProxyTracingWriter>(mock_writer), batch_size);
    ASSERT_NE(provider, nullptr);

    Barrier barrier(num_threads);

    std::vector<std::thread> threads;
    threads.reserve(num_threads);

    for (size_t t = 0; t < num_threads; ++t)
    {
        threads.emplace_back([&, t]()
        {
            barrier.wait();
            for (size_t i = 0; i < spans_per_thread; ++i)
            {
                eCAL::tracing::SPublisherSpanData span{};
                span.op_type      = eCAL::tracing::operation_type::send;
                span.entity_id    = static_cast<uint64_t>(t * spans_per_thread + i);
                span.process_id   = 1;
                span.payload_size = 42;
                span.clock        = static_cast<long long>(i);
                span.layer        = eCAL::tracing::tl_trace_shm;
                span.start_ns     = 1000 + static_cast<long long>(i);
                span.end_ns       = 2000 + static_cast<long long>(i);
                provider->WriteSpan(span);
            }
        });
    }

    // Join all writer threads.
    for (auto& th : threads)
        th.join();

    // Destroy the provider to flush any remaining buffered spans.
    provider.reset();

    // Every span must be written exactly once — no duplicates, no drops.
    EXPECT_EQ(mock_writer.SpanCount(), total_spans);
}

TEST(TestTraceProvider, ConcurrentMetadataWrites)
{
    constexpr size_t num_threads          = 100;
    constexpr size_t metadata_per_thread  = 100;
    constexpr size_t total_metadata       = num_threads * metadata_per_thread;  // 10000
    constexpr size_t batch_size           = 500;

    MockTracingWriter mock_writer;

    auto provider = eCAL::tracing::CTraceProvider::Create(
        std::make_unique<ProxyTracingWriter>(mock_writer), batch_size);
    ASSERT_NE(provider, nullptr);

    Barrier barrier(num_threads);

    std::vector<std::thread> threads;
    threads.reserve(num_threads);

    for (size_t t = 0; t < num_threads; ++t)
    {
        threads.emplace_back([&, t]()
        {
            barrier.wait();
            for (size_t i = 0; i < metadata_per_thread; ++i)
            {
                eCAL::tracing::STopicMetadata metadata{};
                metadata.entity_id  = static_cast<uint64_t>(t * metadata_per_thread + i);
                metadata.process_id = 1;
                metadata.host_name  = "test_host";
                metadata.topic_name = "topic_" + std::to_string(t) + "_" + std::to_string(i);
                metadata.encoding   = "protobuf";
                metadata.type_name  = "TestType";
                metadata.direction  = eCAL::tracing::topic_direction::publisher;
                provider->WriteMetadata(metadata);
            }
        });
    }

    for (auto& th : threads)
        th.join();

    provider.reset();

    EXPECT_EQ(mock_writer.MetadataCount(), total_metadata);
}