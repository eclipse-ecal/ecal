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

#include "tracing_writer.h"

#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdlib>
#include <ctime>

#include <ecal/process.h>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace eCAL
{
namespace tracing
{

    static std::string getCurrentTimestamp()
    {
        std::time_t now = std::time(nullptr);
        std::tm* tm_info = std::localtime(&now);
        std::ostringstream oss;
        oss << std::put_time(tm_info, "%Y%m%d_%H%M%S");
        return oss.str();
    }

    CTracingWriter::CTracingWriter()
        : timestamp_(getCurrentTimestamp())
    {}

    std::string CTracingWriter::getSpansFilePath() const
    {
        const char* data_dir = std::getenv("ECAL_TRACING_DATA_DIR");
        if (data_dir == nullptr)
        {
            std::cerr << "Fatal: Mandatory environment variable ECAL_TRACING_DATA_DIR is not set." << std::endl;
            std::abort();
        }
        return std::string(data_dir) + "/ecal_spans_" + std::to_string(eCAL::Process::GetProcessID()) + "_" + timestamp_ + ".jsonl";
    }

    std::string CTracingWriter::getTopicMetadataFilePath() const
    {
        const char* data_dir = std::getenv("ECAL_TRACING_DATA_DIR");
        if (data_dir == nullptr)
        {
            std::cerr << "Fatal: Mandatory environment variable ECAL_TRACING_DATA_DIR is not set." << std::endl;
            std::abort();
        }
        return std::string(data_dir) + "/ecal_topic_metadata_" + std::to_string(eCAL::Process::GetProcessID()) + "_" + timestamp_ + ".jsonl";
    }

    void CTracingWriter::writeBatchSpans(const std::vector<SSpanData>& batch)
    {
        try
        {
            std::lock_guard<std::mutex> lock(spans_mutex_);
            std::string filepath = getSpansFilePath();

            std::ofstream output_file(filepath, std::ios::app);
            if (output_file.is_open())
            {
                for (const auto& span : batch)
                {
                    json span_obj;
                    span_obj["entity_id"]    = span.entity_id;
                    span_obj["topic_id"]     = span.topic_id;
                    span_obj["process_id"]   = span.process_id;
                    span_obj["payload_size"] = span.payload_size;
                    span_obj["clock"]        = span.clock;
                    span_obj["layer"]        = span.layer;
                    span_obj["start_ns"]     = span.start_ns;
                    span_obj["end_ns"]       = span.end_ns;
                    span_obj["op_type"]      = span.op_type;

                    output_file << span_obj.dump() << "\n";
                }
                output_file.close();
            }
            else
            {
                std::cerr << "Warning: Could not open spans file: " << filepath << std::endl;
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error writing spans to JSONL: " << e.what() << std::endl;
        }
    }

    void CTracingWriter::writeTopicMetadata(const STopicMetadata& metadata)
    {
        try
        {
            std::lock_guard<std::mutex> lock(metadata_mutex_);
            std::string filepath = getTopicMetadataFilePath();

            json obj;
            obj["tracing_version"] = metadata.tracing_version;
            obj["entity_id"]   = metadata.entity_id;
            obj["process_id"]  = metadata.process_id;
            obj["host_name"]   = metadata.host_name;
            obj["topic_name"]  = metadata.topic_name;
            obj["encoding"]    = metadata.encoding;
            obj["type_name"]   = metadata.type_name;
            obj["direction"]   = (metadata.direction == topic_direction::publisher) ? "publisher" : "subscriber";

            std::ofstream output_file(filepath, std::ios::app);
            if (output_file.is_open())
            {
                output_file << obj.dump() << "\n";
                output_file.close();
            }
            else
            {
                std::cerr << "Warning: Could not open topic metadata file: " << filepath << std::endl;
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error writing topic metadata to JSONL: " << e.what() << std::endl;
        }
    }

} // namespace tracing
} // namespace eCAL
