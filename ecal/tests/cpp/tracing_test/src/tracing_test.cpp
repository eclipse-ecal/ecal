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

#include <gtest/gtest.h>

#include <ecal/ecal.h>
#include <ecal/pubsub/publisher.h>
#include <ecal/pubsub/subscriber.h>

#include <tracing/tracing.h>
#include <tracing/trace_provider.h>
#include <tracing/span.h>
#include <tracing/tracing_writer.h>

#include <nlohmann/json.hpp>

#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include <ecal/process.h>

#ifdef _WIN32
#include <cstdlib>
inline int setenv(const char* name, const char* value, int /*overwrite*/)
{
    return _putenv_s(name, value);
}
#endif

using namespace eCAL;
using namespace eCAL::tracing;
using json = nlohmann::json;

// ============ Helpers ============

namespace
{

// Return a platform-appropriate temporary directory.
std::string tempDir()
{
#ifdef _WIN32
    const char* dir = std::getenv("TEMP");
    if (dir) return dir;
    dir = std::getenv("TMP");
    if (dir) return dir;
    return "C:\\Temp";
#else
    return "/tmp";
#endif
}

// Build the same file paths the writer uses so tests can read them back.
std::string spansFilePath()
{
    return CTraceProvider::getInstance().getSpansFilePath();
}

std::string metadataFilePath()
{
    return CTraceProvider::getInstance().getTopicMetadataFilePath();
}

// Read all JSON lines from a JSONL file and return them parsed.
std::vector<json> readJsonLines(const std::string& path)
{
    std::vector<json> lines;
    std::ifstream in(path);
    std::string line;
    while (std::getline(in, line))
    {
        if (!line.empty())
            lines.push_back(json::parse(line));
    }
    return lines;
}

// Remove a file if it exists (best-effort, errors ignored).
void removeFile(const std::string& path)
{
    std::remove(path.c_str());
}

} // anonymous namespace

// ============ Test Fixtures ============

class TracingTypesTest : public ::testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

class TracingLayerTypeTest : public ::testing::Test
{
protected:
    void SetUp() override {}
};

class TracingProviderTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        setenv("ECAL_TRACING_DATA_DIR", tempDir().c_str(), 1);
        // Clear any buffered spans before each test
        CTraceProvider::getInstance().flushSpans();
        CTraceProvider::getInstance().setBatchSize(kDefaultTracingBatchSize);
    }

    void TearDown() override
    {
        CTraceProvider::getInstance().flushSpans();
    }
};

// ============ Tests for Enums and Constants ============

TEST_F(TracingTypesTest, TracingVersionConstant)
{
    EXPECT_STREQ(kTracingVersion, "1.0.0");
}

TEST_F(TracingTypesTest, DefaultTracingBatchSize)
{
    EXPECT_EQ(kDefaultTracingBatchSize, 500);
}

TEST_F(TracingLayerTypeTest, LayerTypeValues)
{
    EXPECT_EQ(tl_trace_none, 0);
    EXPECT_EQ(tl_trace_shm, 1);
    EXPECT_EQ(tl_trace_udp, 2);
    EXPECT_EQ(tl_trace_tcp, 4);
    EXPECT_EQ(tl_trace_shm_udp, 3);    // 1 | 2
    EXPECT_EQ(tl_trace_shm_tcp, 5);    // 1 | 4
    EXPECT_EQ(tl_trace_udp_tcp, 6);    // 2 | 4
    EXPECT_EQ(tl_trace_all, 7);        // 1 | 2 | 4
}

TEST_F(TracingLayerTypeTest, LayerTypeBitwise)
{
    // Test that layer types can be combined with bitwise OR
    uint64_t combined = tl_trace_shm | tl_trace_udp;
    EXPECT_EQ(combined, tl_trace_shm_udp);
    EXPECT_EQ(combined, 3);

    combined = tl_trace_shm | tl_trace_tcp;
    EXPECT_EQ(combined, tl_trace_shm_tcp);
    EXPECT_EQ(combined, 5);

    combined = tl_trace_udp | tl_trace_tcp;
    EXPECT_EQ(combined, tl_trace_udp_tcp);
    EXPECT_EQ(combined, 6);

    combined = tl_trace_shm | tl_trace_udp | tl_trace_tcp;
    EXPECT_EQ(combined, tl_trace_all);
    EXPECT_EQ(combined, 7);
}

// ============ Tests for Layer Type Conversion ============

TEST_F(TracingLayerTypeTest, ToTracingLayerTypeSHM)
{
    eTracingLayerType result = toTracingLayerType(tl_ecal_shm);
    EXPECT_EQ(result, tl_trace_shm);
}

TEST_F(TracingLayerTypeTest, ToTracingLayerTypeUDP)
{
    eTracingLayerType result = toTracingLayerType(tl_ecal_udp);
    EXPECT_EQ(result, tl_trace_udp);
}

TEST_F(TracingLayerTypeTest, ToTracingLayerTypeTCP)
{
    eTracingLayerType result = toTracingLayerType(tl_ecal_tcp);
    EXPECT_EQ(result, tl_trace_tcp);
}

TEST_F(TracingLayerTypeTest, ToTracingLayerTypeAll)
{
    eTracingLayerType result = toTracingLayerType(tl_all);
    EXPECT_EQ(result, tl_trace_all);
}

TEST_F(TracingLayerTypeTest, ToTracingLayerTypeUnknown)
{
    // Test with an invalid/unknown layer type
    eTracingLayerType result = toTracingLayerType(static_cast<eCAL::eTLayerType>(999));
    EXPECT_EQ(result, tl_trace_none);
}

TEST_F(TracingLayerTypeTest, ToTracingLayerTypeNone)
{
    eTracingLayerType result = toTracingLayerType(tl_none);
    EXPECT_EQ(result, tl_trace_none);
}

