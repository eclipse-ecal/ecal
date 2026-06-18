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

#include "tracing_writer_jsonl.h"

#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <type_traits>

#include <ecal/process.h>
#include <ecal/util.h>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace {

  template<class... Ts>
  struct Overloaded : Ts... {
    using Ts::operator()...;
  };

  template<class... Ts>
  Overloaded(Ts...) -> Overloaded<Ts...>;


  using namespace eCAL::tracing;

  json toJson(const STopicMetadata& metadata)
  {
    json metadata_as_json;
    metadata_as_json["tracing_version"] = metadata.tracing_version;
    metadata_as_json["entity_id"] = metadata.entity_id;
    metadata_as_json["process_id"] = metadata.process_id;
    metadata_as_json["host_name"] = metadata.host_name;
    metadata_as_json["topic_name"] = metadata.topic_name;
    metadata_as_json["encoding"] = metadata.encoding;
    metadata_as_json["type_name"] = metadata.type_name;
    metadata_as_json["direction"] = (metadata.direction == topic_direction::publisher) ? "publisher" : "subscriber";
    return metadata_as_json;
  }

  json toJson(const SPublisherSpanData& span)
  {
    json span_as_json;
    span_as_json["op_type"]      = static_cast<int>(span.op_type);
    span_as_json["entity_id"]    = span.entity_id;
    span_as_json["process_id"]   = span.process_id;
    span_as_json["payload_size"] = span.payload_size;
    span_as_json["clock"]        = span.clock;
    span_as_json["layer"]        = span.layer;
    span_as_json["start_ns"]     = span.start_ns;
    span_as_json["end_ns"]       = span.end_ns;
    return span_as_json;
  }

  json toJson(const SSubscriberSpanData& span)
  {
    json span_as_json;
    span_as_json["op_type"]      = static_cast<int>(span.op_type);
    span_as_json["entity_id"]    = span.entity_id;
    span_as_json["topic_id"]     = span.topic_id;
    span_as_json["process_id"]   = span.process_id;
    span_as_json["payload_size"] = span.payload_size;
    span_as_json["clock"]        = span.clock;
    span_as_json["layer"]        = span.layer;
    span_as_json["start_ns"]     = span.start_ns;
    span_as_json["end_ns"]       = span.end_ns;
    return span_as_json;
  }

  std::string GetCurrentTimestamp()
  {
    std::time_t now = std::time(nullptr);
    std::tm* tm_info = std::localtime(&now);
    std::ostringstream oss;
    oss << std::put_time(tm_info, "%Y%m%d_%H%M%S");
    return oss.str();
  }

  static int unique_cnt = 0;
}

namespace eCAL
{
  namespace tracing
  {
    CTracingWriterJSONL::CTracingWriterJSONL()
      : CTracingWriterJSONL(std::to_string(eCAL::Process::GetProcessID()) + "_" + std::to_string(++unique_cnt) + "_" + GetCurrentTimestamp(), std::filesystem::path(Util::GeteCALTraceDir()))
    {
    }


    CTracingWriterJSONL::CTracingWriterJSONL(const std::string& file_id, std::filesystem::path trace_directory)
      : spans_file_path_(trace_directory / (std::string("ecal_spans_") + file_id + ".jsonl"))
      , metadata_file_path_(trace_directory / (std::string("ecal_metadata_") + file_id + ".jsonl"))
      , spans_file_(spans_file_path_, std::ios::out | std::ios::trunc)
      , metadata_file_(metadata_file_path_, std::ios::out | std::ios::trunc)
    {
    }

    std::filesystem::path CTracingWriterJSONL::GetSpansFilePath() const
    {
      return spans_file_path_;
    }

    std::filesystem::path CTracingWriterJSONL::GetTopicMetadataFilePath() const
    {
      return metadata_file_path_;
    }


    void CTracingWriterJSONL::WriteTraceInfo(const std::vector<TraceInfo>& batch)
    {
      for (const auto& span_variant : batch)
      {
        std::visit(
          Overloaded{
            [this](const SpanData& span) { WriteSpanData(span); },
            [this](const STopicMetadata& metadata) { WriteTopicMetadata(metadata); }
          },
          span_variant
        );
      }
    }

    void CTracingWriterJSONL::WriteSpanData(const SpanData& span_data)
    {
      try
      {
        if (spans_file_.is_open())
        {
          auto span_obj = std::visit([](const auto& span) { return toJson(span); }, span_data);
          spans_file_ << span_obj.dump() << "\n";
        }
      }
      catch (const std::exception& e)
      {
        std::cerr << "Error writing spans to JSONL: " << e.what() << std::endl;
      }
    }

    void CTracingWriterJSONL::WriteTopicMetadata(const STopicMetadata& metadata)
    {
      try
      {
        if (metadata_file_.is_open())
        {
          auto metadata_json = toJson(metadata);
          metadata_file_ << metadata_json.dump() << "\n";
        }
      }
      catch (const std::exception& e)
      {
        std::cerr << "Error writing metadata to JSONL: " << e.what() << std::endl;
      }
    }
  }
}
