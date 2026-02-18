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
#include <list>
#include <ecal/types.h>
#include <serialization/ecal_struct_sample_common.h>
#include <serialization/ecal_struct_sample_payload.h>
#include <ecal/ecal.h>
#include <time.h>

namespace eCAL
{
namespace tracing
{

    class CSendSpan {
    public:
        CSendSpan(const STopicId topic_id, long long clock, eTLayerType layer, size_t payload_size);
        ~CSendSpan();

    private:
        uint64_t entity_id;
        uint64_t process_id;
        size_t payload_size;
        long long clock;
        uint64_t layer;
        timespec start_time;
        timespec end_time;
  };

  class CReceiveSpan {
    public:
        CReceiveSpan(const eCAL::Payload::TopicInfo topic_info, long long clock, eTLayerType layer);
        ~CReceiveSpan();

    private:
        uint64_t entity_id;
        uint64_t process_id;
        long long clock;
        uint64_t layer;
        timespec start_time;
        timespec end_time;
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

        std::list<CSendSpan>& getSendSpans() { return send_spans; }
        std::list<CReceiveSpan>& getReceiveSpans() { return receive_spans; }

    private:
        CTraceProvider() = default;
        ~CTraceProvider() = default;

        std::list<CSendSpan> send_spans;
        std::list<CReceiveSpan> receive_spans;
  };

} // namespace tracing
} // namespace eCAL
