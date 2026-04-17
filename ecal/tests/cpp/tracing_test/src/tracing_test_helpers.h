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

#include <gtest/gtest.h>

#include <ecal/ecal.h>
#include <ecal/pubsub/publisher.h>
#include <ecal/pubsub/subscriber.h>

#include <tracing/tracing.h>
#include <tracing/trace_provider.h>
#include <tracing/span.h>
#include <tracing/tracing_writer.h>

#include <nlohmann/json.hpp>

#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include <ecal/process.h>
#include <ecal_global_accessors.h>

#ifdef _WIN32
#include <cstdlib>
inline int setenv(const char* name, const char* value, int /*overwrite*/)
{
    return _putenv_s(name, value);
}
#endif

using json = nlohmann::json;

// Return a platform-appropriate temporary directory.
inline std::string tempDir()
{
#ifdef _WIN32
    const char* dir = std::getenv("TEMP");
    if (dir) return dir;
    dir = std::getenv("TMP");
    if (dir) return dir;
    return "C:\\Temp";
#else
    return "/tmp";
#endif
}

// Remove a file if it exists.
inline void removeFile(const std::string& path)
{
    std::remove(path.c_str());
}

// Read a JSONL file and return a vector of parsed JSON objects.
inline std::vector<json> readJsonLines(const std::string& path)
{
    std::vector<json> result;
    std::ifstream file(path);
    std::string line;
    while (std::getline(file, line))
    {
        if (!line.empty())
        {
            try { result.push_back(json::parse(line)); }
            catch (...) {}
        }
    }
    return result;
}

// Convenience accessors for the provider's output file paths.
inline std::string spansFilePath()
{
    auto p = eCAL::g_trace_provider();
    if (p) return p->getSpansFilePath();
    return {};
}

inline std::string metadataFilePath()
{
    auto p = eCAL::g_trace_provider();
    if (p) return p->getTopicMetadataFilePath();
    return {};
}

// Wait for the provider's background writer thread to drain the span buffer.
inline bool waitForBufferDrain(eCAL::tracing::CTraceProvider* provider, int timeout_ms = 500)
{
    auto deadline = std::chrono::steady_clock::now()
                  + std::chrono::milliseconds(timeout_ms);
    while (!provider->getSpans().empty()
           && std::chrono::steady_clock::now() < deadline)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    return provider->getSpans().empty();
}

// Force the provider to synchronously flush all buffered spans.
// Intended for setUp/tearDown cleanup.
inline void triggerFlush(eCAL::tracing::CTraceProvider* provider)
{
    provider->forceFlush();
}
