/* ========================= eCAL LICENSE =================================
 *
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

#pragma once

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <mutex>

#include "tracing_writer.h"
#include "tracing.h"

namespace eCAL
{
  namespace tracing
  {
    // Responsible for serializing span and metadata to JSONL files.
    // Separated from CTraceProvider to isolate the I/O concern.
    // This class is not thread-safe; CTraceProviderDefault handles synchronization and buffering before calling this writer.
    class CTracingWriterJSONL : public TracingWriter
    {
    public:
      CTracingWriterJSONL();
      CTracingWriterJSONL(const std::string& file_id, std::filesystem::path trace_directory);
      ~CTracingWriterJSONL() override = default;

      CTracingWriterJSONL(const CTracingWriterJSONL&)            = delete;
      CTracingWriterJSONL& operator=(const CTracingWriterJSONL&) = delete;
      CTracingWriterJSONL(CTracingWriterJSONL&&)                 = delete;
      CTracingWriterJSONL& operator=(CTracingWriterJSONL&&)      = delete;

      // Write a batch of spans to the JSONL spans file
      void WriteTraceInfo(const std::vector<TraceInfo>& batch) override;

      // File path accessors (path is fixed at construction time)
      std::filesystem::path GetSpansFilePath() const;
      std::filesystem::path GetTopicMetadataFilePath() const;

    private:
      void WriteSpanData(const SpanData& span_data);
      void WriteTopicMetadata(const STopicMetadata& metadata);

      std::filesystem::path spans_file_path_;
      std::filesystem::path metadata_file_path_;

      std::ofstream spans_file_;
      std::ofstream metadata_file_;
    };
  }
}
