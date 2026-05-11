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

#include "trace_provider_default.h"
#include "tracing_writer.h"
#include "tracing_writer_jsonl.h"
#include "util/single_instance_helper.h"

#include <cstdlib>
#include <atomic>
#include <thread>

namespace eCAL
namespace tracing
{

    std::shared_ptr<CTraceProviderDefault> CTraceProviderDefault::Create(std::unique_ptr<TracingWriter> writer, size_t batch_size)
    {
      try
      {
        return Util::CSingleInstanceHelper<CTraceProviderDefault>::Create(std::move(writer), batch_size);
      }
      catch (const std::exception& e)
      {
        return nullptr;
      }
    }

    CTraceProviderDefault::CTraceProviderDefault(std::unique_ptr<TracingWriter> writer, size_t batch_size)
        : batch_size_(batch_size), writer_(std::move(writer))
    {
        writer_thread_ = std::thread(&CTraceProviderDefault::WriterThreadLoop, this);
    }
    
    CTraceProviderDefault::~CTraceProviderDefault()
    {
        {
            std::lock_guard<std::mutex> const lock(thread_mutex);
            stop_thread_ = true;
            write_cv_.notify_all();
        }
        writer_thread_.join();
    }


    void CTraceProviderDefault::WriteSpan(const SpanDataVariant& span_data)
    {
        std::lock_guard<std::mutex> const lock(thread_mutex);
        span_buffer_.push_back(span_data);
        if (span_buffer_.size() >= batch_size_)
        {
            write_cv_.notify_one();
        }
    }

    void CTraceProviderDefault::WriterThreadLoop()
    {
        std::vector<SpanDataVariant> span_flusher;
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
                writer_->WriteSpansToFile(span_flusher);
                span_flusher.clear();
            }
        }
    }

    void CTraceProviderDefault::WriteMetadata(const STopicMetadata& metadata)
    {
        writer_->WriteMetadataToFile(metadata);
    }

} // namespace tracing
} // namespace eCAL
