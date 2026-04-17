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

// ============ Tests for CSpan RAII Wrapper ============

class SpanTest : public ::testing::Test
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

TEST_F(SpanTest, SendSpanConstruction)
{
    eCAL::STopicId topic_id;
    topic_id.topic_id.entity_id = 111;
    topic_id.topic_id.process_id = 222;

    {
        CSpan span(topic_id, 100, tl_trace_shm, 256, send);
    }

    if (auto provider = eCAL::g_trace_provider(); provider)
    {
        auto spans = provider->getSpans();
        ASSERT_EQ(spans.size(), 1);

        EXPECT_EQ(spans[0].entity_id, 111);
        EXPECT_EQ(spans[0].process_id, 222);
        EXPECT_EQ(spans[0].payload_size, 256);
        EXPECT_EQ(spans[0].clock, 100);
        EXPECT_EQ(spans[0].layer, tl_trace_shm);
        EXPECT_EQ(spans[0].op_type, send);
        EXPECT_EQ(spans[0].topic_id, 0);  // Send spans have topic_id = 0
    }
}

TEST_F(SpanTest, SendSpanTimestamps)
{
    eCAL::STopicId topic_id;
    topic_id.topic_id.entity_id = 111;
    topic_id.topic_id.process_id = 222;

    {
        CSpan span(topic_id, 100, tl_trace_shm, 256, send);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    if (auto provider = eCAL::g_trace_provider(); provider)
    {
        auto spans = provider->getSpans();
        ASSERT_EQ(spans.size(), 1);

        EXPECT_GT(spans[0].start_ns, 0);
        EXPECT_GT(spans[0].end_ns, 0);
        EXPECT_LT(spans[0].start_ns, spans[0].end_ns);
        EXPECT_GE(spans[0].end_ns - spans[0].start_ns, 500000); // at least 0.5 ms
    }
}

TEST_F(SpanTest, ReceiveSpanConstruction)
{
    eCAL::Payload::TopicInfo topic_info;
    topic_info.topic_id = 999;
    topic_info.process_id = 333;

    {
        CSpan span(444, topic_info, 200, tl_trace_udp, 512, receive);
    }

    if (auto provider = eCAL::g_trace_provider(); provider)
    {
        auto spans = provider->getSpans();
        ASSERT_EQ(spans.size(), 1);

        EXPECT_EQ(spans[0].entity_id, 444);
        EXPECT_EQ(spans[0].topic_id, 999);
        EXPECT_EQ(spans[0].process_id, 333);
        EXPECT_EQ(spans[0].payload_size, 512);
        EXPECT_EQ(spans[0].clock, 200);
        EXPECT_EQ(spans[0].layer, tl_trace_udp);
        EXPECT_EQ(spans[0].op_type, receive);
    }
}

TEST_F(SpanTest, ReceiveSpanTimestamps)
{
    eCAL::Payload::TopicInfo topic_info;
    topic_info.topic_id = 999;
    topic_info.process_id = 333;

    {
        CSpan span(444, topic_info, 200, tl_trace_udp, 512, receive);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    if (auto provider = eCAL::g_trace_provider(); provider)
    {
        auto spans = provider->getSpans();
        ASSERT_EQ(spans.size(), 1);

        EXPECT_GT(spans[0].start_ns, 0);
        EXPECT_GT(spans[0].end_ns, 0);
        EXPECT_LT(spans[0].start_ns, spans[0].end_ns);
    }
}

TEST_F(SpanTest, CallbackExecutionSpan)
{
    eCAL::STopicId topic_id;
    topic_id.topic_id.entity_id = 50;
    topic_id.topic_id.process_id = 60;

    {
        CSpan span(topic_id, 300, tl_trace_tcp, 1024, callback_execution);
    }

    if (auto provider = eCAL::g_trace_provider(); provider)
    {
        auto spans = provider->getSpans();
        ASSERT_EQ(spans.size(), 1);
        EXPECT_EQ(spans[0].op_type, callback_execution);
        EXPECT_EQ(spans[0].layer, tl_trace_tcp);
        EXPECT_EQ(spans[0].payload_size, 1024);
    }
}

TEST_F(SpanTest, MultipleOperationTypes)
{
    eCAL::STopicId topic_id;
    topic_id.topic_id.entity_id = 111;
    topic_id.topic_id.process_id = 222;

    {
        CSpan span(topic_id, 100, tl_trace_shm, 256, send);
    }
    {
        CSpan span(topic_id, 100, tl_trace_shm, 256, callback_execution);
    }

    if (auto provider = eCAL::g_trace_provider(); provider)
    {
        auto spans = provider->getSpans();
        ASSERT_EQ(spans.size(), 2);

        EXPECT_EQ(spans[0].op_type, send);
        EXPECT_EQ(spans[1].op_type, callback_execution);
    }
}

TEST_F(SpanTest, MultipleLayerTypes)
{
    eCAL::STopicId topic_id;
    topic_id.topic_id.entity_id = 111;
    topic_id.topic_id.process_id = 222;

    { CSpan span1(topic_id, 100, tl_trace_shm,     256, send); }
    { CSpan span2(topic_id, 100, tl_trace_udp,     256, send); }
    { CSpan span3(topic_id, 100, tl_trace_tcp,     256, send); }
    { CSpan span4(topic_id, 100, tl_trace_shm_udp, 256, send); }

    if (auto provider = eCAL::g_trace_provider(); provider)
    {
        auto spans = provider->getSpans();
        ASSERT_EQ(spans.size(), 4);

        EXPECT_EQ(spans[0].layer, tl_trace_shm);
        EXPECT_EQ(spans[1].layer, tl_trace_udp);
        EXPECT_EQ(spans[2].layer, tl_trace_tcp);
        EXPECT_EQ(spans[3].layer, tl_trace_shm_udp);
    }
}
