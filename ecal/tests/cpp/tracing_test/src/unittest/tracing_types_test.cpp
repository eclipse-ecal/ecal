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

// ============ Tests for Enums and Constants ============

class TracingTypesTest : public ::testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(TracingTypesTest, TracingVersionConstant)
{
    EXPECT_STREQ(kTracingVersion, "1.0.0");
}

TEST_F(TracingTypesTest, DefaultTracingBatchSize)
{
    EXPECT_EQ(kDefaultTracingBatchSize, 500);
}

TEST_F(TracingTypesTest, SSpanDataDefaultInitialization)
{
    SSpanData span{};
    EXPECT_EQ(span.topic_id, 0);
    EXPECT_EQ(span.entity_id, 0);
    EXPECT_EQ(span.process_id, 0);
    EXPECT_EQ(span.payload_size, 0);
    EXPECT_EQ(span.clock, 0);
    EXPECT_EQ(span.layer, 0);
    EXPECT_EQ(span.start_ns, 0);
    EXPECT_EQ(span.end_ns, 0);
}

TEST_F(TracingTypesTest, SSpanDataAssignment)
{
    SSpanData span;
    span.entity_id = 12345;
    span.topic_id = 67890;
    span.process_id = 111;
    span.payload_size = 256;
    span.clock = 100;
    span.layer = tl_trace_shm;
    span.start_ns = 1000000;
    span.end_ns = 2000000;
    span.op_type = send;

    EXPECT_EQ(span.entity_id, 12345);
    EXPECT_EQ(span.topic_id, 67890);
    EXPECT_EQ(span.process_id, 111);
    EXPECT_EQ(span.payload_size, 256);
    EXPECT_EQ(span.clock, 100);
    EXPECT_EQ(span.layer, tl_trace_shm);
    EXPECT_EQ(span.start_ns, 1000000);
    EXPECT_EQ(span.end_ns, 2000000);
    EXPECT_EQ(span.op_type, send);
}

TEST_F(TracingTypesTest, STopicMetadataDefaultVersion)
{
    STopicMetadata metadata;
    EXPECT_STREQ(metadata.tracing_version.c_str(), kTracingVersion);
}

TEST_F(TracingTypesTest, STopicMetadataAssignment)
{
    STopicMetadata metadata;
    metadata.entity_id = 999;
    metadata.process_id = 42;
    metadata.host_name = "test-host";
    metadata.topic_name = "test_topic";
    metadata.encoding = "protobuf";
    metadata.type_name = "TestType";
    metadata.direction = publisher;

    EXPECT_EQ(metadata.entity_id, 999);
    EXPECT_EQ(metadata.process_id, 42);
    EXPECT_EQ(metadata.host_name, "test-host");
    EXPECT_EQ(metadata.topic_name, "test_topic");
    EXPECT_EQ(metadata.encoding, "protobuf");
    EXPECT_EQ(metadata.type_name, "TestType");
    EXPECT_EQ(metadata.direction, publisher);
}

TEST_F(TracingTypesTest, OperationTypeEnum)
{
    EXPECT_EQ(send, 0);
    EXPECT_EQ(receive, 1);
    EXPECT_EQ(callback_execution, 2);
}

TEST_F(TracingTypesTest, TopicDirectionEnum)
{
    EXPECT_EQ(publisher, 0);
    EXPECT_EQ(subscriber, 1);
}
