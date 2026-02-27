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
#include <csignal>
#include <cstdlib>
#include <atomic>

namespace eCAL
{ namespace tracing {

    // Helper function to get file path with PID
    std::string getSendSpansFilePath()
    {
        return std::string(std::getenv("HOME")) + "/workspace/eCAL-tracing-backend/data/ecal_publisher_spans_" + std::to_string(getpid()) + ".jsonl";
    }

    std::string getReceiveSpansFilePath()
    {
        return std::string(std::getenv("HOME")) + "/workspace/eCAL-tracing-backend/data/ecal_subscriber_spans_" + std::to_string(getpid()) + ".jsonl";
    }

    std::string getTopicMetadataFilePath()
    {
        return std::string(std::getenv("HOME")) + "/workspace/eCAL-tracing-backend/data/ecal_topic_metadata_" + std::to_string(getpid()) + ".jsonl";
    }

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
        
        CTraceProvider::getInstance().bufferSendSpan(data);
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
        CTraceProvider::getInstance().bufferReceiveSpan(data);
    }

    CShmHandshakeSpan::CShmHandshakeSpan(uint64_t entity_id, int32_t process_id, long long clock)
    {
        auto now = system_clock::now();
        data.start_ns = duration_cast<nanoseconds>(now.time_since_epoch()).count();
        data.entity_id = entity_id;
        data.process_id = process_id;
        data.clock = clock;
        data.layer = tl_ecal_shm;
        data.op_type = operation_type::shm_handshake;
        data.payload_size = 0;  // handshake doesn't transfer payload
    }

    CShmHandshakeSpan::~CShmHandshakeSpan()
    {
        auto now = system_clock::now();
        data.end_ns = duration_cast<nanoseconds>(now.time_since_epoch()).count();
        CTraceProvider::getInstance().bufferSendSpan(data);
    }

    // CTraceProvider implementation
    std::atomic<bool> CTraceProvider::flush_done_{false};

    CTraceProvider::CTraceProvider()
    {
        registerExitHandlers();
    }
    
    CTraceProvider::~CTraceProvider()
    {
        if (!flush_done_.exchange(true))
        {
            flushAllSpans();
        }
    }

    void CTraceProvider::registerExitHandlers()
    {
        // atexit runs before static destructors — first chance to flush
        std::atexit(atExitHandler);

        // Catch common termination / crash signals
        struct sigaction sa{};
        sa.sa_handler = signalHandler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_RESETHAND;  // restore default handler after first invocation

        sigaction(SIGINT,  &sa, nullptr);
        sigaction(SIGTERM, &sa, nullptr);
        sigaction(SIGABRT, &sa, nullptr);
        sigaction(SIGSEGV, &sa, nullptr);
        sigaction(SIGBUS,  &sa, nullptr);
        sigaction(SIGFPE,  &sa, nullptr);
    }

    void CTraceProvider::atExitHandler()
    {
        if (!flush_done_.exchange(true))
        {
            getInstance().flushAllSpans();
        }
    }

    void CTraceProvider::signalHandler(int signum)
    {
        // Best-effort flush — not fully async-signal-safe, but
        // maximises the chance of persisting buffered spans.
        if (!flush_done_.exchange(true))
        {
            getInstance().flushAllSpans();
        }

        // Re-raise with default handler so the OS generates the
        // expected exit status / core dump.
        signal(signum, SIG_DFL);
        raise(signum);
    }

    void CTraceProvider::bufferSendSpan(const SSendSpanData& span_data)
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

    void CTraceProvider::bufferReceiveSpan(const SReceiveSpanData& span_data)
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
        
        writeBatchSendSpans(batch_to_send);
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
        
        writeBatchReceiveSpans(batch_to_send);
    }

    void CTraceProvider::addTopicMetadata(const STopicMetadata& metadata)
    {
        writeTopicMetadata(metadata);
    }

    void CTraceProvider::flushAllSpans()
    {
        flushSendSpans();
        flushReceiveSpans();
    }

    void CTraceProvider::writeBatchSendSpans(const std::vector<SSendSpanData>& batch)
    {
        // Write send spans to JSONL file (one JSON object per line)
        try
        {
            std::string filepath = getSendSpansFilePath();
            
            // Open file in append mode
            std::ofstream output_file(filepath, std::ios::app);
            if (output_file.is_open())
            {
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
                    
                    // Write each span as a single line
                    output_file << span_obj.dump() << "\n";
                }
                output_file.close();
            }
            else
            {
                std::cerr << "Warning: Could not open send spans file: " << filepath << std::endl;
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error writing send spans to JSONL: " << e.what() << std::endl;
        }
    }

    void CTraceProvider::writeBatchReceiveSpans(const std::vector<SReceiveSpanData>& batch)
    {
        // Write receive spans to JSONL file (one JSON object per line)
        try
        {
            std::string filepath = getReceiveSpansFilePath();
            
            // Open file in append mode
            std::ofstream output_file(filepath, std::ios::app);
            if (output_file.is_open())
            {
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

                    // Write each span as a single line
                    output_file << span_obj.dump() << "\n";
                }
                output_file.close();
            }
            else
            {
                std::cerr << "Warning: Could not open receive spans file: " << filepath << std::endl;
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error writing receive spans to JSONL: " << e.what() << std::endl;
        }
    }

    void CTraceProvider::writeTopicMetadata(const STopicMetadata& metadata)
    {
        try
        {
            std::lock_guard<std::mutex> lock(metadata_mutex_);
            std::string filepath = getTopicMetadataFilePath();

            json obj;
            obj["entity_id"]   = metadata.entity_id;
            obj["process_id"]  = metadata.process_id;
            obj["host_name"]   = metadata.host_name;
            obj["topic_name"]  = metadata.topic_name;
            obj["encoding"]    = metadata.encoding;
            obj["type_name"]   = metadata.type_name;
            obj["direction"]   = (metadata.direction == topic_direction::publisher) ? "publisher" : "subscriber";

            // Open file in append mode and write as a single line
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