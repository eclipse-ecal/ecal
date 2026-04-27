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

#pragma once

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
  class CTracingWriterJSONL : public TracingWriter
  {
  public:
    CTracingWriterJSONL();
    ~CTracingWriterJSONL() override = default;

    CTracingWriterJSONL(const CTracingWriterJSONL&)            = delete;
    CTracingWriterJSONL& operator=(const CTracingWriterJSONL&) = delete;
    CTracingWriterJSONL(CTracingWriterJSONL&&)                 = delete;
    CTracingWriterJSONL& operator=(CTracingWriterJSONL&&)      = delete;

    // Write a batch of spans to the JSONL spans file
    void WriteSpansToFile(const std::vector<SpanDataVariant>& batch) override;

    // Write a single topic metadata entry to the JSONL metadata file
    void WriteMetadataToFile(const STopicMetadata& metadata) override;

    // File path accessors (path is fixed at construction time)
    std::string GetSpansFilePath() const;
    std::string GetTopicMetadataFilePath() const;

  private:
    mutable std::mutex spans_mutex_;
    mutable std::mutex metadata_mutex_;
    std::string        timestamp_;
    std::string        trace_dir_;
  };

} // namespace tracing
} // namespace eCAL
