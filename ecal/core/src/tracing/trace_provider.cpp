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
#include <thread>

namespace eCAL
{
namespace tracing
{

    CTraceProvider::CTraceProvider()
        : writer_(std::make_unique<CTracingWriter>())
    {
        writer_thread_ = std::thread(&CTraceProvider::writerThreadLoop, this);
    }
    
    CTraceProvider::~CTraceProvider()
    {
        {
            std::lock_guard<std::mutex> lock(thread_mutex);
            stop_thread_ = true;
            write_cv_.notify_all();
        }
        writer_thread_.join();
    }


    void CTraceProvider::bufferSpan(const SSpanData& span_data)
    {
        std::lock_guard<std::mutex> lock(thread_mutex);
        span_buffer_.push_back(span_data);
        if (span_buffer_.size() >= batch_size_)
        {
            write_cv_.notify_one();
        }
    }

    void CTraceProvider::writerThreadLoop()
    {
        std::vector<SSpanData> span_flusher;
        while (true)
        {
            {
                std::unique_lock<std::mutex> lock(thread_mutex);
                write_cv_.wait(lock, [this]()
                {
                    return stop_thread_|| (span_buffer_.size() >= batch_size_);
                });
                if (stop_thread_ && span_buffer_.empty())
                {
                    break;
                }
                span_flusher.swap(span_buffer_);
            }
            if (!span_flusher.empty())
            {
                writer_->writeBatchSpans(span_flusher);
                span_flusher.clear();
            }
        }
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
