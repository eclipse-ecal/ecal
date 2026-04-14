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

// ============ Edge Case Tests ============

class EdgeCaseTest : public ::testing::Test
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

TEST_F(EdgeCaseTest, AllLayerTypesCombined)
{
    eCAL::STopicId topic_id;
    topic_id.topic_id.entity_id = 1;
    topic_id.topic_id.process_id = 2;

    eTracingLayerType all_layers = tl_trace_all;

    {
        CSpan span(topic_id, 100, all_layers, 256, send);
    }

    auto provider = eCAL::g_trace_provider();
    if (!provider) return;
    auto spans = provider->getSpans();
    ASSERT_EQ(spans.size(), 1);
    EXPECT_EQ(spans[0].layer, static_cast<uint64_t>(all_layers));
}

TEST_F(EdgeCaseTest, NoneLayerType)
{
    eCAL::STopicId topic_id;
    topic_id.topic_id.entity_id = 1;
    topic_id.topic_id.process_id = 2;

    {
        CSpan span(topic_id, 0, tl_trace_none, 0, send);
    }

    auto provider = eCAL::g_trace_provider();
    if (!provider) return;
    auto spans = provider->getSpans();
    ASSERT_EQ(spans.size(), 1);
    EXPECT_EQ(spans[0].layer, tl_trace_none);
}

TEST_F(EdgeCaseTest, EmptyStringMetadata)
{
    STopicMetadata metadata;
    metadata.host_name = "";
    metadata.topic_name = "";
    metadata.encoding = "";
    metadata.type_name = "";
    metadata.entity_id = 0;
    metadata.process_id = 0;
    metadata.direction = publisher;

    CTracingWriter writer;
    EXPECT_NO_THROW(writer.writeTopicMetadata(metadata));

    auto lines = readJsonLines(writer.getTopicMetadataFilePath());
    ASSERT_EQ(lines.size(), 1);
    EXPECT_EQ(lines[0]["host_name"].get<std::string>(), "");
    EXPECT_EQ(lines[0]["topic_name"].get<std::string>(), "");
}

TEST_F(EdgeCaseTest, ForceFlushSingleSpan)
{
    auto provider = eCAL::g_trace_provider();
    if (!provider) return;

    SSpanData span{};
    span.entity_id = 42;
    span.op_type = send;
    provider->bufferSpan(span);

    provider->forceFlush();
    EXPECT_EQ(provider->getSpans().size(), 0);
}

TEST_F(EdgeCaseTest, RapidForceFlushCycles)
{
    auto provider = eCAL::g_trace_provider();
    if (!provider) return;

    for (int cycle = 0; cycle < 10; ++cycle)
    {
        SSpanData span{};
        span.entity_id = cycle;
        span.op_type = send;
        provider->bufferSpan(span);
        provider->forceFlush();
        EXPECT_EQ(provider->getSpans().size(), 0);
    }
}

TEST_F(EdgeCaseTest, MaxEntityAndProcessIds)
{
    SSpanData span{};
    span.entity_id = UINT64_MAX;
    span.process_id = UINT64_MAX;
    span.topic_id = UINT64_MAX;
    span.op_type = send;

    CTracingWriter writer;
    writer.writeBatchSpans({span});

    auto lines = readJsonLines(writer.getSpansFilePath());
    ASSERT_EQ(lines.size(), 1);
    EXPECT_EQ(lines[0]["entity_id"].get<uint64_t>(), UINT64_MAX);
    EXPECT_EQ(lines[0]["process_id"].get<uint64_t>(), UINT64_MAX);
    EXPECT_EQ(lines[0]["topic_id"].get<uint64_t>(), UINT64_MAX);
}

TEST_F(EdgeCaseTest, ReceiveSpanViaProviderFlush)
{
    auto provider = eCAL::g_trace_provider();
    if (!provider) return;

    eCAL::Payload::TopicInfo topic_info;
    topic_info.topic_id = 555;
    topic_info.process_id = 666;

    {
        CSpan span(777, topic_info, 42, tl_trace_tcp, 2048, receive);
    }

    auto spans = provider->getSpans();
    ASSERT_EQ(spans.size(), 1);
    EXPECT_EQ(spans[0].entity_id, 777);
    EXPECT_EQ(spans[0].topic_id, 555);
    EXPECT_EQ(spans[0].process_id, 666);
    EXPECT_EQ(spans[0].op_type, receive);

    triggerFlush(provider.get());
    EXPECT_EQ(provider->getSpans().size(), 0);

    auto lines = readJsonLines(spansFilePath());
    ASSERT_GE(lines.size(), 1);
    bool found = false;
    for (const auto& line : lines)
    {
        if (line["entity_id"].get<uint64_t>() == 777)
        {
            EXPECT_EQ(line["topic_id"].get<uint64_t>(), 555);
            EXPECT_EQ(line["op_type"].get<int>(), receive);
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found);
}
