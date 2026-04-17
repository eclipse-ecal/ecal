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

// ============ Tests for CTracingWriter ============

class TracingWriterTest : public ::testing::Test
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
        removeFile(writer_->getSpansFilePath());
        removeFile(writer_->getTopicMetadataFilePath());
    }

    void TearDown() override
    {
        if (auto provider = eCAL::g_trace_provider(); provider)
        {
            triggerFlush(provider.get());
        }
        removeFile(writer_->getSpansFilePath());
        removeFile(writer_->getTopicMetadataFilePath());
    }
};

TEST_F(TracingWriterTest, WriterConstruction)
{
    SUCCEED();
}

TEST_F(TracingWriterTest, WriteBatchSpansVerifyContent)
{
    std::vector<SSpanData> batch;

    for (int i = 0; i < 3; ++i)
    {
        SSpanData span{};
        span.entity_id    = 100 + i;
        span.topic_id     = 200 + i;
        span.process_id   = 300 + i;
        span.payload_size = 256 * (i + 1);
        span.clock        = 10 * i;
        span.layer        = tl_trace_shm;
        span.start_ns     = 1000000 + i * 100;
        span.end_ns       = 2000000 + i * 100;
        span.op_type      = send;
        batch.push_back(span);
    }

    writer_->writeBatchSpans(batch);

    auto lines = readJsonLines(writer_->getSpansFilePath());
    ASSERT_EQ(lines.size(), 3);

    for (int i = 0; i < 3; ++i)
    {
        EXPECT_EQ(lines[i]["entity_id"].get<uint64_t>(),  100 + i);
        EXPECT_EQ(lines[i]["topic_id"].get<uint64_t>(),   200 + i);
        EXPECT_EQ(lines[i]["process_id"].get<uint64_t>(), 300 + i);
        EXPECT_EQ(lines[i]["payload_size"].get<size_t>(), 256 * (i + 1));
        EXPECT_EQ(lines[i]["clock"].get<long long>(),     10 * i);
        EXPECT_EQ(lines[i]["layer"].get<uint64_t>(),      tl_trace_shm);
        EXPECT_EQ(lines[i]["start_ns"].get<long long>(),  1000000 + i * 100);
        EXPECT_EQ(lines[i]["end_ns"].get<long long>(),    2000000 + i * 100);
        EXPECT_EQ(lines[i]["op_type"].get<int>(),         send);
    }
}

TEST_F(TracingWriterTest, WriteBatchSpansReceive)
{
    SSpanData span{};
    span.entity_id    = 10;
    span.topic_id     = 20;
    span.process_id   = 30;
    span.payload_size = 512;
    span.clock        = 5;
    span.layer        = tl_trace_udp;
    span.start_ns     = 5000000;
    span.end_ns       = 6000000;
    span.op_type      = receive;

    writer_->writeBatchSpans({span});

    auto lines = readJsonLines(writer_->getSpansFilePath());
    ASSERT_EQ(lines.size(), 1);
    EXPECT_EQ(lines[0]["op_type"].get<int>(), receive);
    EXPECT_EQ(lines[0]["topic_id"].get<uint64_t>(), 20);
    EXPECT_EQ(lines[0]["layer"].get<uint64_t>(), tl_trace_udp);
}

TEST_F(TracingWriterTest, WriteBatchSpansAppends)
{
    SSpanData span1{};
    span1.entity_id = 1;
    span1.op_type = send;
    writer_->writeBatchSpans({span1});

    SSpanData span2{};
    span2.entity_id = 2;
    span2.op_type = send;
    writer_->writeBatchSpans({span2});

    auto lines = readJsonLines(writer_->getSpansFilePath());
    ASSERT_EQ(lines.size(), 2);
    EXPECT_EQ(lines[0]["entity_id"].get<uint64_t>(), 1);
    EXPECT_EQ(lines[1]["entity_id"].get<uint64_t>(), 2);
}

TEST_F(TracingWriterTest, WriteEmptyBatch)
{
    std::vector<SSpanData> batch;

    EXPECT_NO_THROW(writer_->writeBatchSpans(batch));

    auto lines = readJsonLines(writer_->getSpansFilePath());
    EXPECT_EQ(lines.size(), 0);
}

TEST_F(TracingWriterTest, WriteTopicMetadataVerifyContent)
{
    STopicMetadata metadata;
    metadata.entity_id = 555;
    metadata.process_id = 666;
    metadata.host_name = "test-host";
    metadata.topic_name = "test_topic";
    metadata.encoding = "protobuf";
    metadata.type_name = "TestMessage";
    metadata.direction = publisher;

    writer_->writeTopicMetadata(metadata);

    auto lines = readJsonLines(writer_->getTopicMetadataFilePath());
    ASSERT_EQ(lines.size(), 1);

    const auto& obj = lines[0];
    EXPECT_EQ(obj["tracing_version"].get<std::string>(), kTracingVersion);
    EXPECT_EQ(obj["entity_id"].get<uint64_t>(), 555);
    EXPECT_EQ(obj["process_id"].get<int32_t>(), 666);
    EXPECT_EQ(obj["host_name"].get<std::string>(), "test-host");
    EXPECT_EQ(obj["topic_name"].get<std::string>(), "test_topic");
    EXPECT_EQ(obj["encoding"].get<std::string>(), "protobuf");
    EXPECT_EQ(obj["type_name"].get<std::string>(), "TestMessage");
    EXPECT_EQ(obj["direction"].get<std::string>(), "publisher");
}