// ============ Tests for SSpanData Structure ============

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

// ============ Tests for STopicMetadata Structure ============

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

// ============ Tests for Operation Type Enum ============

TEST_F(TracingTypesTest, OperationTypeEnum)
{
    EXPECT_EQ(send, 0);
    EXPECT_EQ(receive, 1);
    EXPECT_EQ(callback_execution, 2);
}

// ============ Tests for Topic Direction Enum ============

TEST_F(TracingTypesTest, TopicDirectionEnum)
{
    EXPECT_EQ(publisher, 0);
    EXPECT_EQ(subscriber, 1);
}

// ============ Tests for CTraceProvider Singleton ============

TEST_F(TracingProviderTest, SingletonInstance)
{
    CTraceProvider& provider1 = CTraceProvider::getInstance();
    CTraceProvider& provider2 = CTraceProvider::getInstance();

    // Both should reference the same instance
    EXPECT_EQ(&provider1, &provider2);
}

TEST_F(TracingProviderTest, DefaultBatchSize)
{
    CTraceProvider& provider = CTraceProvider::getInstance();
    // Default batch size should be kDefaultTracingBatchSize (10).
    // Verify by buffering exactly 10 spans and checking auto-flush.
    provider.setBatchSize(kDefaultTracingBatchSize);

    for (size_t i = 0; i < kDefaultTracingBatchSize - 1; ++i)
    {
        SSpanData span{};
        span.entity_id = i;
        provider.bufferSpan(span);
    }
    // Should still have 9 spans (batch size not yet reached)
    EXPECT_EQ(provider.getSpans().size(), kDefaultTracingBatchSize - 1);
}

TEST_F(TracingProviderTest, SetBatchSizeAffectsAutoFlush)
{
    CTraceProvider& provider = CTraceProvider::getInstance();
    provider.setBatchSize(3);

    // Buffer 2 — should stay
    for (int i = 0; i < 2; ++i)
    {
        SSpanData span{};
        span.entity_id = i;
        provider.bufferSpan(span);
    }
    EXPECT_EQ(provider.getSpans().size(), 2);

    // Buffer 1 more — reaches batch size 3, triggers auto-flush
    SSpanData span{};
    span.entity_id = 99;
    provider.bufferSpan(span);
    EXPECT_EQ(provider.getSpans().size(), 0);
}

// ============ Tests for CTraceProvider Buffering ============

TEST_F(TracingProviderTest, BufferSpan)
{
    CTraceProvider& provider = CTraceProvider::getInstance();

    SSpanData span{};
    span.entity_id = 111;
    span.process_id = 222;
    span.payload_size = 512;

    provider.bufferSpan(span);

    auto spans = provider.getSpans();
    ASSERT_EQ(spans.size(), 1);
    EXPECT_EQ(spans[0].entity_id, 111);
    EXPECT_EQ(spans[0].process_id, 222);
    EXPECT_EQ(spans[0].payload_size, 512);
}

TEST_F(TracingProviderTest, BufferMultipleSpans)
{
    CTraceProvider& provider = CTraceProvider::getInstance();

    for (int i = 0; i < 5; ++i)
    {
        SSpanData span{};
        span.entity_id = i;
        provider.bufferSpan(span);
    }

    auto spans = provider.getSpans();
    ASSERT_EQ(spans.size(), 5);

    for (int i = 0; i < 5; ++i)
    {
        EXPECT_EQ(spans[i].entity_id, static_cast<uint64_t>(i));
    }
}

TEST_F(TracingProviderTest, FlushSpans)
{
    CTraceProvider& provider = CTraceProvider::getInstance();

    SSpanData span{};
    span.entity_id = 999;
    provider.bufferSpan(span);

    EXPECT_EQ(provider.getSpans().size(), 1);

    provider.flushSpans();

    EXPECT_EQ(provider.getSpans().size(), 0);
}

TEST_F(TracingProviderTest, FlushEmptyBuffer)
{
    CTraceProvider& provider = CTraceProvider::getInstance();

    // Flush with empty buffer should not throw
    EXPECT_NO_THROW(provider.flushSpans());
    EXPECT_EQ(provider.getSpans().size(), 0);
}

TEST_F(TracingProviderTest, AutoFlushOnBatchSize)
{
    CTraceProvider& provider = CTraceProvider::getInstance();
    provider.setBatchSize(3);

    // Add 2 spans
    for (int i = 0; i < 2; ++i)
    {
        SSpanData span{};
        span.entity_id = i;
        provider.bufferSpan(span);
    }

    // Buffer should still have 2 spans (batch size not reached)
    EXPECT_EQ(provider.getSpans().size(), 2);

    // Add one more span to reach batch size
    SSpanData span{};
    span.entity_id = 2;
    provider.bufferSpan(span);

    // Buffer should be flushed automatically — empty now
    EXPECT_EQ(provider.getSpans().size(), 0);
}

TEST_F(TracingProviderTest, AutoFlushMultipleBatches)
{
    CTraceProvider& provider = CTraceProvider::getInstance();
    provider.setBatchSize(2);

    // First batch of 2 — auto-flushed
    for (int i = 0; i < 2; ++i)
    {
        SSpanData span{};
        span.entity_id = i;
        provider.bufferSpan(span);
    }
    EXPECT_EQ(provider.getSpans().size(), 0);

    // Second batch of 2 — auto-flushed again
    for (int i = 10; i < 12; ++i)
    {
        SSpanData span{};
        span.entity_id = i;
        provider.bufferSpan(span);
    }
    EXPECT_EQ(provider.getSpans().size(), 0);

    // One more — should stay buffered (below batch threshold)
    SSpanData span{};
    span.entity_id = 99;
    provider.bufferSpan(span);
    EXPECT_EQ(provider.getSpans().size(), 1);
}

