/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
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

#pragma once

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>
#include <string>

class ScopedTestDirectory
{
public:
  explicit ScopedTestDirectory(const std::string& name)
    : path_(std::filesystem::temp_directory_path() / name)
  {
    std::filesystem::remove_all(path_);
    std::filesystem::create_directories(path_);
  }

  ~ScopedTestDirectory()
  {
    std::filesystem::remove_all(path_);
  }

  const std::filesystem::path& Path() const
  {
    return path_;
  }

private:
  std::filesystem::path path_;
};

inline size_t CountAndValidateJsonlLines(const std::filesystem::path& filepath)
{
  std::ifstream file(filepath);
  EXPECT_TRUE(file.is_open()) << "Failed to open: " << filepath;

  size_t count = 0;
  std::string line;
  while (std::getline(file, line))
  {
    if (line.empty()) continue;
    EXPECT_NO_THROW(nlohmann::json::parse(line))
      << "Invalid JSON on line " << (count + 1) << ": " << line;
    ++count;
  }
  return count;
}