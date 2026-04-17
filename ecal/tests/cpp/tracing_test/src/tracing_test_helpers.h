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

#include <ecal/core.h>

#include <tracing/tracing.h>
#include <tracing/tracing_writer.h>

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include <atomic>
#include <cstdlib>
#include <fstream>
#include <mutex>
#include <string>
#include <vector>

class ScopedTraceDirOverride
{
public:
    explicit ScopedTraceDirOverride(const std::string& path)
    {
        if (const char* previous_value = std::getenv(kTraceDirEnvVar))
        {
            had_previous_value_ = true;
            previous_value_ = previous_value;
        }

#ifdef _WIN32
        _putenv_s(kTraceDirEnvVar, path.c_str());
#else
        setenv(kTraceDirEnvVar, path.c_str(), 1);
#endif
    }

    ~ScopedTraceDirOverride()
    {
#ifdef _WIN32
        _putenv_s(kTraceDirEnvVar, had_previous_value_ ? previous_value_.c_str() : "");
#else
        if (had_previous_value_)
            setenv(kTraceDirEnvVar, previous_value_.c_str(), 1);
        else
            unsetenv(kTraceDirEnvVar);
#endif
    }

private:
    static constexpr const char* kTraceDirEnvVar = "ECAL_TRACE_DIR";

    bool had_previous_value_{false};
    std::string previous_value_;
};

inline eCAL::Configuration GetTracingConfiguration()
{
    return eCAL::Configuration{};
}

// Mock writer that counts spans and metadata for test assertions.
class MockTracingWriter : public eCAL::tracing::TracingWriter
{
public:
    void WriteSpansToFile(const std::vector<eCAL::tracing::SpanDataVariant>& batch) override
    {
        span_count_ += batch.size();
    }

    void WriteMetadataToFile(const eCAL::tracing::STopicMetadata& /*metadata*/) override
    {
        metadata_count_++;
    }

    size_t SpanCount() const
    {
        return span_count_.load();
    }

    size_t MetadataCount() const
    {
        return metadata_count_.load();
    }

    void Clear()
    {
        span_count_       = 0;
        metadata_count_   = 0;

    }

private:
    mutable std::mutex mutex_;
    std::atomic<size_t> span_count_{0};
    std::atomic<size_t> metadata_count_{0};
};

// This allows the mock to outlive the CTraceProvider that only owns this proxy.
class ProxyTracingWriter : public eCAL::tracing::TracingWriter
{
public:
    explicit ProxyTracingWriter(MockTracingWriter& target) : target_(target) {}

    void WriteSpansToFile(const std::vector<eCAL::tracing::SpanDataVariant>& batch) override
    {
        target_.WriteSpansToFile(batch);
    }

    void WriteMetadataToFile(const eCAL::tracing::STopicMetadata& metadata) override
    {
        target_.WriteMetadataToFile(metadata);
    }

private:
    MockTracingWriter& target_;
};

// Count the number of lines in a file and validate each line is valid JSON.
inline size_t CountAndValidateJsonlLines(const std::string& filepath)
{
    std::ifstream file(filepath);
    EXPECT_TRUE(file.is_open()) << "Failed to open: " << filepath;

    size_t count = 0;
    std::string line;
    while (std::getline(file, line))
    {
      if (line.empty()) continue;
      // Every line must be valid JSON — a corrupted/interleaved write would break parsing.
      EXPECT_NO_THROW(nlohmann::json::parse(line))
          << "Invalid JSON on line " << (count + 1) << ": " << line;
      ++count;
    }
    return count;
}