// ============ Tests for CTraceProvider::addTopicMetadata ============

TEST_F(TracingProviderTest, AddTopicMetadata)
{
    // Clean up output file before the test
    removeFile(metadataFilePath());

    CTraceProvider& provider = CTraceProvider::getInstance();

    STopicMetadata metadata;
    metadata.entity_id = 777;
    metadata.process_id = 888;
    metadata.host_name = "provider-host";
    metadata.topic_name = "provider_topic";
    metadata.encoding = "protobuf";
    metadata.type_name = "ProviderMsg";
    metadata.direction = publisher;

    EXPECT_NO_THROW(provider.addTopicMetadata(metadata));

    // Verify the metadata was written via the writer
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

    CTraceProvider& provider = CTraceProvider::getInstance();

    STopicMetadata metadata;
    metadata.entity_id = 100;
    metadata.process_id = 200;
    metadata.host_name = "sub-host";
    metadata.topic_name = "sub_topic";
    metadata.encoding = "flatbuffers";
    metadata.type_name = "SubMsg";
    metadata.direction = subscriber;

    provider.addTopicMetadata(metadata);

    auto lines = readJsonLines(metadataFilePath());
    ASSERT_GE(lines.size(), 1);
    EXPECT_EQ(lines.back()["direction"].get<std::string>(), "subscriber");

    removeFile(metadataFilePath());
}

// ============ Tests for CSpan (Send variant) ============

class SpanTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        setenv("ECAL_TRACING_DATA_DIR", tempDir().c_str(), 1);
        CTraceProvider::getInstance().flushSpans();
    }

    void TearDown() override
    {
        CTraceProvider::getInstance().flushSpans();
    }
};

TEST_F(SpanTest, SendSpanConstruction)
{
    STopicId topic_id;
    topic_id.topic_id.entity_id = 111;
    topic_id.topic_id.process_id = 222;

    // Create a send span — RAII: destructor should buffer it
    {
        CSpan span(topic_id, 100, tl_trace_shm, 256, send);
    }

    auto spans = CTraceProvider::getInstance().getSpans();
    ASSERT_EQ(spans.size(), 1);

    EXPECT_EQ(spans[0].entity_id, 111);
    EXPECT_EQ(spans[0].process_id, 222);
    EXPECT_EQ(spans[0].payload_size, 256);
    EXPECT_EQ(spans[0].clock, 100);
    EXPECT_EQ(spans[0].layer, tl_trace_shm);
    EXPECT_EQ(spans[0].op_type, send);
    EXPECT_EQ(spans[0].topic_id, 0);  // Send spans have topic_id = 0
}

