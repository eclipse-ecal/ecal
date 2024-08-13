/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2024 Continental Corporation
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

#include <ecal/ecal_core.h>
#include <ecal/ecal_config.h>
#include <ecal/ecal_log.h>
#include <ecal/ecal_log_level.h>

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <cstdio>

TEST(logging_sinks /*unused*/, file_logging /*unused*/)
{
  const std::string logging_path = "./";
  const std::string unit_name = "logging_test";
  const std::string log_message = "Logging test for file.";
  auto& ecal_config = eCAL::GetConfiguration();

  ecal_config.logging.sinks.file.enable = true;
  ecal_config.logging.sinks.file.path = logging_path;
  ecal_config.logging.sinks.file.filter_log_file = log_level_all;

  eCAL::Initialize(ecal_config, unit_name.c_str(), eCAL::Init::Logging);

  eCAL::Logging::Log(log_message);

  std::string filepath;
  for (const auto& entry : std::filesystem::directory_iterator(logging_path))
  {
    if (entry.is_regular_file())
    {
      if (entry.path().string().find(unit_name) != std::string::npos)
      {
        filepath = entry.path().string();
      }
    }
  }

  EXPECT_NE(filepath, "");

  std::ifstream logfile(filepath);
  
  std::string line;
  std::getline(logfile, line);
  EXPECT_NE(line, "");
  
  auto find_message = line.find(log_message);
  EXPECT_NE(find_message, std::string::npos);
    
  eCAL::Finalize();
  logfile.close();

  if (!filepath.empty()) std::remove(filepath.c_str());
}