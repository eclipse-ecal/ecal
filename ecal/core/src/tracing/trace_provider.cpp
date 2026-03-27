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

#include "trace_provider.h"
#include "tracing_writer.h"

#include <cstdlib>
#include <atomic>

namespace eCAL
{
namespace tracing
{

    // CTraceProvider implementation
    std::atomic<bool> CTraceProvider::flush_done_{false};

    CTraceProvider::CTraceProvider()
        : writer_(std::make_unique<CTracingWriter>())
    {
        registerExitHandlers();
    }
    
    CTraceProvider::~CTraceProvider()
    {
        if (!flush_done_.exchange(true))
        {
            flushSpans();
        }
    }

    void CTraceProvider::registerExitHandlers()
    {
        // atexit runs before static destructors — first chance to flush
        std::atexit(atExitHandler);
    }

    void CTraceProvider::atExitHandler()
    {
        if (!flush_done_.exchange(true))
        {
            getInstance().flushSpans();
        }
    }

    void CTraceProvider::bufferSpan(const SSpanData& span_data)
    {
        bool should_flush = false;
        {
            std::lock_guard<std::mutex> lock(buffer_mutex_);
            span_buffer_.push_back(span_data);
            should_flush = (span_buffer_.size() >= batch_size_);
        }
        
        // Auto-flush if batch size reached
        if (should_flush)
        {
            flushSpans();
        }
    }

    void CTraceProvider::flushSpans()
    {
        std::vector<SSpanData> batch_to_send;
        {
            std::lock_guard<std::mutex> lock(buffer_mutex_);
            if (span_buffer_.empty())
                return;
            
            batch_to_send.swap(span_buffer_);
        }
        
        writer_->writeBatchSpans(batch_to_send);
    }

    void CTraceProvider::addTopicMetadata(const STopicMetadata& metadata)
    {
        writer_->writeTopicMetadata(metadata);
    }

    std::string CTraceProvider::getSpansFilePath() const
    {
        return writer_->getSpansFilePath();
    }

    std::string CTraceProvider::getTopicMetadataFilePath() const
    {
        return writer_->getTopicMetadataFilePath();
    }

} // namespace tracing
} // namespace eCAL
