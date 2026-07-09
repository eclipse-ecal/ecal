/* ========================= eCAL LICENSE =================================
 *
 * Copyright 2026 AUMOVIO and subsidiaries. All rights reserved.
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

#include <cstddef>
#include <cstdint>
#include <string>
#include <variant>

namespace eCAL
{
  namespace tracing
  {
    constexpr const char* kTracingVersion = "1.0.0";

    enum operation_type
    {
      send = 0,
      receive = 1,
      callback_execution = 2
    };

    enum topic_direction
    {
      publisher = 0,
      subscriber = 1
    };

    enum eTracingLayerType : uint64_t
    {
      tl_trace_none    = 0,
      tl_trace_shm     = 1 << 0,
      tl_trace_udp     = 1 << 1,
      tl_trace_tcp     = 1 << 2,
      tl_trace_shm_udp = tl_trace_shm | tl_trace_udp,
      tl_trace_shm_tcp = tl_trace_shm | tl_trace_tcp,
      tl_trace_udp_tcp = tl_trace_udp | tl_trace_tcp,
      tl_trace_all     = tl_trace_shm | tl_trace_udp | tl_trace_tcp,
    };

    struct STopicMetadata
    {
      std::string      tracing_version{kTracingVersion};
      uint64_t         entity_id{ 0 };
      int32_t          process_id{ 0 };
      std::string      process_name;
      std::string      host_name;
      std::string      topic_name;
      std::string      encoding;
      std::string      type_name;
      topic_direction  direction{ topic_direction::publisher };
    };

    struct SPublisherSpanData
    {
      operation_type op_type;
      uint64_t       entity_id;
      size_t         payload_size;
      long long      clock;
      uint64_t       layer;
      long long      start_ns;
      long long      end_ns;
    };

    struct SSubscriberSpanData
    {
      operation_type op_type;
      uint64_t       entity_id;
      uint64_t       topic_id;
      size_t         payload_size;
      long long      clock;
      uint64_t       layer;
      long long      start_ns;
      long long      end_ns;
    };

    using SpanData = std::variant<SPublisherSpanData, SSubscriberSpanData>;
    using TraceInfo = std::variant<STopicMetadata, SpanData>;
  }
}