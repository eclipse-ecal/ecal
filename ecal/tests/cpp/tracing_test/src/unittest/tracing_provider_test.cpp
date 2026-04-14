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

// ============ Tests for CTraceProvider Singleton and Buffering ============

class TracingProviderTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        setenv("ECAL_TRACING_DATA_DIR", tempDir().c_str(), 1);
        if (auto provider = eCAL::g_trace_provider(); provider)
        {
            triggerFlush(provider.get());
        }
    }

    void TearDown() override
    {
        if (auto provider = eCAL::g_trace_provider(); provider)
        {
            triggerFlush(provider.get());
        }
    }
};

TEST_F(TracingProviderTest, SingletonInstance)
{
    auto provider = eCAL::g_trace_provider();
    if (!provider) return;

    CTraceProvider& provider1 = *provider;
    CTraceProvider& provider2 = *provider;

    EXPECT_EQ(&provider1, &provider2);
}

TEST_F(TracingProviderTest, DefaultBatchSize)
{
    auto provider = eCAL::g_trace_provider();
    if (!provider) return;

    for (size_t i = 0; i < kDefaultTracingBatchSize - 1; ++i)
    {
        SSpanData span{};
        span.entity_id = i;
        provider->bufferSpan(span);
    }
    EXPECT_EQ(provider->getSpans().size(), kDefaultTracingBatchSize - 1);
}

TEST_F(TracingProviderTest, BufferSpan)
{
    auto provider = eCAL::g_trace_provider();
    if (!provider) return;

    SSpanData span{};
    span.entity_id = 111;
    span.process_id = 222;
    span.payload_size = 512;

    provider->bufferSpan(span);

    auto spans = provider->getSpans();
    ASSERT_EQ(spans.size(), 1);
    EXPECT_EQ(spans[0].entity_id, 111);
    EXPECT_EQ(spans[0].process_id, 222);
    EXPECT_EQ(spans[0].payload_size, 512);
}

TEST_F(TracingProviderTest, BufferMultipleSpans)
{
    auto provider = eCAL::g_trace_provider();
    if (!provider) return;

    for (int i = 0; i < 5; ++i)
    {
        SSpanData span{};
        span.entity_id = i;
        provider->bufferSpan(span);
    }

    auto spans = provider->getSpans();
    ASSERT_EQ(spans.size(), 5);

    for (int i = 0; i < 5; ++i)
    {
        EXPECT_EQ(spans[i].entity_id, static_cast<uint64_t>(i));
    }
}

TEST_F(TracingProviderTest, ForceFlushDrainsBuffer)
{
    auto provider = eCAL::g_trace_provider();
    if (!provider) return;

    SSpanData span{};
    span.entity_id = 999;
    provider->bufferSpan(span);

    provider->forceFlush();
    EXPECT_EQ(provider->getSpans().size(), 0);
}

TEST_F(TracingProviderTest, EmptyBufferStaysEmpty)
{
    auto provider = eCAL::g_trace_provider();
    if (!provider) return;

    EXPECT_EQ(provider->getSpans().size(), 0);
}

TEST_F(TracingProviderTest, BufferFlushedOnForceFlush)
{
    auto provider = eCAL::g_trace_provider();
    if (!provider) return;

    for (int i = 0; i < 3; ++i)
    {
        SSpanData span{};
        span.entity_id = i;
        provider->bufferSpan(span);
    }

    EXPECT_EQ(provider->getSpans().size(), 3);

    provider->forceFlush();
    EXPECT_EQ(provider->getSpans().size(), 0);
}

TEST_F(TracingProviderTest, MultipleForceFlushCycles)
{
    auto provider = eCAL::g_trace_provider();
    if (!provider) return;

    for (int i = 0; i < 2; ++i)
    {
        SSpanData span{};
        span.entity_id = i;
        provider->bufferSpan(span);
    }
    provider->forceFlush();
    EXPECT_EQ(provider->getSpans().size(), 0);

    for (int i = 10; i < 12; ++i)
    {
        SSpanData span{};
        span.entity_id = i;
        provider->bufferSpan(span);
    }
    provider->forceFlush();
    EXPECT_EQ(provider->getSpans().size(), 0);

    SSpanData span{};
    span.entity_id = 99;
    provider->bufferSpan(span);
    EXPECT_EQ(provider->getSpans().size(), 1);
}

TEST_F(TracingProviderTest, AddTopicMetadata)
{
    removeFile(metadataFilePath());

    auto provider = eCAL::g_trace_provider();
    if (!provider) return;

    STopicMetadata metadata;
    metadata.entity_id = 777;
    metadata.process_id = 888;
    metadata.host_name = "provider-host";
    metadata.topic_name = "provider_topic";
    metadata.encoding = "protobuf";
    metadata.type_name = "ProviderMsg";
    metadata.direction = publisher;

    EXPECT_NO_THROW(provider->addTopicMetadata(metadata));

    auto lines = readJsonLines(metadataFilePath());
    ASSERT_GE(lines.size(), 1);
    const auto& obj = lines.back();
    EXPECT_EQ(obj["entity_id"].get<uint64_t>(), 777);
    EXPECT_EQ(obj["process_id"].get<int32_t>(), 888);
    EXPECT_EQ(obj["host_name"].get<std::string>(), "provider-host");
    EXPECT_EQ(obj["topic_name"].get<std::string>(), "provider_topic");
    EXPECT_EQ(obj["encoding"].get<std::string>(), "protobuf");
    EXPECT_EQ(obj["type_name"].get<std::string>(), "ProviderMsg");
    EXPECT_EQ(obj["direction"].get<std::string>(), "publisher");
    EXPECT_EQ(obj["tracing_version"].get<std::string>(), kTracingVersion);

    removeFile(metadataFilePath());
}

TEST_F(TracingProviderTest, AddTopicMetadataSubscriber)
{
    removeFile(metadataFilePath());

    auto provider = eCAL::g_trace_provider();
    if (!provider) return;

    STopicMetadata metadata;
    metadata.entity_id = 100;
    metadata.process_id = 200;
    metadata.host_name = "sub-host";
    metadata.topic_name = "sub_topic";
    metadata.encoding = "flatbuffers";
    metadata.type_name = "SubMsg";
    metadata.direction = subscriber;

    provider->addTopicMetadata(metadata);

    auto lines = readJsonLines(metadataFilePath());
    ASSERT_GE(lines.size(), 1);
    EXPECT_EQ(lines.back()["direction"].get<std::string>(), "subscriber");

    removeFile(metadataFilePath());
}
