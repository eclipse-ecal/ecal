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

#include "span.h"
#include "trace_provider.h"
#include "../ecal_global_accessors.h"

#include <chrono>

using namespace std::chrono;

namespace eCAL
{
namespace tracing
{

    // Send span constructor
    CSpan::CSpan(const STopicId& topic_id, long long clock, eTracingLayerType layer, size_t payload_size, operation_type op_type)
    {
        auto now = system_clock::now();
        data.start_ns     = duration_cast<nanoseconds>(now.time_since_epoch()).count();
        data.entity_id    = topic_id.topic_id.entity_id;
        data.process_id   = topic_id.topic_id.process_id;
        data.payload_size = payload_size;
        data.clock        = clock;
        data.layer        = layer;
        data.op_type      = op_type;
    }

    // Receive span constructor
    CSpan::CSpan(EntityIdT entity_id, const eCAL::Payload::TopicInfo& topic_info, long long clock, eTracingLayerType layer, size_t payload_size, operation_type op_type)
    {
        auto now = system_clock::now();
        data.start_ns   = duration_cast<nanoseconds>(now.time_since_epoch()).count();
        data.entity_id  = entity_id;
        data.topic_id   = topic_info.topic_id;
        data.process_id = topic_info.process_id;
        data.payload_size = payload_size;
        data.clock      = clock;
        data.layer      = layer;
        data.op_type    = op_type;
    }

    CSpan::~CSpan()
    {
        auto now = system_clock::now();
        data.end_ns = duration_cast<nanoseconds>(now.time_since_epoch()).count();
        if (auto provider = g_trace_provider(); provider) provider->bufferSpan(data);
    }

} // namespace tracing
} // namespace eCAL