TEST_F(TracingWriterTest, WriteTopicMetadataSubscriber)
{
    STopicMetadata metadata;
    metadata.entity_id = 10;
    metadata.process_id = 20;
    metadata.host_name = "sub-host";
    metadata.topic_name = "sub_topic";
    metadata.encoding = "raw";
    metadata.type_name = "RawData";
    metadata.direction = subscriber;

    writer_->writeTopicMetadata(metadata);

    auto lines = readJsonLines(writer_->getTopicMetadataFilePath());
    ASSERT_EQ(lines.size(), 1);
    EXPECT_EQ(lines[0]["direction"].get<std::string>(), "subscriber");
}

TEST_F(TracingWriterTest, WriteMultipleMetadataAppends)
{
    for (int i = 0; i < 3; ++i)
    {
        STopicMetadata metadata;
        metadata.entity_id = 555 + i;
        metadata.process_id = 666 + i;
        metadata.host_name = "host-" + std::to_string(i);
        metadata.topic_name = "topic_" + std::to_string(i);
        metadata.encoding = "protobuf";
        metadata.type_name = "TestMessage";
        metadata.direction = (i % 2 == 0) ? publisher : subscriber;

        writer_->writeTopicMetadata(metadata);
    }

    auto lines = readJsonLines(writer_->getTopicMetadataFilePath());
    ASSERT_EQ(lines.size(), 3);

    for (int i = 0; i < 3; ++i)
    {
        EXPECT_EQ(lines[i]["entity_id"].get<uint64_t>(), 555 + i);
        EXPECT_EQ(lines[i]["process_id"].get<int32_t>(), 666 + i);
        EXPECT_EQ(lines[i]["host_name"].get<std::string>(), "host-" + std::to_string(i));
        EXPECT_EQ(lines[i]["topic_name"].get<std::string>(), "topic_" + std::to_string(i));
        std::string expected_dir = (i % 2 == 0) ? "publisher" : "subscriber";
        EXPECT_EQ(lines[i]["direction"].get<std::string>(), expected_dir);
    }
}

TEST_F(TracingWriterTest, EmptyStringMetadataFields)
{
    STopicMetadata metadata;
    metadata.host_name = "";
    metadata.topic_name = "";
    metadata.encoding = "";
    metadata.type_name = "";
    metadata.entity_id = 0;
    metadata.process_id = 0;
    metadata.direction = publisher;

    writer_->writeTopicMetadata(metadata);

    auto lines = readJsonLines(writer_->getTopicMetadataFilePath());
    ASSERT_EQ(lines.size(), 1);
    EXPECT_EQ(lines[0]["host_name"].get<std::string>(), "");
    EXPECT_EQ(lines[0]["topic_name"].get<std::string>(), "");
    EXPECT_EQ(lines[0]["encoding"].get<std::string>(), "");
    EXPECT_EQ(lines[0]["type_name"].get<std::string>(), "");
    EXPECT_EQ(lines[0]["entity_id"].get<uint64_t>(), 0);
    EXPECT_EQ(lines[0]["process_id"].get<int32_t>(), 0);
}

TEST_F(TracingWriterTest, WriteBatchSpansCallbackExecution)
{
    SSpanData span{};
    span.entity_id    = 7;
    span.topic_id     = 0;
    span.process_id   = 8;
    span.payload_size = 128;
    span.clock        = 42;
    span.layer        = tl_trace_tcp;
    span.start_ns     = 100;
    span.end_ns       = 200;
    span.op_type      = callback_execution;

    writer_->writeBatchSpans({span});

    auto lines = readJsonLines(writer_->getSpansFilePath());
    ASSERT_EQ(lines.size(), 1);
    EXPECT_EQ(lines[0]["op_type"].get<int>(), callback_execution);
    EXPECT_EQ(lines[0]["layer"].get<uint64_t>(), tl_trace_tcp);
}

TEST_F(TracingWriterTest, WriteBatchSpansAllLayerTypes)
{
    const std::vector<uint64_t> layer_types = {
        tl_trace_none, tl_trace_shm, tl_trace_udp, tl_trace_tcp,
        tl_trace_shm_udp, tl_trace_shm_tcp, tl_trace_udp_tcp, tl_trace_all
    };

    std::vector<SSpanData> batch;
    for (auto layer : layer_types)
    {
        SSpanData span{};
        span.layer = layer;
        span.op_type = send;
        batch.push_back(span);
    }

    writer_->writeBatchSpans(batch);

    auto lines = readJsonLines(writer_->getSpansFilePath());
    ASSERT_EQ(lines.size(), layer_types.size());

    for (size_t i = 0; i < layer_types.size(); ++i)
    {
        EXPECT_EQ(lines[i]["layer"].get<uint64_t>(), layer_types[i]);
    }
}
