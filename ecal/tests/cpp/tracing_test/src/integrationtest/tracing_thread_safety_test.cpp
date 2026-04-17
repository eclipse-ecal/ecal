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

// ============ Thread Safety Tests ============

class ThreadSafetyTest : public ::testing::Test
{
protected:
    std::unique_ptr<CTracingWriter> writer_;

    void SetUp() override
    {
        setenv("ECAL_TRACING_DATA_DIR", tempDir().c_str(), 1);
        if (auto provider = eCAL::g_trace_provider(); provider)
        {
            triggerFlush(provider.get());
        }
        writer_ = std::make_unique<CTracingWriter>();
        removeFile(spansFilePath());
        removeFile(writer_->getSpansFilePath());
        removeFile(writer_->getTopicMetadataFilePath());
    }

    void TearDown() override
    {
        if (auto provider = eCAL::g_trace_provider(); provider)
        {
            triggerFlush(provider.get());
        }
        removeFile(spansFilePath());
        removeFile(writer_->getSpansFilePath());
        removeFile(writer_->getTopicMetadataFilePath());
    }
};

TEST_F(ThreadSafetyTest, ConcurrentSpanBuffering)
{
    auto provider = eCAL::g_trace_provider();
    if (!provider) return;

    const int num_threads = 4;
    const int spans_per_thread = 25;
    std::vector<std::thread> threads;

    for (int t = 0; t < num_threads; ++t)
    {
        threads.emplace_back([&provider, t, spans_per_thread]()
        {
            eCAL::STopicId topic_id;
            topic_id.topic_id.entity_id = t;
            topic_id.topic_id.process_id = eCAL::Process::GetProcessID();

            for (int i = 0; i < spans_per_thread; ++i)
            {
                CSpan span(topic_id, i, tl_trace_shm, 256, send);
            }
        });
    }

    for (auto& t : threads) t.join();

    auto spans = provider->getSpans();
    EXPECT_EQ(spans.size(), num_threads * spans_per_thread);
}

TEST_F(ThreadSafetyTest, ConcurrentMetadataWriting)
{
    const int num_threads = 4;
    const int metadata_per_thread = 5;
    std::vector<std::thread> threads;

    for (int t = 0; t < num_threads; ++t)
    {
        threads.emplace_back([this, t, metadata_per_thread]()
        {
            for (int i = 0; i < metadata_per_thread; ++i)
            {
                STopicMetadata metadata;
                metadata.entity_id = t * 100 + i;
                metadata.process_id = eCAL::Process::GetProcessID();
                metadata.host_name = "host-" + std::to_string(t);
                metadata.topic_name = "topic_" + std::to_string(t) + "_" + std::to_string(i);
                metadata.encoding = "protobuf";
                metadata.type_name = "TestMessage";
                metadata.direction = publisher;

                EXPECT_NO_THROW(writer_->writeTopicMetadata(metadata));
            }
        });
    }

    for (auto& t : threads) t.join();

    auto lines = readJsonLines(writer_->getTopicMetadataFilePath());
    EXPECT_EQ(lines.size(), num_threads * metadata_per_thread);
}

TEST_F(ThreadSafetyTest, ConcurrentBufferingWithForceFlush)
{
    auto provider = eCAL::g_trace_provider();
    if (!provider) return;

    const int num_threads = 4;
    const int spans_per_thread = 20;
    std::vector<std::thread> threads;

    for (int t = 0; t < num_threads; ++t)
    {
        threads.emplace_back([&provider, t, spans_per_thread]()
        {
            for (int i = 0; i < spans_per_thread; ++i)
            {
                SSpanData span{};
                span.entity_id = t * 1000 + i;
                span.process_id = eCAL::Process::GetProcessID();
                span.op_type = send;
                provider->bufferSpan(span);
            }
        });
    }

    for (auto& t : threads) t.join();

    triggerFlush(provider.get());

    EXPECT_EQ(provider->getSpans().size(), 0);

    auto lines = readJsonLines(spansFilePath());
    EXPECT_GE(lines.size(), static_cast<size_t>(num_threads * spans_per_thread));
}

TEST_F(ThreadSafetyTest, ConcurrentGetSpans)
{
    auto provider = eCAL::g_trace_provider();
    if (!provider) return;

    for (int i = 0; i < 50; ++i)
    {
        SSpanData span{};
        span.entity_id = i;
        span.op_type = send;
        provider->bufferSpan(span);
    }

    const int num_readers = 3;
    std::vector<std::thread> threads;

    for (int t = 0; t < num_readers; ++t)
    {
        threads.emplace_back([&provider]()
        {
            for (int i = 0; i < 100; ++i)
            {
                auto spans = provider->getSpans();
                EXPECT_GE(spans.size(), 0u);
            }
        });
    }

    threads.emplace_back([&provider]()
    {
        for (int i = 0; i < 50; ++i)
        {
            SSpanData span{};
            span.entity_id = 1000 + i;
            span.op_type = send;
            provider->bufferSpan(span);
        }
    });

    for (auto& t : threads) t.join();

    auto spans = provider->getSpans();
    EXPECT_EQ(spans.size(), 100u);
}

TEST_F(ThreadSafetyTest, ConcurrentWriteBatchSpansIntegrity)
{
    // Regression: writeBatchSpans previously had no mutex, so concurrent
    // flushes interleaved writes producing corrupted JSONL lines.
    const int num_threads = 8;
    const int batches_per_thread = 20;
    const int spans_per_batch = 5;
    std::vector<std::thread> threads;

    for (int t = 0; t < num_threads; ++t)
    {
        threads.emplace_back([this, t, batches_per_thread, spans_per_batch]()
        {
            for (int b = 0; b < batches_per_thread; ++b)
            {
                std::vector<SSpanData> batch;
                for (int s = 0; s < spans_per_batch; ++s)
                {
                    SSpanData span{};
                    span.entity_id = static_cast<uint64_t>(t);
                    span.clock = b * spans_per_batch + s;
                    span.op_type = send;
                    batch.push_back(span);
                }
                writer_->writeBatchSpans(batch);
            }
        });
    }

    for (auto& t : threads) t.join();

    auto lines = readJsonLines(writer_->getSpansFilePath());
    EXPECT_EQ(lines.size(),
              static_cast<size_t>(num_threads) * batches_per_thread * spans_per_batch);
}