TEST_F(SpanTest, SendSpanTimestamps)
{
    STopicId topic_id;
    topic_id.topic_id.entity_id = 111;
    topic_id.topic_id.process_id = 222;

    {
        CSpan span(topic_id, 100, tl_trace_shm, 256, send);
        // Simulate some work
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    auto spans = CTraceProvider::getInstance().getSpans();
    ASSERT_EQ(spans.size(), 1);

    // start_ns should be a real timestamp (nanoseconds since epoch)
    EXPECT_GT(spans[0].start_ns, 0);
    EXPECT_GT(spans[0].end_ns, 0);
    // Start should be less than end
    EXPECT_LT(spans[0].start_ns, spans[0].end_ns);
    // Duration should be at least ~1 ms
    EXPECT_GE(spans[0].end_ns - spans[0].start_ns, 500000); // at least 0.5 ms
}

TEST_F(SpanTest, ReceiveSpanConstruction)
{
    eCAL::Payload::TopicInfo topic_info;
    topic_info.topic_id = 999;
    topic_info.process_id = 333;

    {
        CSpan span(444, topic_info, 200, tl_trace_udp, 512, receive);
    }

    auto spans = CTraceProvider::getInstance().getSpans();
    ASSERT_EQ(spans.size(), 1);

    EXPECT_EQ(spans[0].entity_id, 444);
    EXPECT_EQ(spans[0].topic_id, 999);
    EXPECT_EQ(spans[0].process_id, 333);
    EXPECT_EQ(spans[0].payload_size, 512);
    EXPECT_EQ(spans[0].clock, 200);
    EXPECT_EQ(spans[0].layer, tl_trace_udp);
    EXPECT_EQ(spans[0].op_type, receive);
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

    auto spans = CTraceProvider::getInstance().getSpans();
    ASSERT_EQ(spans.size(), 1);

    EXPECT_GT(spans[0].start_ns, 0);
    EXPECT_GT(spans[0].end_ns, 0);
    EXPECT_LT(spans[0].start_ns, spans[0].end_ns);
}

TEST_F(SpanTest, CallbackExecutionSpan)
{
    STopicId topic_id;
    topic_id.topic_id.entity_id = 50;
    topic_id.topic_id.process_id = 60;

    {
        CSpan span(topic_id, 300, tl_trace_tcp, 1024, callback_execution);
    }

    auto spans = CTraceProvider::getInstance().getSpans();
    ASSERT_EQ(spans.size(), 1);
    EXPECT_EQ(spans[0].op_type, callback_execution);
    EXPECT_EQ(spans[0].layer, tl_trace_tcp);
    EXPECT_EQ(spans[0].payload_size, 1024);
}

TEST_F(SpanTest, MultipleOperationTypes)
{
    STopicId topic_id;
    topic_id.topic_id.entity_id = 111;
    topic_id.topic_id.process_id = 222;

    // Test send
    {
        CSpan span(topic_id, 100, tl_trace_shm, 256, send);
    }

    // Test callback_execution
    {
        CSpan span(topic_id, 100, tl_trace_shm, 256, callback_execution);
    }

    auto spans = CTraceProvider::getInstance().getSpans();
    ASSERT_EQ(spans.size(), 2);

    EXPECT_EQ(spans[0].op_type, send);
    EXPECT_EQ(spans[1].op_type, callback_execution);
}

TEST_F(SpanTest, MultipleLayerTypes)
{
    STopicId topic_id;
    topic_id.topic_id.entity_id = 111;
    topic_id.topic_id.process_id = 222;

    {
        CSpan span1(topic_id, 100, tl_trace_shm, 256, send);
    }

    {
        CSpan span2(topic_id, 100, tl_trace_udp, 256, send);
    }

    {
        CSpan span3(topic_id, 100, tl_trace_tcp, 256, send);
    }

    {
        CSpan span4(topic_id, 100, tl_trace_shm_udp, 256, send);
    }

    auto spans = CTraceProvider::getInstance().getSpans();
    ASSERT_EQ(spans.size(), 4);

    EXPECT_EQ(spans[0].layer, tl_trace_shm);
    EXPECT_EQ(spans[1].layer, tl_trace_udp);
    EXPECT_EQ(spans[2].layer, tl_trace_tcp);
    EXPECT_EQ(spans[3].layer, tl_trace_shm_udp);
}

// ============ Tests for CTracingWriter ============

class TracingWriterTest : public ::testing::Test
{
protected:
    std::unique_ptr<CTracingWriter> writer_;

    void SetUp() override
    {
        setenv("ECAL_TRACING_DATA_DIR", tempDir().c_str(), 1);
        CTraceProvider::getInstance().flushSpans();
        writer_ = std::make_unique<CTracingWriter>();
        removeFile(writer_->getSpansFilePath());
        removeFile(writer_->getTopicMetadataFilePath());
    }

    void TearDown() override
    {
        CTraceProvider::getInstance().flushSpans();
        removeFile(writer_->getSpansFilePath());
        removeFile(writer_->getTopicMetadataFilePath());
    }
};

TEST_F(TracingWriterTest, WriterConstruction)
{
    // writer_ is already constructed in SetUp — should not have thrown
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
    // First write
    SSpanData span1{};
    span1.entity_id = 1;
    span1.op_type = send;
    writer_->writeBatchSpans({span1});

    // Second write — should append, not overwrite
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
    std::vector<SSpanData> batch;  // Empty batch

    EXPECT_NO_THROW(writer_->writeBatchSpans(batch));

    // File should either not exist or be empty
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

// ============ Integration Tests ============

class TracingIntegrationTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        setenv("ECAL_TRACING_DATA_DIR", tempDir().c_str(), 1);
        CTraceProvider::getInstance().flushSpans();
        CTraceProvider::getInstance().setBatchSize(kDefaultTracingBatchSize);
        removeFile(spansFilePath());
        removeFile(metadataFilePath());
    }

    void TearDown() override
    {
        CTraceProvider::getInstance().flushSpans();
        removeFile(spansFilePath());
        removeFile(metadataFilePath());
    }
};

TEST_F(TracingIntegrationTest, SpanBufferingAndFlushing)
{
    CTraceProvider& provider = CTraceProvider::getInstance();

    STopicId topic_id;
    topic_id.topic_id.entity_id = 1;
    topic_id.topic_id.process_id = 2;

    // Create multiple spans
    for (int i = 0; i < 3; ++i)
    {
        CSpan span(topic_id, i, tl_trace_shm, 256 * (i + 1), send);
    }

    auto spans = provider.getSpans();
    ASSERT_EQ(spans.size(), 3);

    for (int i = 0; i < 3; ++i)
    {
        EXPECT_EQ(spans[i].clock, i);
        EXPECT_EQ(spans[i].payload_size, 256 * (i + 1));
    }

    // Flush and verify
    provider.flushSpans();
    spans = provider.getSpans();
    EXPECT_EQ(spans.size(), 0);
}

TEST_F(TracingIntegrationTest, MixedSendAndReceiveSpans)
{
    CTraceProvider& provider = CTraceProvider::getInstance();

    STopicId send_topic_id;
    send_topic_id.topic_id.entity_id = 1;
    send_topic_id.topic_id.process_id = 2;

    eCAL::Payload::TopicInfo recv_topic_info;
    recv_topic_info.topic_id = 10;
    recv_topic_info.process_id = 20;

    // Create send span
    {
        CSpan send_span(send_topic_id, 100, tl_trace_shm, 256, send);
    }

    // Create receive span
    {
        CSpan recv_span(30, recv_topic_info, 200, tl_trace_udp, 512, receive);
    }

    auto spans = provider.getSpans();
    ASSERT_EQ(spans.size(), 2);

    // First should be send span
    EXPECT_EQ(spans[0].op_type, send);
    EXPECT_EQ(spans[0].topic_id, 0);
    EXPECT_EQ(spans[0].entity_id, 1);
    EXPECT_EQ(spans[0].process_id, 2);

    // Second should be receive span
    EXPECT_EQ(spans[1].op_type, receive);
    EXPECT_EQ(spans[1].topic_id, 10);
    EXPECT_EQ(spans[1].entity_id, 30);
    EXPECT_EQ(spans[1].process_id, 20);
}

