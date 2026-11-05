/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
 * Copyright 2026 AUMOVIO and subsidiaries. All rights reserved.
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

#include <ecal/tracing/types.h>
#include <ecal/tracing/writer_jsonl.h>

#include <gtest/gtest.h>

#include <fstream>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

TEST(TestTracingWriterJSONL, SpanWrites)
{
  constexpr size_t total_spans = 4000;
  ScopedTestDirectory trace_directory("ecal_tracing_writer_span_writes");

  std::filesystem::path spans_path;

  {
    eCAL::tracing::CTracingWriterJSONL writer("span_writes", trace_directory.Path());
    spans_path = writer.GetSpansFilePath();

    std::vector<eCAL::tracing::TraceInfo> batch;
    batch.reserve(total_spans);

    for (size_t i = 0; i < total_spans; ++i)
    {
      eCAL::tracing::SPublisherSpanData span{};
      span.op_type      = eCAL::tracing::operation_type::send;
      span.entity_id    = static_cast<uint64_t>(i);
      span.payload_size = 64;
      span.clock        = static_cast<long long>(i);
      span.layer        = eCAL::tracing::tl_trace_shm;
      span.start_ns     = 1000;
      span.end_ns       = 2000;
      batch.push_back(span);
    }

    writer.WriteTraceInfo(batch);
  }

  EXPECT_EQ(spans_path.filename().string(), "ecal_spans_span_writes.jsonl");
  EXPECT_EQ(CountAndValidateJsonlLines(spans_path), total_spans);
}

TEST(TestTracingWriterJSONL, MetadataWrites)
{
  constexpr size_t metadata_instances = 200;
  ScopedTestDirectory trace_directory("ecal_tracing_writer_metadata_writes");

  std::filesystem::path metadata_path;

  {
    eCAL::tracing::CTracingWriterJSONL writer("metadata_writes", trace_directory.Path());
    metadata_path = writer.GetTopicMetadataFilePath();

    for (size_t i = 0; i < metadata_instances; ++i)
    {
      eCAL::tracing::STopicMetadata metadata{};
      metadata.entity_id    = static_cast<uint64_t>(i);
      metadata.process_id   = 1;
      metadata.process_name = "my_process";
      metadata.host_name    = "test_host";
      metadata.topic_name   = "topic_" + std::to_string(i);
      metadata.encoding     = "protobuf";
      metadata.type_name    = "TestType";
      metadata.direction    = eCAL::tracing::topic_direction::publisher;
      writer.WriteTraceInfo({metadata});
    }
  }

  EXPECT_EQ(metadata_path.filename().string(), "ecal_metadata_metadata_writes.jsonl");
  EXPECT_EQ(CountAndValidateJsonlLines(metadata_path), metadata_instances);
}

TEST(TestTracingWriterJSONL, PublisherSpanJsonFields)
{
  ScopedTestDirectory trace_directory("ecal_tracing_writer_publisher_fields");

  std::filesystem::path spans_path;

  {
    eCAL::tracing::CTracingWriterJSONL writer("publisher_fields", trace_directory.Path());
    spans_path = writer.GetSpansFilePath();

    eCAL::tracing::SPublisherSpanData span{};
    span.op_type      = eCAL::tracing::operation_type::send;
    span.entity_id    = 42;
    span.payload_size = 256;
    span.clock        = 7;
    span.layer        = eCAL::tracing::tl_trace_udp;
    span.start_ns     = 1000;
    span.end_ns       = 2000;

    std::vector<eCAL::tracing::TraceInfo> batch;
    batch.push_back(span);
    writer.WriteTraceInfo(batch);
  }

  std::ifstream file(spans_path);
  ASSERT_TRUE(file.is_open());

  std::string line;
  ASSERT_TRUE(std::getline(file, line));
  auto j = nlohmann::json::parse(line);

  EXPECT_EQ(j.at("op_type"), static_cast<int>(eCAL::tracing::operation_type::send));
  EXPECT_EQ(j.at("entity_id"), 42u);
  EXPECT_EQ(j.at("payload_size"), 256u);
  EXPECT_EQ(j.at("clock"), 7);
  EXPECT_EQ(j.at("layer"), static_cast<uint64_t>(eCAL::tracing::tl_trace_udp));
  EXPECT_EQ(j.at("start_ns"), 1000);
  EXPECT_EQ(j.at("end_ns"), 2000);
}

TEST(TestTracingWriterJSONL, SubscriberSpanJsonFields)
{
  ScopedTestDirectory trace_directory("ecal_tracing_writer_subscriber_fields");

  std::filesystem::path spans_path;

  {
    eCAL::tracing::CTracingWriterJSONL writer("subscriber_fields", trace_directory.Path());
    spans_path = writer.GetSpansFilePath();

    eCAL::tracing::SSubscriberSpanData span{};
    span.op_type      = eCAL::tracing::operation_type::receive;
    span.entity_id    = 99;
    span.topic_id     = 55;
    span.payload_size = 512;
    span.clock        = 3;
    span.layer        = eCAL::tracing::tl_trace_tcp;
    span.start_ns     = 3000;
    span.end_ns       = 4000;

    std::vector<eCAL::tracing::TraceInfo> batch;
    batch.push_back(span);
    writer.WriteTraceInfo(batch);
  }

  std::ifstream file(spans_path);
  ASSERT_TRUE(file.is_open());

  std::string line;
  ASSERT_TRUE(std::getline(file, line));
  auto j = nlohmann::json::parse(line);

  EXPECT_EQ(j.at("op_type"), static_cast<int>(eCAL::tracing::operation_type::receive));
  EXPECT_EQ(j.at("entity_id"), 99u);
  EXPECT_EQ(j.at("topic_id"), 55u);
  EXPECT_EQ(j.at("payload_size"), 512u);
  EXPECT_EQ(j.at("clock"), 3);
  EXPECT_EQ(j.at("layer"), static_cast<uint64_t>(eCAL::tracing::tl_trace_tcp));
  EXPECT_EQ(j.at("start_ns"), 3000);
  EXPECT_EQ(j.at("end_ns"), 4000);
}