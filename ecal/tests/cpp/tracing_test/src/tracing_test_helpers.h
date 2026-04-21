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

#include <tracing/tracing.h>
#include <tracing/tracing_writer.h>

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include <cstdlib>
#include <fstream>
#include <mutex>
#include <string>
#include <vector>

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

// Cross-platform helpers for environment variable manipulation.
inline void SetEnv(const std::string& name, const std::string& value)
{
#ifdef _WIN32
  _putenv_s(name.c_str(), value.c_str());
#else
  setenv(name.c_str(), value.c_str(), 1);
#endif
}

inline void UnsetEnv(const std::string& name)
{
#ifdef _WIN32
  _putenv_s(name.c_str(), "");
#else
  unsetenv(name.c_str());
#endif
}

// RAII helper to set and restore an environment variable.
class ScopedEnvVar
{
public:
  ScopedEnvVar(const char* name, const std::string& value)
    : name_(name)
  {
    const char* old = std::getenv(name);
    had_old_ = (old != nullptr);
    if (had_old_) old_value_ = old;
    SetEnv(name_, value);
  }

  ~ScopedEnvVar()
  {
    if (had_old_)
      SetEnv(name_, old_value_);
    else
      UnsetEnv(name_);
  }

  ScopedEnvVar(const ScopedEnvVar&)            = delete;
  ScopedEnvVar& operator=(const ScopedEnvVar&) = delete;

private:
  std::string name_;
  std::string old_value_;
  bool        had_old_{false};
};

