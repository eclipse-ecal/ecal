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

#include "tracing.h"

#include <vector>
#include <mutex>
#include <memory>
#include <atomic>

// Forward declaration
namespace eCAL { namespace tracing { class CTracingWriter; } }

namespace eCAL
{
namespace tracing
{

  class CTraceProvider {
    public:
        static CTraceProvider& getInstance()
        {
            static CTraceProvider instance;
            return instance;
        }

        CTraceProvider(const CTraceProvider&)            = delete;
        CTraceProvider& operator=(const CTraceProvider&) = delete;
        CTraceProvider(CTraceProvider&&)                 = delete;
        CTraceProvider& operator=(CTraceProvider&&)      = delete;

        // Buffer management
        void setBatchSize(size_t batch_size) { batch_size_ = batch_size; }
        
        // Add span data to buffer
        void bufferSpan(const SSpanData& span_data);

        // Topic metadata — written directly to file (no buffering)
        void addTopicMetadata(const STopicMetadata& metadata);
        
        // Get buffered spans
        std::vector<SSpanData> getSpans()
        {
            std::lock_guard<std::mutex> lock(buffer_mutex_);
            return span_buffer_;
        }
        
        // Flush buffered spans
        void flushSpans();

        // File path accessors (delegated to the internal writer)
        std::string getSpansFilePath() const;
        std::string getTopicMetadataFilePath() const;

    private:
        CTraceProvider();
        ~CTraceProvider();

        void registerExitHandlers();
        static void atExitHandler();
        static std::atomic<bool> flush_done_;
        
        std::vector<SSpanData> span_buffer_;
        size_t batch_size_{kDefaultTracingBatchSize};
        mutable std::mutex buffer_mutex_;

        std::unique_ptr<CTracingWriter> writer_;
  };

} // namespace tracing
} // namespace eCAL
