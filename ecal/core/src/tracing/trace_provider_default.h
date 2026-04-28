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

#include "trace_provider.h"
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

  class CTraceProviderDefault : public TraceProvider
  {
    friend class Util::CSingleInstanceHelper<CTraceProviderDefault>;

    public:

        static std::shared_ptr<CTraceProviderDefault> Create(std::unique_ptr<TracingWriter> writer = std::make_unique<CTracingWriterJSONL>(), size_t batch_size = kDefaultTracingBatchSize);

        CTraceProviderDefault(const CTraceProviderDefault&)            = delete;
        CTraceProviderDefault& operator=(const CTraceProviderDefault&) = delete;
        CTraceProviderDefault(CTraceProviderDefault&&)                 = delete;
        CTraceProviderDefault& operator=(CTraceProviderDefault&&)      = delete;

        ~CTraceProviderDefault() override;
        
        // Write span data to buffer (accepts any span type via variant)
        void WriteSpan(const SpanDataVariant& span_data) override;

        // metadata — written directly to file (no buffering)
        void WriteMetadata(const STopicMetadata& metadata) override;
        
    private:
        CTraceProviderDefault(std::unique_ptr<TracingWriter> writer, size_t batch_size);
        void WriterThreadLoop();

        std::atomic<size_t> batch_size_{kDefaultTracingBatchSize};
        std::vector<SpanDataVariant> span_buffer_;
        mutable std::mutex thread_mutex;
        std::condition_variable write_cv_;
        bool stop_thread_{false};
        std::thread writer_thread_;
        std::unique_ptr<TracingWriter> writer_;
  };

} // namespace tracing
} // namespace eCAL
