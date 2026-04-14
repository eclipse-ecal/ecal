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

// ============ Integration Tests ============

class TracingIntegrationTest : public ::testing::Test
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

TEST_F(TracingIntegrationTest, SpanBufferingAndFlushing)
{
    auto provider = eCAL::g_trace_provider();
    if (!provider) return;

    eCAL::STopicId topic_id;
    topic_id.topic_id.entity_id = 1;
    topic_id.topic_id.process_id = 2;

    for (int i = 0; i < 3; ++i)
    {
        CSpan span(topic_id, i, tl_trace_shm, 256 * (i + 1), send);
    }

    auto spans = provider->getSpans();
    ASSERT_EQ(spans.size(), 3);

    for (int i = 0; i < 3; ++i)
    {
        EXPECT_EQ(spans[i].clock, i);
        EXPECT_EQ(spans[i].payload_size, 256 * (i + 1));
    }

    triggerFlush(provider.get());
    EXPECT_EQ(provider->getSpans().size(), 0);
}

TEST_F(TracingIntegrationTest, MixedSendAndReceiveSpans)
{
    auto provider = eCAL::g_trace_provider();
    if (!provider) return;

    eCAL::STopicId send_topic_id;
    send_topic_id.topic_id.entity_id = 1;
    send_topic_id.topic_id.process_id = 2;

    eCAL::Payload::TopicInfo recv_topic_info;
    recv_topic_info.topic_id = 10;
    recv_topic_info.process_id = 20;

    {
        CSpan send_span(send_topic_id, 100, tl_trace_shm, 256, send);
    }
    {
        CSpan recv_span(30, recv_topic_info, 200, tl_trace_udp, 512, receive);
    }

    auto spans = provider->getSpans();
    ASSERT_EQ(spans.size(), 2);

    EXPECT_EQ(spans[0].op_type, send);
    EXPECT_EQ(spans[0].topic_id, 0);
    EXPECT_EQ(spans[0].entity_id, 1);
    EXPECT_EQ(spans[0].process_id, 2);

    EXPECT_EQ(spans[1].op_type, receive);
    EXPECT_EQ(spans[1].topic_id, 10);
    EXPECT_EQ(spans[1].entity_id, 30);
    EXPECT_EQ(spans[1].process_id, 20);
}

TEST_F(TracingIntegrationTest, CompleteTracingFlow)
{
    auto provider = eCAL::g_trace_provider();
    if (!provider) return;

    STopicMetadata metadata;
    metadata.entity_id = 111;
    metadata.process_id = eCAL::Process::GetProcessID();
    metadata.host_name = "integration-host";
    metadata.topic_name = "integration_test_topic";
    metadata.encoding = "protobuf";
    metadata.type_name = "IntegrationData";
    metadata.direction = publisher;

    provider->addTopicMetadata(metadata);

    eCAL::STopicId topic_id;
    topic_id.topic_id.entity_id = 111;
    topic_id.topic_id.process_id = eCAL::Process::GetProcessID();

    for (int i = 0; i < 5; ++i)
    {
        CSpan span(topic_id, i * 10, tl_trace_shm, 256 * (i + 1), send);
    }

    auto spans = provider->getSpans();
    ASSERT_EQ(spans.size(), 5);

    for (int i = 0; i < 5; ++i)
    {
        EXPECT_EQ(spans[i].entity_id, 111);
        EXPECT_EQ(spans[i].process_id, static_cast<uint64_t>(eCAL::Process::GetProcessID()));
        EXPECT_EQ(spans[i].payload_size, 256 * (i + 1));
        EXPECT_EQ(spans[i].clock, i * 10);
    }

    triggerFlush(provider.get());
    EXPECT_EQ(provider->getSpans().size(), 0);

    auto metadata_lines = readJsonLines(metadataFilePath());
    ASSERT_GE(metadata_lines.size(), 1);
    EXPECT_EQ(metadata_lines.back()["topic_name"].get<std::string>(), "integration_test_topic");
    EXPECT_EQ(metadata_lines.back()["direction"].get<std::string>(), "publisher");

    auto span_lines = readJsonLines(spansFilePath());
    ASSERT_GE(span_lines.size(), 5);

    size_t matching = 0;
    for (const auto& line : span_lines)
    {
        if (line["entity_id"].get<uint64_t>() == 111)
            ++matching;
    }
    EXPECT_EQ(matching, 5u);
}

TEST_F(TracingIntegrationTest, AutoFlushWritesToFile)
{
    auto provider = eCAL::g_trace_provider();
    if (!provider) return;

    eCAL::STopicId topic_id;
    topic_id.topic_id.entity_id = 42;
    topic_id.topic_id.process_id = 43;

    for (int i = 0; i < 3; ++i)
    {
        CSpan span(topic_id, i, tl_trace_shm, 100, send);
    }

    provider->forceFlush();
    EXPECT_EQ(provider->getSpans().size(), 0);

    auto lines = readJsonLines(spansFilePath());
    ASSERT_GE(lines.size(), 3);
    for (int i = 0; i < 3; ++i)
    {
        size_t idx = lines.size() - 3 + i;
        EXPECT_EQ(lines[idx]["entity_id"].get<uint64_t>(), 42);
        EXPECT_EQ(lines[idx]["clock"].get<long long>(), i);
    }
}
