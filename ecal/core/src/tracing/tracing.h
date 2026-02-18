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
#include <ctime>
#include <list>
#include <vector>
#include <mutex>
#include <ecal/types.h>
#include <serialization/ecal_struct_sample_common.h>
#include <serialization/ecal_struct_sample_payload.h>
#include <ecal/ecal.h>
#include <time.h>

namespace eCAL
{
namespace tracing
{
    // Data structures to hold span information
    struct SSendSpanData
    {
        uint64_t entity_id;
        uint64_t process_id;
        size_t payload_size;
        long long clock;
        uint64_t layer;
        long long duration_us; // duration in microseconds
    };

    struct SReceiveSpanData
    {
        uint64_t entity_id;
        uint64_t process_id;
        long long clock;
        uint64_t layer;
        long long duration_us; // duration in microseconds
    };

    class CSendSpan {
    public:
        CSendSpan(const STopicId topic_id, long long clock, eTLayerType layer, size_t payload_size);
        ~CSendSpan();

    private:
        SSendSpanData data;
        timespec start_time;
    };

    class CReceiveSpan {
    public:
        CReceiveSpan(const eCAL::Payload::TopicInfo topic_info, long long clock, eTLayerType layer);
        ~CReceiveSpan();

    private:
        SReceiveSpanData data;
        timespec start_time;
    };

  class CTraceProvider {
    public:
        static CTraceProvider& getInstance()
        {
            static CTraceProvider instance;
            return instance;
        }

        CTraceProvider(const CTraceProvider&) = delete;
        void operator=(const CTraceProvider&) = delete;

        // Buffer management
        void setSendSpanBatchSize(size_t batch_size) { send_batch_size_ = batch_size; }
        void setReceiveSpanBatchSize(size_t batch_size) { receive_batch_size_ = batch_size; }
        
        // Add span data to buffer
        void addSendSpan(const SSendSpanData& span_data);
        void addReceiveSpan(const SReceiveSpanData& span_data);
        
        // Get buffered spans
        std::vector<SSendSpanData> getSendSpans() { return send_span_buffer_; }
        std::vector<SReceiveSpanData> getReceiveSpans() { return receive_span_buffer_; }
        
        // Flush buffered spans (send in batches if buffer is full or on demand)
        void flushSendSpans();
        void flushReceiveSpans();
        void flushAllSpans();

    private:
        CTraceProvider();
        ~CTraceProvider();
        
        std::vector<SSendSpanData> send_span_buffer_;
        std::vector<SReceiveSpanData> receive_span_buffer_;
        size_t send_batch_size_{10};
        size_t receive_batch_size_{10};
        mutable std::mutex buffer_mutex_;
        
        // Internal method for batch sending (to be implemented by user for their specific backend)
        void sendBatchSendSpans(const std::vector<SSendSpanData>& batch);
        void sendBatchReceiveSpans(const std::vector<SReceiveSpanData>& batch);
  };

} // namespace tracing

} // namespace eCAL
