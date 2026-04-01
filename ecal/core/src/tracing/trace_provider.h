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
#include "itracing_writer.h"
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

        static std::shared_ptr<CTraceProvider> Create();
        ~CTraceProvider();

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
            std::lock_guard<std::mutex> lock(thread_mutex);
            return span_buffer_;
        }
        

        // File path accessors (delegated to the internal writer)
        std::string getSpansFilePath() const;
        std::string getTopicMetadataFilePath() const;

        // Replace the active writer. The new writer takes effect immediately.
        // Pass nullptr to disable writing.
        void setWriter(std::unique_ptr<ITracingWriter> writer);

    private:
        CTraceProvider();
        void writerThreadLoop();

        std::atomic<size_t> batch_size_{kDefaultTracingBatchSize};
        std::vector<SSpanData> span_buffer_;
        mutable std::mutex thread_mutex;
        std::condition_variable write_cv_;
        bool stop_thread_{false};
        std::thread writer_thread_;
        std::unique_ptr<ITracingWriter> writer_;
  };

} // namespace tracing
} // namespace eCAL