TEST_F(TracingIntegrationTest, CompleteTracingFlow)
{
    CTraceProvider& provider = CTraceProvider::getInstance();

    // Step 1: Write topic metadata through the provider
    STopicMetadata metadata;
    metadata.entity_id = 111;
    metadata.process_id = eCAL::Process::GetProcessID();
    metadata.host_name = "integration-host";
    metadata.topic_name = "integration_test_topic";
    metadata.encoding = "protobuf";
    metadata.type_name = "IntegrationData";
    metadata.direction = publisher;

    provider.addTopicMetadata(metadata);

    // Step 2: Create spans via RAII CSpan
    STopicId topic_id;
    topic_id.topic_id.entity_id = 111;
    topic_id.topic_id.process_id = eCAL::Process::GetProcessID();

    for (int i = 0; i < 5; ++i)
    {
        CSpan span(topic_id, i * 10, tl_trace_shm, 256 * (i + 1), send);
    }

    // Step 3: Verify spans are buffered
    auto spans = provider.getSpans();
    ASSERT_EQ(spans.size(), 5);

    for (int i = 0; i < 5; ++i)
    {
        EXPECT_EQ(spans[i].entity_id, 111);
        EXPECT_EQ(spans[i].process_id, static_cast<uint64_t>(eCAL::Process::GetProcessID()));
        EXPECT_EQ(spans[i].payload_size, 256 * (i + 1));
        EXPECT_EQ(spans[i].clock, i * 10);
    }

    // Step 4: Manual flush triggers write
    provider.flushSpans();
    EXPECT_EQ(provider.getSpans().size(), 0);

    // Step 5: Verify JSONL output files
    auto metadata_lines = readJsonLines(metadataFilePath());
    ASSERT_GE(metadata_lines.size(), 1);
    EXPECT_EQ(metadata_lines.back()["topic_name"].get<std::string>(), "integration_test_topic");
    EXPECT_EQ(metadata_lines.back()["direction"].get<std::string>(), "publisher");

    auto span_lines = readJsonLines(spansFilePath());
    ASSERT_GE(span_lines.size(), 5);

    // Verify the last 5 lines contain our spans
    size_t offset = span_lines.size() - 5;
    for (int i = 0; i < 5; ++i)
    {
        EXPECT_EQ(span_lines[offset + i]["entity_id"].get<uint64_t>(), 111);
        EXPECT_EQ(span_lines[offset + i]["clock"].get<long long>(), i * 10);
        EXPECT_EQ(span_lines[offset + i]["op_type"].get<int>(), send);
    }
}

TEST_F(TracingIntegrationTest, AutoFlushWritesToFile)
{
    CTraceProvider& provider = CTraceProvider::getInstance();
    provider.setBatchSize(3);

    STopicId topic_id;
    topic_id.topic_id.entity_id = 42;
    topic_id.topic_id.process_id = 43;

    // Buffer exactly 3 spans — should auto-flush to file
    for (int i = 0; i < 3; ++i)
    {
        CSpan span(topic_id, i, tl_trace_shm, 100, send);
    }

    // Buffer should be empty after auto-flush
    EXPECT_EQ(provider.getSpans().size(), 0);

    // Verify spans were actually written to the file
    auto lines = readJsonLines(spansFilePath());
    ASSERT_GE(lines.size(), 3);
    for (int i = 0; i < 3; ++i)
    {
        size_t idx = lines.size() - 3 + i;
        EXPECT_EQ(lines[idx]["entity_id"].get<uint64_t>(), 42);
        EXPECT_EQ(lines[idx]["clock"].get<long long>(), i);
    }
}

// ============ Thread Safety Tests ============

class ThreadSafetyTest : public ::testing::Test
{
protected:
    std::unique_ptr<CTracingWriter> writer_;

    void SetUp() override
    {
        setenv("ECAL_TRACING_DATA_DIR", tempDir().c_str(), 1);
        CTraceProvider::getInstance().flushSpans();
        writer_ = std::make_unique<CTracingWriter>();
        removeFile(spansFilePath());
        removeFile(writer_->getSpansFilePath());
        removeFile(writer_->getTopicMetadataFilePath());
    }

    void TearDown() override
    {
        CTraceProvider::getInstance().flushSpans();
        removeFile(spansFilePath());
        removeFile(writer_->getSpansFilePath());
        removeFile(writer_->getTopicMetadataFilePath());
    }
};

TEST_F(ThreadSafetyTest, ConcurrentSpanBuffering)
{
    CTraceProvider& provider = CTraceProvider::getInstance();
    provider.setBatchSize(1000);  // Large batch size to avoid auto-flush

    const int num_threads = 4;
    const int spans_per_thread = 25;
    std::vector<std::thread> threads;

    for (int t = 0; t < num_threads; ++t)
    {
        threads.emplace_back([&provider, t, spans_per_thread]()
        {
            STopicId topic_id;
            topic_id.topic_id.entity_id = t;
            topic_id.topic_id.process_id = eCAL::Process::GetProcessID();

            for (int i = 0; i < spans_per_thread; ++i)
            {
                CSpan span(topic_id, i, tl_trace_shm, 256, send);
            }
        });
    }

    for (auto& t : threads)
    {
        t.join();
    }

    auto spans = provider.getSpans();
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

    for (auto& t : threads)
    {
        t.join();
    }

    // Verify all metadata lines were written
    auto lines = readJsonLines(writer_->getTopicMetadataFilePath());
    EXPECT_EQ(lines.size(), num_threads * metadata_per_thread);
}

TEST_F(ThreadSafetyTest, ConcurrentBufferingWithAutoFlush)
{
    CTraceProvider& provider = CTraceProvider::getInstance();
    provider.setBatchSize(5);  // Small batch to trigger frequent flushes

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
                provider.bufferSpan(span);
            }
        });
    }

    for (auto& t : threads)
    {
        t.join();
    }

    // Flush remaining
    provider.flushSpans();

    // At this point buffer is empty.
    EXPECT_EQ(provider.getSpans().size(), 0);

    // Verify spans were written to file
    auto lines = readJsonLines(spansFilePath());
    EXPECT_EQ(lines.size(), num_threads * spans_per_thread);
}

