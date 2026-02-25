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
#include <cstdlib>
#include <csignal>
#include <atomic>
#include <list>
#include <vector>
#include <mutex>
#include <chrono>
#include <ecal/types.h>
#include <serialization/ecal_struct_sample_common.h>
#include <serialization/ecal_struct_sample_payload.h>
#include <ecal/ecal.h>
#include <time.h>
#include <nlohmann/json.hpp>
using json = nlohmann::json;
using namespace std::chrono;

namespace eCAL
{
namespace tracing
{

    enum operation_type
    {
        send,
        receive,
        callback_execution
    };

    enum topic_direction
    {
        publisher,
        subscriber
    };

    // Metadata captured when a topic is created
    struct STopicMetadata
    {
        uint64_t         entity_id;       // unique entity id
        int32_t          process_id;      // PID of the owning process
        std::string      host_name;       // host that created the topic
        std::string      topic_name;      // topic name used for pub/sub matching
        std::string      encoding;        // datatype encoding (e.g. protobuf)
        std::string      type_name;       // datatype name
        topic_direction  direction;       // publisher or subscriber
    };

    // Data structures to hold span information
    struct SSendSpanData
    {
        uint64_t entity_id;
        uint64_t process_id;
        size_t payload_size;
        long long clock;
        uint64_t layer;
        long long start_ns;  // start timestamp in nanoseconds
        long long end_ns;    // end timestamp in nanoseconds
        operation_type op_type;
    };

    struct SReceiveSpanData
    {
        EntityIdT entity_id;
        uint64_t topic_id;
        uint64_t process_id;
        long long clock;
        uint64_t layer;
        long long start_ns;  // start timestamp in nanoseconds
        long long end_ns;    // end timestamp in nanoseconds
        operation_type op_type;
    };

    class CSendSpan {
    public:
        CSendSpan(const STopicId topic_id, long long clock, eTLayerType layer, size_t payload_size, operation_type op_type);
        ~CSendSpan();

    private:
        SSendSpanData data;
    };

    class CReceiveSpan {
    public:
        CReceiveSpan(EntityIdT entity_id, const eCAL::Payload::TopicInfo topic_info, long long clock, eTLayerType layer, operation_type op_type);
        ~CReceiveSpan();

    private:
        SReceiveSpanData data;
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

        // Topic metadata — written directly to file (no buffering)
        void addTopicMetadata(const STopicMetadata& metadata);
        
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

        void registerExitHandlers();
        static void atExitHandler();
        static void signalHandler(int signum);
        static std::atomic<bool> flush_done_;
        
        std::vector<SSendSpanData> send_span_buffer_;
        std::vector<SReceiveSpanData> receive_span_buffer_;
        size_t send_batch_size_{10};
        size_t receive_batch_size_{10};
        mutable std::mutex buffer_mutex_;
        mutable std::mutex metadata_mutex_;
        
        // Internal method for batch sending (to be implemented by user for their specific backend)
        void sendBatchSendSpans(const std::vector<SSendSpanData>& batch);
        void sendBatchReceiveSpans(const std::vector<SReceiveSpanData>& batch);
        void writeTopicMetadata(const STopicMetadata& metadata);
  };

} // namespace tracing

} // namespace eCAL
