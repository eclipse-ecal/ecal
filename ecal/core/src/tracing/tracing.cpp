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
#include <fstream>
#include <filesystem>
#include <iostream>
#include <unistd.h>

namespace eCAL
{ namespace tracing {

    // Helper function to get file path with PID
    std::string getSendSpansFilePath()
    {
        return std::string(std::getenv("HOME")) + "/workspace/eCAL-tracing-backend/data/ecal_publisher_spans_" + std::to_string(getpid()) + ".json";
    }

    std::string getReceiveSpansFilePath()
    {
        return std::string(std::getenv("HOME")) + "/workspace/eCAL-tracing-backend/data/ecal_subscriber_spans_" + std::to_string(getpid()) + ".json";
    }

    // Fixed file locations for trace data, to be changed
    const std::string SEND_SPANS_FILE = std::string(std::getenv("HOME")) + "/workspace/eCAL-tracing-backend/data/ecal_publisher_spans.json";
    const std::string RECEIVE_SPANS_FILE = std::string(std::getenv("HOME")) + "/workspace/eCAL-tracing-backend/data/ecal_subscriber_spans.json";

    CSendSpan::CSendSpan(const STopicId topic_id, long long clock, eTLayerType layer, size_t payload_size, operation_type op_type)
    {
        auto now = system_clock::now();
        data.start_ns = duration_cast<nanoseconds>(now.time_since_epoch()).count();
        data.entity_id = topic_id.topic_id.entity_id;
        data.process_id = topic_id.topic_id.process_id;
        data.payload_size = payload_size;
        data.clock = clock;
        data.layer = layer;
        data.op_type = op_type;
    }

    CSendSpan::~CSendSpan()
    {
        auto now = system_clock::now();
        data.end_ns = duration_cast<nanoseconds>(now.time_since_epoch()).count();
        
        CTraceProvider::getInstance().addSendSpan(data);
    }

    CReceiveSpan::CReceiveSpan(EntityIdT entity_id, const eCAL::Payload::TopicInfo topic_info, long long clock, eTLayerType layer, operation_type op_type)
    {
        auto now = system_clock::now();
        data.start_ns = duration_cast<nanoseconds>(now.time_since_epoch()).count();
        data.entity_id = entity_id;
        data.topic_id = topic_info.topic_id;
        data.process_id = topic_info.process_id;
        data.clock = clock;
        data.layer = layer;
        data.op_type = op_type;
    }

    CReceiveSpan::~CReceiveSpan()
    {
        auto now = system_clock::now();
        data.end_ns = duration_cast<nanoseconds>(now.time_since_epoch()).count();  
        CTraceProvider::getInstance().addReceiveSpan(data);
    }

    // CTraceProvider implementation
    CTraceProvider::CTraceProvider() = default;
    
    CTraceProvider::~CTraceProvider() = default;

    void CTraceProvider::addSendSpan(const SSendSpanData& span_data)
    {
        {
            std::lock_guard<std::mutex> lock(buffer_mutex_);
            send_span_buffer_.push_back(span_data);
        }
        
        // Auto-flush if batch size reached
        if (send_span_buffer_.size() >= send_batch_size_)
        {
            flushSendSpans();
        }
    }

    void CTraceProvider::addReceiveSpan(const SReceiveSpanData& span_data)
    {
        {
            std::lock_guard<std::mutex> lock(buffer_mutex_);
            receive_span_buffer_.push_back(span_data);
        }
        
        // Auto-flush if batch size reached
        if (receive_span_buffer_.size() >= receive_batch_size_)
        {
            flushReceiveSpans();
        }
    }

    void CTraceProvider::flushSendSpans()
    {
        std::vector<SSendSpanData> batch_to_send;
        {
            std::lock_guard<std::mutex> lock(buffer_mutex_);
            if (send_span_buffer_.empty())
                return;
            
            batch_to_send.swap(send_span_buffer_);
        }
        
        sendBatchSendSpans(batch_to_send);
    }

    void CTraceProvider::flushReceiveSpans()
    {
        std::vector<SReceiveSpanData> batch_to_send;
        {
            std::lock_guard<std::mutex> lock(buffer_mutex_);
            if (receive_span_buffer_.empty())
                return;
            
            batch_to_send.swap(receive_span_buffer_);
        }
        
        sendBatchReceiveSpans(batch_to_send);
    }

    void CTraceProvider::flushAllSpans()
    {
        flushSendSpans();
        flushReceiveSpans();
    }

    void CTraceProvider::sendBatchSendSpans(const std::vector<SSendSpanData>& batch)
    {
        // Write send spans to JSON file
        try
        {
            std::string filepath = getSendSpansFilePath();
            json json_array = json::array();
            
            for (const auto& span : batch)
            {
                json span_obj;
                span_obj["entity_id"] = span.entity_id;
                span_obj["process_id"] = span.process_id;
                span_obj["clock"] = span.clock;
                span_obj["layer"] = span.layer;
                span_obj["payload_size"] = span.payload_size;
                span_obj["start_ns"] = span.start_ns;
                span_obj["end_ns"] = span.end_ns;
                span_obj["op_type"] = span.op_type;
                
                json_array.push_back(span_obj);
            }
            
            // Read existing data if file exists
            json all_data = json::array();
            if (std::filesystem::exists(filepath))
            {
                std::ifstream input_file(filepath);
                if (input_file.is_open())
                {
                    input_file >> all_data;
                    input_file.close();
                }
            }
            
            // Append new spans
            for (const auto& span : json_array)
            {
                all_data.push_back(span);
            }
            
            // Write back to file
            std::ofstream output_file(filepath);
            if (output_file.is_open())
            {
                output_file << all_data.dump(2) << std::endl;
                output_file.close();
            }
            else
            {
                std::cerr << "Warning: Could not open send spans file: " << filepath << std::endl;
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error writing send spans to JSON: " << e.what() << std::endl;
        }
    }

    void CTraceProvider::sendBatchReceiveSpans(const std::vector<SReceiveSpanData>& batch)
    {
        // Write receive spans to JSON file
        try
        {
            std::string filepath = getReceiveSpansFilePath();
            json json_array = json::array();
            
            for (const auto& span : batch)
            {
                json span_obj;
                span_obj["entity_id"] = span.entity_id;
                span_obj["topic_id"] = span.topic_id;
                span_obj["process_id"] = span.process_id;
                span_obj["clock"] = span.clock;
                span_obj["layer"] = span.layer;
                span_obj["start_ns"] = span.start_ns;
                span_obj["end_ns"] = span.end_ns;
                span_obj["op_type"] = span.op_type;

                json_array.push_back(span_obj);
            }
            
            // Read existing data if file exists
            json all_data = json::array();
            if (std::filesystem::exists(filepath))
            {
                std::ifstream input_file(filepath);
                if (input_file.is_open())
                {
                    input_file >> all_data;
                    input_file.close();
                }
            }
            
            // Append new spans
            for (const auto& span : json_array)
            {
                all_data.push_back(span);
            }
            
            // Write back to file
            std::ofstream output_file(filepath);
            if (output_file.is_open())
            {
                output_file << all_data.dump(2) << std::endl;
                output_file.close();
            }
            else
            {
                std::cerr << "Warning: Could not open receive spans file: " << filepath << std::endl;
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error writing receive spans to JSON: " << e.what() << std::endl;
        }
    }

} // namespace tracing

} // namespace eCAL