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
#include <memory>
#include <chrono>
#include <ecal/types.h>
#include <serialization/ecal_struct_sample_common.h>
#include <serialization/ecal_struct_sample_payload.h>
#include <ecal/ecal.h>
#include <time.h>
#include <nlohmann/json.hpp>
using json = nlohmann::json;
using namespace std::chrono;

// Forward declaration
namespace eCAL { namespace tracing { class CTracingWriter; } }

namespace eCAL
{
namespace tracing
{

    // Version of the tracing implementation.
    constexpr const char* kTracingVersion = "1.0.0";

    // Default batch size for span buffering before flushing to backend (jsonl file)
    constexpr size_t kDefaultTracingBatchSize = 10;

    // Specifies the type of operation being traced
    enum operation_type
    {
        send,
        receive,
        callback_execution,
        shm_handshake
    };

    // Specifies the direction of the topic (publisher or subscriber)
    enum topic_direction
    {
        publisher,
        subscriber
    };

    // Bitmask enum for active transport layers used in tracing spans.
    // These use power-of-two values so combinations can be expressed with bitwise OR.
    enum eTracingLayerType : uint64_t
    {
        tl_trace_none    = 0,
        tl_trace_shm     = 1 << 0,  // 1
        tl_trace_udp     = 1 << 1,  // 2
        tl_trace_tcp     = 1 << 2,  // 4
        tl_trace_shm_udp = tl_trace_shm | tl_trace_udp,  // 3
        tl_trace_shm_tcp = tl_trace_shm | tl_trace_tcp,   // 5
        tl_trace_udp_tcp = tl_trace_udp | tl_trace_tcp,   // 6
        tl_trace_all     = tl_trace_shm | tl_trace_udp | tl_trace_tcp,  // 7
    };

    // Convert from eTLayerType (used in core APIs) to eTracingLayerType.
    inline eTracingLayerType toTracingLayerType(eTLayerType layer)
    {
      switch (layer)
      {
        case tl_ecal_shm: return tl_trace_shm;
        case tl_ecal_udp: return tl_trace_udp;
        case tl_ecal_tcp: return tl_trace_tcp;
        case tl_all:      return tl_trace_all;
        default:          return tl_trace_none;
      }
    }

    // Metadata captured when a topic is created
    struct STopicMetadata
    {
        std::string      tracing_version{kTracingVersion}; // tracing format version
        uint64_t         entity_id;       // unique entity id
        int32_t          process_id;      // PID of the owning process
        std::string      host_name;       // host that created the topic
        std::string      topic_name;      // topic name used for pub/sub matching
        std::string      encoding;        // datatype encoding (e.g. protobuf)
        std::string      type_name;       // datatype name
        topic_direction  direction;       // publisher or subscriber
    };

    // Unified span data structure.
    // All span types share the same struct; fields not applicable to a
    // particular operation_type are left at their zero-initialised default.
    //   - payload_size: populated for send / shm_handshake spans
    //   - topic_id:     populated for receive spans
    struct SSpanData
    {
        uint64_t       entity_id{0};
        uint64_t       topic_id{0};        // receive-only (0 for send spans)
        uint64_t       process_id{0};
        size_t         payload_size{0};    // send-only   (0 for receive spans)
        long long      clock{0};
        uint64_t       layer{0};
        long long      start_ns{0};       // start timestamp in nanoseconds
        long long      end_ns{0};         // end timestamp in nanoseconds
        operation_type op_type{send};
    };

    // RAII span — records start_ns on construction, end_ns + buffer on destruction.
    // Overloaded constructors cover send, receive, and SHM-handshake use cases.
    class CSpan {
    public:
        // Send span (publisher)
        CSpan(const STopicId& topic_id, long long clock, eTracingLayerType layer, size_t payload_size, operation_type op_type);
        // Receive span (subscriber)
        CSpan(EntityIdT entity_id, const eCAL::Payload::TopicInfo& topic_info, long long clock, eTracingLayerType layer, operation_type op_type);
        // SHM handshake span
        CSpan(uint64_t entity_id, int32_t process_id, long long clock);

        ~CSpan();

        CSpan(const CSpan&)            = delete;
        CSpan& operator=(const CSpan&) = delete;
        CSpan(CSpan&&)                 = delete;
        CSpan& operator=(CSpan&&)      = delete;

    private:
        SSpanData data;
    };

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
        std::vector<SSpanData> getSpans() { return span_buffer_; }
        
        // Flush buffered spans
        void flushSpans();

    private:
        CTraceProvider();
        ~CTraceProvider();

        void registerExitHandlers();
        static void atExitHandler();
        static void signalHandler(int signum);
        static std::atomic<bool> flush_done_;
        
        std::vector<SSpanData> span_buffer_;
        size_t batch_size_{kDefaultTracingBatchSize};
        mutable std::mutex buffer_mutex_;

        std::unique_ptr<CTracingWriter> writer_;
  };

} // namespace tracing

} // namespace eCAL
