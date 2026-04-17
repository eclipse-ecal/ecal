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
#include "tracing_writer.h"
#include "tracing_writer_jsonl.h"
#include "util/single_instance_helper.h"

#include <vector>
#include <mutex>
#include <memory>
#include <atomic>
#include <thread>
#include <condition_variable>

namespace eCAL
{
namespace tracing
{

  class CTraceProvider 
  {
    friend class Util::CSingleInstanceHelper<CTraceProvider>;

    public:

        static std::shared_ptr<CTraceProvider> Create(std::unique_ptr<TracingWriter> writer = std::make_unique<CTracingWriterJSONL>(), size_t batch_size = kDefaultTracingBatchSize);

        CTraceProvider(const CTraceProvider&)            = delete;
        CTraceProvider& operator=(const CTraceProvider&) = delete;
        CTraceProvider(CTraceProvider&&)                 = delete;
        CTraceProvider& operator=(CTraceProvider&&)      = delete;

        ~CTraceProvider();
        
        // Add span data to buffer
        void bufferSpan(const SSpanData& span_data);

        // Topic metadata — written directly to file (no buffering)
        void addTopicMetadata(const STopicMetadata& metadata);
        
        // Get buffered spans
        std::vector<SSpanData> getSpans()
        {
            std::lock_guard<std::mutex> lock(thread_mutex);
            return span_buffer_;
        }

        // Synchronously flush all buffered spans to the writer
        void forceFlush()
        {
            std::vector<SSpanData> to_write;
            {
                std::lock_guard<std::mutex> lock(thread_mutex);
                to_write.swap(span_buffer_);
            }
            if (!to_write.empty())
                writer_->writeBatchSpans(to_write);
        }

    private:
        CTraceProvider(std::unique_ptr<TracingWriter> writer, size_t batch_size);
        void writerThreadLoop();

        std::atomic<size_t> batch_size_{kDefaultTracingBatchSize};
        std::vector<SSpanData> span_buffer_;
        mutable std::mutex thread_mutex;
        std::condition_variable write_cv_;
        bool stop_thread_{false};
        std::thread writer_thread_;
        std::unique_ptr<TracingWriter> writer_;
  };

} // namespace tracing
} // namespace eCAL
