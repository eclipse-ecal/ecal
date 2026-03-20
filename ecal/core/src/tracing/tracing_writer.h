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

#include "tracing.h"

namespace eCAL
{
namespace tracing
{

  // Responsible for serializing span and metadata to JSONL files.
  // Separated from CTraceProvider to isolate the I/O concern.
  class CTracingWriter
  {
  public:
    CTracingWriter();
    ~CTracingWriter() = default;

    CTracingWriter(const CTracingWriter&)            = delete;
    CTracingWriter& operator=(const CTracingWriter&) = delete;
    CTracingWriter(CTracingWriter&&)                 = delete;
    CTracingWriter& operator=(CTracingWriter&&)      = delete;

    // Write a batch of spans to the JSONL spans file
    void writeBatchSpans(const std::vector<SSpanData>& batch);

    // Write a single topic metadata entry to the JSONL metadata file
    void writeTopicMetadata(const STopicMetadata& metadata);

    // File path accessors (path is fixed at construction time)
    std::string getSpansFilePath() const;
    std::string getTopicMetadataFilePath() const;

  private:
    mutable std::mutex spans_mutex_;
    mutable std::mutex metadata_mutex_;
    std::string        timestamp_;
  };

} // namespace tracing
} // namespace eCAL
