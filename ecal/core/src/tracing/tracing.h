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

/**
 * @file   tracing/tracing.h
 * @brief  Shared tracing types, enums, and constants used across the tracing subsystem.
**/

#pragma once

#include <string>
#include <cstdint>
#include <variant>
#include <ecal/types.h>
#include <ecal_struct_sample_common.h>

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
        send = 0,
        receive = 1,
        callback_execution = 2
    };

    // Specifies the direction of the topic (publisher or subscriber)
    enum topic_direction
    {
        publisher = 0,
        subscriber = 1
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

    struct SPublisherSpanData
    {
        operation_type op_type;
        uint64_t       entity_id;
        uint64_t       process_id;
        size_t         payload_size;
        long long      clock;
        uint64_t       layer;
        long long      start_ns;       // start timestamp in nanoseconds
        long long      end_ns;         // end timestamp in nanoseconds
    };

    struct SSubscriberSpanData
    {
        operation_type op_type;
        uint64_t       entity_id;
        uint64_t       topic_id;
        uint64_t       process_id;
        size_t         payload_size;
        long long      clock;
        uint64_t       layer;
        long long      start_ns;       // start timestamp in nanoseconds
        long long      end_ns;         // end timestamp in nanoseconds
    };

    // Variant type for buffering heterogeneous span data.
    // Extend this variant when new span types are added (e.g. client/server).
    using SpanDataVariant = std::variant<SPublisherSpanData, SSubscriberSpanData>;

} // namespace tracing
} // namespace eCAL
