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

#include <tracing/tracing_writer_jsonl.h>
#include <tracing/tracing.h>

#include <gtest/gtest.h>

#include <ecal_utils/barrier.h>

#include <atomic>
#include <filesystem>
#include <fstream>
#include <string>
#include <thread>
#include <vector>

#include <nlohmann/json.hpp>

TEST(TestTracingWriterJSONL, ConcurrentSpanWrites)
{
  constexpr size_t num_threads      = 100;
  constexpr size_t batches_per_thread = 50;
  constexpr size_t spans_per_batch  = 500;
  constexpr size_t total_spans      = num_threads * batches_per_thread * spans_per_batch;

  // Use a temporary directory for output files.
  auto tmp_dir = std::filesystem::temp_directory_path() / "ecal_tracing_writer_test_spans";
  std::filesystem::create_directories(tmp_dir);
  ScopedEnvVar env("ECAL_TRACING_DATA_DIR", tmp_dir.string());

  {
    eCAL::tracing::CTracingWriterJSONL writer;

    Barrier barrier(num_threads);

    std::vector<std::thread> threads;
    threads.reserve(num_threads);

    for (size_t t = 0; t < num_threads; ++t)
    {
      threads.emplace_back([&, t]()
      {
        barrier.wait();

        for (size_t b = 0; b < batches_per_thread; ++b)
        {
          std::vector<eCAL::tracing::SpanDataVariant> batch;
          batch.reserve(spans_per_batch);

          for (size_t i = 0; i < spans_per_batch; ++i)
          {
            eCAL::tracing::SPublisherSpanData span{};
            span.op_type      = eCAL::tracing::operation_type::send;
            span.entity_id    = static_cast<uint64_t>(t * batches_per_thread * spans_per_batch + b * spans_per_batch + i);
            span.process_id   = 1;
            span.payload_size = 64;
            span.clock        = static_cast<long long>(i);
            span.layer        = eCAL::tracing::tl_trace_shm;
            span.start_ns     = 1000;
            span.end_ns       = 2000;
            batch.push_back(span);
          }
          writer.WriteSpansToFile(batch);
        }
      });
    }

    for (auto& th : threads) th.join();

    // Verify: every span line must be valid JSON and the total count must match.
    std::string spans_path = writer.GetSpansFilePath();
    size_t line_count = CountAndValidateJsonlLines(spans_path);
    EXPECT_EQ(line_count, total_spans);
  }

  // Clean up temp files.
  std::filesystem::remove_all(tmp_dir);
}

TEST(TestTracingWriterJSONL, ConcurrentMetadataWrites)
{
  constexpr size_t num_threads          = 100;
  constexpr size_t metadata_per_thread  = 200;
  constexpr size_t total_metadata       = num_threads * metadata_per_thread;

  auto tmp_dir = std::filesystem::temp_directory_path() / "ecal_tracing_writer_test_metadata";
  std::filesystem::create_directories(tmp_dir);
  ScopedEnvVar env("ECAL_TRACING_DATA_DIR", tmp_dir.string());

  {
    eCAL::tracing::CTracingWriterJSONL writer;

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
          writer.WriteMetadataToFile(metadata);
        }
      });
    }

    for (auto& th : threads) th.join();

    std::string metadata_path = writer.GetTopicMetadataFilePath();
    size_t line_count = CountAndValidateJsonlLines(metadata_path);
    EXPECT_EQ(line_count, total_metadata);
  }

  std::filesystem::remove_all(tmp_dir);
}

TEST(TestTracingWriterJSONL, PublisherSpanJsonFields)
{
  auto tmp_dir = std::filesystem::temp_directory_path() / "ecal_tracing_writer_test_pub_fields";
  std::filesystem::create_directories(tmp_dir);
  ScopedEnvVar env("ECAL_TRACING_DATA_DIR", tmp_dir.string());

  {
    eCAL::tracing::CTracingWriterJSONL writer;

    eCAL::tracing::SPublisherSpanData span{};
    span.op_type      = eCAL::tracing::operation_type::send;
    span.entity_id    = 42;
    span.process_id   = 123;
    span.payload_size = 256;
    span.clock        = 7;
    span.layer        = eCAL::tracing::tl_trace_udp;
    span.start_ns     = 1000;
    span.end_ns       = 2000;

    std::vector<eCAL::tracing::SpanDataVariant> batch;
    batch.push_back(span);
    writer.WriteSpansToFile(batch);

    std::ifstream file(writer.GetSpansFilePath());
    ASSERT_TRUE(file.is_open());

    std::string line;
    ASSERT_TRUE(std::getline(file, line));
    auto j = nlohmann::json::parse(line);

    EXPECT_EQ(j.at("op_type"),      static_cast<int>(eCAL::tracing::operation_type::send));
    EXPECT_EQ(j.at("entity_id"),    42u);
    EXPECT_EQ(j.at("process_id"),   123u);
    EXPECT_EQ(j.at("payload_size"), 256u);
    EXPECT_EQ(j.at("clock"),        7);
    EXPECT_EQ(j.at("layer"),        static_cast<uint64_t>(eCAL::tracing::tl_trace_udp));
    EXPECT_EQ(j.at("start_ns"),     1000);
    EXPECT_EQ(j.at("end_ns"),       2000);
  }

  std::filesystem::remove_all(tmp_dir);
}

TEST(TestTracingWriterJSONL, SubscriberSpanJsonFields)
{
  auto tmp_dir = std::filesystem::temp_directory_path() / "ecal_tracing_writer_test_sub_fields";
  std::filesystem::create_directories(tmp_dir);
  ScopedEnvVar env("ECAL_TRACING_DATA_DIR", tmp_dir.string());

  {
    eCAL::tracing::CTracingWriterJSONL writer;

    eCAL::tracing::SSubscriberSpanData span{};
    span.op_type      = eCAL::tracing::operation_type::receive;
    span.entity_id    = 99;
    span.topic_id     = 55;
    span.process_id   = 456;
    span.payload_size = 512;
    span.clock        = 3;
    span.layer        = eCAL::tracing::tl_trace_tcp;
    span.start_ns     = 3000;
    span.end_ns       = 4000;

    std::vector<eCAL::tracing::SpanDataVariant> batch;
    batch.push_back(span);
    writer.WriteSpansToFile(batch);

    std::ifstream file(writer.GetSpansFilePath());
    ASSERT_TRUE(file.is_open());

    std::string line;
    ASSERT_TRUE(std::getline(file, line));
    auto j = nlohmann::json::parse(line);

    EXPECT_EQ(j.at("op_type"),      static_cast<int>(eCAL::tracing::operation_type::receive));
    EXPECT_EQ(j.at("entity_id"),    99u);
    EXPECT_EQ(j.at("topic_id"),     55u);
    EXPECT_EQ(j.at("process_id"),   456u);
    EXPECT_EQ(j.at("payload_size"), 512u);
    EXPECT_EQ(j.at("clock"),        3);
    EXPECT_EQ(j.at("layer"),        static_cast<uint64_t>(eCAL::tracing::tl_trace_tcp));
    EXPECT_EQ(j.at("start_ns"),     3000);
    EXPECT_EQ(j.at("end_ns"),       4000);
  }

  std::filesystem::remove_all(tmp_dir);
}

