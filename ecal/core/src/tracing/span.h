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

#include "tracing.h"

#include <ecal/types.h>
#include <serialization/ecal_struct_sample_payload.h>
#include <ecal/pubsub/types.h>

namespace eCAL
{
namespace tracing
{

    // RAII span — records start_ns on construction, end_ns + buffer on destruction.
    // Overloaded constructors cover send and receive use cases.
    class CSpan {
    public:
        // Send span (publisher)
        CSpan(const STopicId& topic_id, long long clock, eTracingLayerType layer, size_t payload_size, operation_type op_type);
        // Receive span (subscriber)
        CSpan(EntityIdT entity_id, const eCAL::Payload::TopicInfo& topic_info, long long clock, eTracingLayerType layer, size_t payload_size, operation_type op_type);

        ~CSpan();

        CSpan(const CSpan&)            = delete;
        CSpan& operator=(const CSpan&) = delete;
        CSpan(CSpan&&)                 = delete;
        CSpan& operator=(CSpan&&)      = delete;

    private:
        SSpanData data;
    };

} // namespace tracing
} // namespace eCAL