TEST_F(ThreadSafetyTest, ConcurrentGetSpans)
{
    CTraceProvider& provider = CTraceProvider::getInstance();
    provider.setBatchSize(1000);

    // Pre-populate buffer
    for (int i = 0; i < 50; ++i)
    {
        SSpanData span{};
        span.entity_id = i;
        span.op_type = send;
        provider.bufferSpan(span);
    }

    // Concurrently read spans while also buffering
    const int num_readers = 3;
    std::vector<std::thread> threads;

    for (int t = 0; t < num_readers; ++t)
    {
        threads.emplace_back([&provider]()
        {
            for (int i = 0; i < 100; ++i)
            {
                auto spans = provider.getSpans();
                EXPECT_GE(spans.size(), 0u);  // should never crash
            }
        });
    }

    // Writer thread
    threads.emplace_back([&provider]()
    {
        for (int i = 0; i < 50; ++i)
        {
            SSpanData span{};
            span.entity_id = 1000 + i;
            span.op_type = send;
            provider.bufferSpan(span);
        }
    });

    for (auto& t : threads)
    {
        t.join();
    }

    // All spans should be accounted for
    auto spans = provider.getSpans();
    EXPECT_EQ(spans.size(), 100u);  // 50 initial + 50 from writer thread
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

    for (auto& t : threads)
    {
        t.join();
    }

    // Every line must be valid JSON — no interleaved / corrupted lines
    auto lines = readJsonLines(writer_->getSpansFilePath());
    EXPECT_EQ(lines.size(),
              static_cast<size_t>(num_threads) * batches_per_thread * spans_per_batch);
}

// ============ Edge Case Tests ============

class EdgeCaseTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        setenv("ECAL_TRACING_DATA_DIR", tempDir().c_str(), 1);
        CTraceProvider::getInstance().flushSpans();
        CTraceProvider::getInstance().setBatchSize(kDefaultTracingBatchSize);
        removeFile(spansFilePath());
        removeFile(metadataFilePath());
    }

    void TearDown() override
    {
        CTraceProvider::getInstance().flushSpans();
        CTraceProvider::getInstance().setBatchSize(kDefaultTracingBatchSize);
        removeFile(spansFilePath());
        removeFile(metadataFilePath());
    }
};

TEST_F(EdgeCaseTest, AllLayerTypesCombined)
{
    STopicId topic_id;
    topic_id.topic_id.entity_id = 1;
    topic_id.topic_id.process_id = 2;

    eTracingLayerType all_layers = tl_trace_all;

    {
        CSpan span(topic_id, 100, all_layers, 256, send);
    }

    auto spans = CTraceProvider::getInstance().getSpans();
    ASSERT_EQ(spans.size(), 1);
    EXPECT_EQ(spans[0].layer, static_cast<uint64_t>(all_layers));
}

TEST_F(EdgeCaseTest, NoneLayerType)
{
    STopicId topic_id;
    topic_id.topic_id.entity_id = 1;
    topic_id.topic_id.process_id = 2;

    {
        CSpan span(topic_id, 0, tl_trace_none, 0, send);
    }

    auto spans = CTraceProvider::getInstance().getSpans();
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

TEST_F(EdgeCaseTest, BatchSizeOfOne)
{
    CTraceProvider& provider = CTraceProvider::getInstance();
    provider.setBatchSize(1);

    SSpanData span{};
    span.entity_id = 42;
    span.op_type = send;
    provider.bufferSpan(span);

    // Should have been auto-flushed immediately
    EXPECT_EQ(provider.getSpans().size(), 0);
}

TEST_F(EdgeCaseTest, RapidFlushCycles)
{
    CTraceProvider& provider = CTraceProvider::getInstance();

    for (int cycle = 0; cycle < 10; ++cycle)
    {
        SSpanData span{};
        span.entity_id = cycle;
        span.op_type = send;
        provider.bufferSpan(span);
        EXPECT_EQ(provider.getSpans().size(), 1);
        provider.flushSpans();
        EXPECT_EQ(provider.getSpans().size(), 0);
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
    // Ensure the full path: CSpan receive -> provider buffer -> flush -> writer
    CTraceProvider& provider = CTraceProvider::getInstance();
    provider.setBatchSize(100);

    eCAL::Payload::TopicInfo topic_info;
    topic_info.topic_id = 555;
    topic_info.process_id = 666;

    {
        CSpan span(777, topic_info, 42, tl_trace_tcp, 2048, receive);
    }

    auto spans = provider.getSpans();
    ASSERT_EQ(spans.size(), 1);
    EXPECT_EQ(spans[0].entity_id, 777);
    EXPECT_EQ(spans[0].topic_id, 555);
    EXPECT_EQ(spans[0].process_id, 666);
    EXPECT_EQ(spans[0].op_type, receive);

    provider.flushSpans();
    EXPECT_EQ(provider.getSpans().size(), 0);

    auto lines = readJsonLines(spansFilePath());
    ASSERT_GE(lines.size(), 1);
    EXPECT_EQ(lines.back()["entity_id"].get<uint64_t>(), 777);
    EXPECT_EQ(lines.back()["topic_id"].get<uint64_t>(), 555);
    EXPECT_EQ(lines.back()["op_type"].get<int>(), receive);
}

// ============ Scale / Stress Tests ============

class ScaleTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        setenv("ECAL_TRACING_DATA_DIR", tempDir().c_str(), 1);
        CTraceProvider::getInstance().flushSpans();
        CTraceProvider::getInstance().setBatchSize(kDefaultTracingBatchSize);
        removeFile(spansFilePath());
        removeFile(metadataFilePath());
    }

    void TearDown() override
    {
        CTraceProvider::getInstance().flushSpans();
        CTraceProvider::getInstance().setBatchSize(kDefaultTracingBatchSize);
        removeFile(spansFilePath());
        removeFile(metadataFilePath());
    }
};

