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
        entity_id = topic_id.topic_id.entity_id;
        process_id = topic_id.topic_id.process_id;
        this->payload_size = payload_size;
        this->clock = clock;
        this->layer = layer;
    }

    CSendSpan::~CSendSpan()
    {
        clock_gettime(CLOCK_MONOTONIC, &end_time);
        CTraceProvider::getInstance().getSendSpans().push_back(*this);
    }

    CReceiveSpan::CReceiveSpan(const eCAL::Payload::TopicInfo topic_info, long long clock, eTLayerType layer)
    {
        clock_gettime(CLOCK_MONOTONIC, &start_time);
        entity_id = topic_info.topic_id;
        process_id = topic_info.process_id;
        this->clock = clock;
        this->layer = layer;
    }

    CReceiveSpan::~CReceiveSpan()
    {
        clock_gettime(CLOCK_MONOTONIC, &end_time);
        CTraceProvider::getInstance().getReceiveSpans().push_back(*this);
    }

} // namespace tracing

} // namespace eCAL