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

#include "span.h"
#include "trace_provider.h"
#include "ecal_global_accessors.h"

#include <chrono>

using tracing_clock = std::chrono::system_clock;
using namespace std::chrono;

namespace eCAL
{
  namespace tracing
  {
    OptionalPublisherSpan CPublisherSpan::Create(const STopicId& topic_id, long long clock, eTracingLayerType layer, size_t payload_size, operation_type op_type)
    {
      auto provider = g_trace_provider();
      if (!provider) return std::nullopt;

      return OptionalPublisherSpan{
          std::in_place,
          CPublisherSpan::ConstructionToken{},
          std::move(provider),
          topic_id,
          clock,
          layer,
          payload_size,
          op_type
      };
    }

    // Send span constructor
    CPublisherSpan::CPublisherSpan(ConstructionToken /*token*/, std::shared_ptr<tracing::TraceProvider> provider_, const STopicId& topic_id, long long clock, eTracingLayerType layer, size_t payload_size, operation_type op_type)
      : provider(std::move(provider_))
    {
      auto now = tracing_clock::now();
      data.start_ns     = duration_cast<nanoseconds>(now.time_since_epoch()).count();
      data.entity_id    = topic_id.topic_id.entity_id;
      data.payload_size = payload_size;
      data.clock        = clock;
      data.layer        = layer;
      data.op_type      = op_type;
    }

    CPublisherSpan::~CPublisherSpan()
    {
      auto now = tracing_clock::now();
      data.end_ns = duration_cast<nanoseconds>(now.time_since_epoch()).count();
      provider->WriteSpan(data);
    }

    OptionalSubscriberSpan CSubscriberSpan::Create(EntityIdT entity_id, const eCAL::Payload::TopicInfo& topic_info, long long clock, eTracingLayerType layer, size_t payload_size, operation_type op_type)
    {
      auto provider = g_trace_provider();
      if (!provider) return std::nullopt;

      return OptionalSubscriberSpan{
          std::in_place,
          CSubscriberSpan::ConstructionToken{},
          std::move(provider),
          entity_id,
          topic_info,
          clock,
          layer,
          payload_size,
          op_type
      };
    }

    // Receive span constructor
    CSubscriberSpan::CSubscriberSpan(ConstructionToken /*token*/, std::shared_ptr<tracing::TraceProvider> provider_, EntityIdT entity_id, const eCAL::Payload::TopicInfo& topic_info, long long clock, eTracingLayerType layer, size_t payload_size, operation_type op_type)
      : provider(std::move(provider_))
    {
      auto now = tracing_clock::now();
      data.start_ns   = duration_cast<nanoseconds>(now.time_since_epoch()).count();
      data.entity_id  = entity_id;
      data.topic_id   = topic_info.topic_id;
      data.payload_size = payload_size;
      data.clock      = clock;
      data.layer      = layer;
      data.op_type    = op_type;
    }

    CSubscriberSpan::~CSubscriberSpan()
    {
      auto now = tracing_clock::now();
      data.end_ns = duration_cast<nanoseconds>(now.time_since_epoch()).count();
      provider->WriteSpan(data);
    }
  }
}
