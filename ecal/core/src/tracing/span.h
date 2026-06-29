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

#include <memory>
#include <optional>

#include "tracing.h"

#include <ecal/types.h>
#include <serialization/ecal_struct_sample_payload.h>
#include <ecal/pubsub/types.h>

namespace eCAL
{
  namespace tracing
  {
    class TraceProvider;

    // RAII span for send (publisher) operations.
    // Records start_ns on construction, end_ns + buffer on destruction.
    class CPublisherSpan
    {
    private:
      struct ConstructionToken
      {
      private:
        ConstructionToken() = default;
        friend class CPublisherSpan;
      };

    public:
      static std::optional<CPublisherSpan> Create(const STopicId& topic_id, long long clock, eTracingLayerType layer, size_t payload_size, operation_type op_type);

      CPublisherSpan(ConstructionToken token, std::shared_ptr<tracing::TraceProvider> provider_, const STopicId& topic_id, long long clock, eTracingLayerType layer, size_t payload_size, operation_type op_type);
      ~CPublisherSpan();

      CPublisherSpan(const CPublisherSpan&)            = delete;
      CPublisherSpan& operator=(const CPublisherSpan&) = delete;
      CPublisherSpan(CPublisherSpan&&)                 = default;
      CPublisherSpan& operator=(CPublisherSpan&&)      = delete;

    private:
      SPublisherSpanData data{};
      std::shared_ptr<tracing::TraceProvider> provider;
    };

    using OptionalPublisherSpan = std::optional<CPublisherSpan>;

    // RAII span for receive (subscriber) operations.
    // Records start_ns on construction, end_ns + buffer on destruction.
    class CSubscriberSpan
    {
    private:
      struct ConstructionToken
      {
      private:
        ConstructionToken() = default;
        friend class CSubscriberSpan;
      };

    public:
      static std::optional<CSubscriberSpan> Create(EntityIdT entity_id, const eCAL::Payload::TopicInfo& topic_info, long long clock, eTracingLayerType layer, size_t payload_size, operation_type op_type);
      CSubscriberSpan(ConstructionToken token, std::shared_ptr<tracing::TraceProvider> provider_, EntityIdT entity_id, const eCAL::Payload::TopicInfo& topic_info, long long clock, eTracingLayerType layer, size_t payload_size, operation_type op_type);
      ~CSubscriberSpan();

    private:
      CSubscriberSpan(const CSubscriberSpan&)            = delete;
      CSubscriberSpan& operator=(const CSubscriberSpan&) = delete;
      CSubscriberSpan(CSubscriberSpan&&)                 = default;
      CSubscriberSpan& operator=(CSubscriberSpan&&)      = delete;

    private:
      SSubscriberSpanData data{};
      std::shared_ptr<tracing::TraceProvider> provider;
    };

    using OptionalSubscriberSpan = std::optional<CSubscriberSpan>;

  }
}