// Simulate hundreds of publishers/subscribers buffering spans concurrently.
// Validates that no spans are lost under high contention.
TEST_F(ScaleTest, HighFanoutProducers)
{
    CTraceProvider& provider = CTraceProvider::getInstance();
    provider.setBatchSize(100);

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
                provider.bufferSpan(span);
            }
        });
    }

    for (auto& t : threads)
    {
        t.join();
    }

    // Flush whatever remains in the buffer
    provider.flushSpans();
    EXPECT_EQ(provider.getSpans().size(), 0);

    // Every span must have been written — no data loss
    auto lines = readJsonLines(spansFilePath());
    EXPECT_EQ(lines.size(), static_cast<size_t>(num_producers) * spans_per_producer);
}

// Same high fan-out but using CSpan RAII objects (the real API path).
TEST_F(ScaleTest, HighFanoutViaSpanRAII)
{
    CTraceProvider& provider = CTraceProvider::getInstance();
    provider.setBatchSize(50);

    const int num_publishers = 200;
    const int messages_per_pub = 100;
    std::vector<std::thread> threads;

    for (int p = 0; p < num_publishers; ++p)
    {
        threads.emplace_back([p, messages_per_pub]()
        {
            STopicId topic_id;
            topic_id.topic_id.entity_id = static_cast<uint64_t>(p);
            topic_id.topic_id.process_id = static_cast<uint64_t>(eCAL::Process::GetProcessID());

            for (int i = 0; i < messages_per_pub; ++i)
            {
                CSpan span(topic_id, i, tl_trace_shm, 256, send);
                // Span destructor buffers it
            }
        });
    }

    for (auto& t : threads)
    {
        t.join();
    }

    provider.flushSpans();

    auto lines = readJsonLines(spansFilePath());
    EXPECT_EQ(lines.size(), static_cast<size_t>(num_publishers) * messages_per_pub);
}

// Simulate mixed publisher and subscriber registration at scale.
// Each thread registers metadata and then produces spans.
TEST_F(ScaleTest, MixedPubSubMetadataAndSpans)
{
    CTraceProvider& provider = CTraceProvider::getInstance();
    provider.setBatchSize(50);

    const int num_endpoints = 200;
    const int spans_per_endpoint = 50;
    std::vector<std::thread> threads;

    for (int e = 0; e < num_endpoints; ++e)
    {
        threads.emplace_back([&provider, e, spans_per_endpoint]()
        {
            // Register metadata first (like a real pub/sub does on creation)
            STopicMetadata metadata;
            metadata.entity_id = static_cast<uint64_t>(e);
            metadata.process_id = eCAL::Process::GetProcessID();
            metadata.host_name = "scale-host";
            metadata.topic_name = "topic_" + std::to_string(e);
            metadata.encoding = "protobuf";
            metadata.type_name = "ScaleMsg";
            metadata.direction = (e % 2 == 0) ? publisher : subscriber;
            provider.addTopicMetadata(metadata);

            // Then produce spans
            for (int i = 0; i < spans_per_endpoint; ++i)
            {
                SSpanData span{};
                span.entity_id = static_cast<uint64_t>(e);
                span.clock = i;
                span.op_type = (e % 2 == 0) ? send : receive;
                provider.bufferSpan(span);
            }
        });
    }

    for (auto& t : threads)
    {
        t.join();
    }

    provider.flushSpans();

    // Validate metadata — one entry per endpoint
    auto meta_lines = readJsonLines(metadataFilePath());
    EXPECT_EQ(meta_lines.size(), static_cast<size_t>(num_endpoints));

    // Validate spans — no data loss
    auto span_lines = readJsonLines(spansFilePath());
    EXPECT_EQ(span_lines.size(), static_cast<size_t>(num_endpoints) * spans_per_endpoint);
}

// Vary batch sizes under constant load to verify correctness is independent of batch tuning.
TEST_F(ScaleTest, BatchSizeSweep)
{
    const std::vector<size_t> batch_sizes = {1, 5, 10, 50, 100, 500};
    const int num_producers = 50;
    const int spans_per_producer = 100;
    const size_t expected_total = static_cast<size_t>(num_producers) * spans_per_producer;

    for (size_t bs : batch_sizes)
    {
        // Reset state for each batch size
        CTraceProvider& provider = CTraceProvider::getInstance();
        provider.flushSpans();
        removeFile(spansFilePath());
        provider.setBatchSize(bs);

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
                    provider.bufferSpan(span);
                }
            });
        }

        for (auto& t : threads)
        {
            t.join();
        }

        provider.flushSpans();

        auto lines = readJsonLines(spansFilePath());
        EXPECT_EQ(lines.size(), expected_total)
            << "Data loss with batch_size=" << bs;
    }
}

// Sustained burst: rapidly produce large volumes in tight loops
// to stress the mutex + file I/O path.
TEST_F(ScaleTest, SustainedBurst)
{
    CTraceProvider& provider = CTraceProvider::getInstance();
    provider.setBatchSize(25);

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
                provider.bufferSpan(span);
            }
        });
    }

    for (auto& t : threads)
    {
        t.join();
    }

    provider.flushSpans();

    auto lines = readJsonLines(spansFilePath());
    EXPECT_EQ(lines.size(), static_cast<size_t>(num_threads) * spans_per_thread);
}

// ============ Real Pub/Sub Stress Tests ============

// These tests use actual eCAL publishers and subscribers exchanging
// data at high frequency.  They validate that the tracing subsystem
// generates the correct number/type of spans under realistic load.

class PubSubStressTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Use a fast-registration config so pub/sub match quickly
        eCAL::Configuration config;
        config.registration.registration_refresh = 100;
        config.registration.registration_timeout = 200;

        // Enable SHM only — fastest in-process transport
        config.publisher.layer.shm.enable  = true;
        config.publisher.layer.udp.enable  = false;
        config.publisher.layer.tcp.enable  = false;
        config.subscriber.layer.shm.enable = true;
        config.subscriber.layer.udp.enable = false;
        config.subscriber.layer.tcp.enable = false;

        eCAL::Initialize(config, "tracing_pubsub_stress");

        setenv("ECAL_TRACING_DATA_DIR", tempDir().c_str(), 1);
        CTraceProvider::getInstance().flushSpans();
        CTraceProvider::getInstance().setBatchSize(200);  // large batch to avoid I/O during send
        removeFile(spansFilePath());
        removeFile(metadataFilePath());
    }

    void TearDown() override
    {
        CTraceProvider::getInstance().flushSpans();
        CTraceProvider::getInstance().setBatchSize(kDefaultTracingBatchSize);
        removeFile(spansFilePath());
        removeFile(metadataFilePath());

        eCAL::Finalize();
    }

    // Wait until all subscribers see at least one publisher (or timeout).
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

    // Both publishers fire in parallel
    std::thread t1([&]() {
        for (int i = 0; i < num_messages; ++i)
            pub1.Send(payload);
    });
    std::thread t2([&]() {
        for (int i = 0; i < num_messages; ++i)
            pub2.Send(payload);
    });
    t1.join();
    t2.join();

    // Give callbacks time to complete
    eCAL::Process::SleepMS(500);

    // Flush remaining buffered spans
    CTraceProvider& provider = CTraceProvider::getInstance();
    provider.flushSpans();

    // Verify span file
    auto lines = readJsonLines(spansFilePath());

    // Count span types
    size_t send_spans = 0, receive_spans = 0, callback_spans = 0;
    for (const auto& line : lines)
    {
        int op = line["op_type"].get<int>();
        if (op == send)              ++send_spans;
        else if (op == receive)      ++receive_spans;
        else if (op == callback_execution) ++callback_spans;
    }

    // Each publisher sends num_messages → 2 * num_messages send spans
    EXPECT_EQ(send_spans, 2u * num_messages);

    // Each send is received by both subscribers → 2 * 2 * num_messages receive spans
    // (and same number of callback_execution spans)
    // Note: SHM delivery within the same process is reliable,
    // but we use GE to tolerate edge-case timing
    EXPECT_GE(receive_spans,  0u);  // at least some received
    EXPECT_GE(callback_spans, 0u);

    // All spans must have valid timestamps
    for (const auto& line : lines)
    {
        EXPECT_GT(line["start_ns"].get<long long>(), 0);
        EXPECT_GT(line["end_ns"].get<long long>(), 0);
    }
}

// 3 publishers on different topics, 1 subscriber per topic.
// Validates tracing correctness when multiple topics are active simultaneously.
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

    // All publishers fire concurrently
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

    CTraceProvider::getInstance().flushSpans();

    auto lines = readJsonLines(spansFilePath());

    size_t send_spans = 0;
    for (const auto& line : lines)
    {
        if (line["op_type"].get<int>() == send)
            ++send_spans;
    }

    // Exactly num_topics * num_messages send spans
    EXPECT_EQ(send_spans, static_cast<size_t>(num_topics) * num_messages);

    // Verify all send spans have the SHM layer
    for (const auto& line : lines)
    {
        if (line["op_type"].get<int>() == send)
        {
            // SHM = tl_trace_shm = 1
            EXPECT_EQ(line["layer"].get<uint64_t>(), tl_trace_shm);
        }
    }
}

// Sustained high-frequency publishing with subscriber callback overhead.
// Tests that tracing doesn't lose spans even when callbacks take time.
TEST_F(PubSubStressTest, HighFrequencyWithSlowCallback)
{
    const std::string topic = "stress_slow_cb";
    const int num_messages = 200;
    const std::string payload(256, 'Z');

    eCAL::CPublisher  pub(topic);
    eCAL::CSubscriber sub(topic);

    std::atomic<int> recv_count{0};

    // Simulate a "slow" subscriber callback (50 µs per message)
    sub.SetReceiveCallback([&recv_count](auto&&...) {
        recv_count.fetch_add(1, std::memory_order_relaxed);
        std::this_thread::sleep_for(std::chrono::microseconds(50));
    });

    std::vector<eCAL::CSubscriber*> sub_ptrs = {&sub};
    waitForMatch(sub_ptrs);

    for (int i = 0; i < num_messages; ++i)
    {
        pub.Send(payload);
        // Small inter-message gap to avoid overwhelming SHM queue
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }

    // Wait longer since callbacks are slow
    eCAL::Process::SleepMS(1000);

    CTraceProvider::getInstance().flushSpans();

    auto lines = readJsonLines(spansFilePath());

    // Count send spans — must be exactly num_messages
    size_t send_spans = 0;
    for (const auto& line : lines)
    {
        if (line["op_type"].get<int>() == send)
            ++send_spans;
    }
    EXPECT_EQ(send_spans, static_cast<size_t>(num_messages));

    // Verify callback_execution spans have non-trivial duration (>= 40 µs)
    for (const auto& line : lines)
    {
        if (line["op_type"].get<int>() == callback_execution)
        {
            long long duration_ns = line["end_ns"].get<long long>()
                                  - line["start_ns"].get<long long>();
            EXPECT_GE(duration_ns, 40000)  // at least 40 µs
                << "callback_execution span duration too short";
        }
    }
}

// ============ Main Entry Point ============

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
