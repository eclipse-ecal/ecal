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

#include <ecal/config/tracing.h>

#include <memory>

namespace eCAL
{
namespace tracing
{

  class TraceProvider
  {
  public:
    TraceProvider()                               = default;
    virtual ~TraceProvider()                      = default;

    TraceProvider(const TraceProvider&)            = delete;
    TraceProvider& operator=(const TraceProvider&) = delete;
    TraceProvider(TraceProvider&&)                 = delete;
    TraceProvider& operator=(TraceProvider&&)      = delete;

    static std::shared_ptr<TraceProvider> Create(const eCAL::Tracing::Configuration& config_);

    virtual void WriteSpan(const SpanDataVariant& span_data) = 0;
    virtual void WriteMetadata(const STopicMetadata& metadata) = 0;
  };

} // namespace tracing
} // namespace eCAL
