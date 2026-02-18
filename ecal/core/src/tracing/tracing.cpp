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

#include "tracing.h"

namespace eCAL
{ namespace tracing {

    CSendSpan::CSendSpan(const STopicId topic_id, long long clock, eTLayerType layer, size_t payload_size)
    {
        clock_gettime(CLOCK_MONOTONIC, &start_time);
        data.entity_id = topic_id.topic_id.entity_id;
        data.process_id = topic_id.topic_id.process_id;
        data.payload_size = payload_size;
        data.clock = clock;
        data.layer = layer;
    }

    CSendSpan::~CSendSpan()
    {
        timespec end_time;
        clock_gettime(CLOCK_MONOTONIC, &end_time);
        data.duration_us = ((end_time.tv_sec - start_time.tv_sec) * 1e9 + (end_time.tv_nsec - start_time.tv_nsec)) / 1000;
        
        CTraceProvider::getInstance().addSendSpan(data);
    }

    CReceiveSpan::CReceiveSpan(const eCAL::Payload::TopicInfo topic_info, long long clock, eTLayerType layer)
    {
        clock_gettime(CLOCK_MONOTONIC, &start_time);
        data.entity_id = topic_info.topic_id;
        data.process_id = topic_info.process_id;
        data.clock = clock;
        data.layer = layer;
    }

    CReceiveSpan::~CReceiveSpan()
    {
        timespec end_time;
        clock_gettime(CLOCK_MONOTONIC, &end_time);
        data.duration_us = ((end_time.tv_sec - start_time.tv_sec) * 1e9 + (end_time.tv_nsec - start_time.tv_nsec)) / 1000;
        
        CTraceProvider::getInstance().addReceiveSpan(data);
    }

    // CTraceProvider implementation
    CTraceProvider::CTraceProvider() = default;
    
    CTraceProvider::~CTraceProvider() = default;

    void CTraceProvider::addSendSpan(const SSendSpanData& span_data)
    {
        {
            std::lock_guard<std::mutex> lock(buffer_mutex_);
            send_span_buffer_.push_back(span_data);
        }
        
        // Auto-flush if batch size reached
        if (send_span_buffer_.size() >= send_batch_size_)
        {
            flushSendSpans();
        }
    }

    void CTraceProvider::addReceiveSpan(const SReceiveSpanData& span_data)
    {
        {
            std::lock_guard<std::mutex> lock(buffer_mutex_);
            receive_span_buffer_.push_back(span_data);
        }
        
        // Auto-flush if batch size reached
        if (receive_span_buffer_.size() >= receive_batch_size_)
        {
            flushReceiveSpans();
        }
    }

    void CTraceProvider::flushSendSpans()
    {
        std::vector<SSendSpanData> batch_to_send;
        {
            std::lock_guard<std::mutex> lock(buffer_mutex_);
            if (send_span_buffer_.empty())
                return;
            
            batch_to_send.swap(send_span_buffer_);
        }
        
        sendBatchSendSpans(batch_to_send);
    }

    void CTraceProvider::flushReceiveSpans()
    {
        std::vector<SReceiveSpanData> batch_to_send;
        {
            std::lock_guard<std::mutex> lock(buffer_mutex_);
            if (receive_span_buffer_.empty())
                return;
            
            batch_to_send.swap(receive_span_buffer_);
        }
        
        sendBatchReceiveSpans(batch_to_send);
    }

    void CTraceProvider::flushAllSpans()
    {
        flushSendSpans();
        flushReceiveSpans();
    }

    void CTraceProvider::sendBatchSendSpans(const std::vector<SSendSpanData>& batch)
    {
        // Default implementation: print to stdout
        // Users can override this behavior by modifying this method
        for (const auto& span : batch)
        {
            std::cout << "SendSpan: entity_id=" << span.entity_id 
                      << ", process_id=" << span.process_id 
                      << ", clock=" << span.clock 
                      << ", layer=" << span.layer 
                      << ", payload_size=" << span.payload_size 
                      << ", duration=" << span.duration_us << " us" << std::endl;
        }
    }

    void CTraceProvider::sendBatchReceiveSpans(const std::vector<SReceiveSpanData>& batch)
    {
        // Default implementation: print to stdout
        // Users can override this behavior by modifying this method
        for (const auto& span : batch)
        {
            std::cout << "ReceiveSpan: entity_id=" << span.entity_id 
                      << ", process_id=" << span.process_id 
                      << ", clock=" << span.clock 
                      << ", layer=" << span.layer 
                      << ", duration=" << span.duration_us << " us" << std::endl;
        }
    }

} // namespace tracing

} // namespace eCAL