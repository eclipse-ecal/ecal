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
#include <ecal/ecal_process.h>

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <cstdio>
#include <thread>

constexpr std::chrono::milliseconds UDP_WAIT_TIME(1);

class CoutRedirect {
public:
    CoutRedirect(std::ostream& newStream) : originalBuffer(std::cout.rdbuf(newStream.rdbuf())) {}
    ~CoutRedirect() { std::cout.rdbuf(originalBuffer); }

    CoutRedirect(const CoutRedirect&) = delete;
    CoutRedirect& operator=(const CoutRedirect&) = delete;
    CoutRedirect(CoutRedirect&&) = delete;
    CoutRedirect& operator=(CoutRedirect&&) = delete;

private:
    std::streambuf* originalBuffer;
};

TEST(logging_to /*unused*/, file /*unused*/)
{
  const std::string logging_path = "./";
  const std::string unit_name    = "logging_to_file_test";
  const std::string log_message  = "Logging to file test.";
  auto& ecal_config              = eCAL::GetConfiguration();

  ecal_config.logging.sinks.udp.enable           = false;
  ecal_config.logging.sinks.console.enable       = false;
  ecal_config.logging.sinks.file.enable          = true;
  ecal_config.logging.sinks.file.path            = logging_path;
  ecal_config.logging.sinks.file.filter_log_file = log_level_all;

  eCAL::Initialize(ecal_config, unit_name.c_str(), eCAL::Init::Logging);

  eCAL::Logging::Log(log_level_info, log_message);

  eCAL::Finalize();

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
    
  logfile.close();

  if (!filepath.empty()) std::remove(filepath.c_str());
}

TEST(logging_to /*unused*/, udp /*unused*/)
{
  const std::string logging_path = "./";
  const std::string unit_name    = "logging_to_udp_test";
  const std::string log_message  = "Logging to udp test.";
  auto& ecal_config              = eCAL::GetConfiguration();

  ecal_config.logging.sinks.file.enable        = false;
  ecal_config.logging.sinks.console.enable     = false;
  ecal_config.logging.sinks.udp.enable         = true;
  ecal_config.logging.sinks.udp.filter_log_udp = log_level_all;

  eCAL::Initialize(ecal_config, unit_name.c_str(), eCAL::Init::Logging);  

  eCAL::Logging::Log(log_level_info, log_message);

  std::this_thread::sleep_for(UDP_WAIT_TIME);
  
  eCAL::Logging::SLogging log;
  eCAL::Logging::GetLogging(log);

  EXPECT_EQ(log.log_messages.size(), 1);
  
  // check whole log message for information
  // check before the size -> crashes the whole test if it's not checked before
  if (!log.log_messages.empty())
  {
    EXPECT_EQ(log.log_messages.front().hname, eCAL::Process::GetHostName());
    EXPECT_EQ(log.log_messages.front().pid,   eCAL::Process::GetProcessID());
    EXPECT_EQ(log.log_messages.front().uname, unit_name);
    EXPECT_EQ(log.log_messages.front().level, log_level_info);
    EXPECT_TRUE(log.log_messages.front().content.find(log_message) != std::string::npos);
  }

  eCAL::Finalize();
}

TEST(logging_to /*unused*/, console /*unused*/)
{
  const std::string logging_path = "./";
  const std::string unit_name    = "logging_to_console_test";
  const std::string log_message  = "Logging to console test.";
  auto& ecal_config              = eCAL::GetConfiguration();

  ecal_config.logging.sinks.file.enable            = false;
  ecal_config.logging.sinks.udp.enable             = false;
  ecal_config.logging.sinks.console.enable         = true;
  ecal_config.logging.sinks.console.filter_log_con = log_level_all;

  eCAL::Initialize(ecal_config, unit_name.c_str(), eCAL::Init::Logging);

  {
    // Redirect the output stream to a stringstream in order to find log messages
    std::stringstream ss;
    CoutRedirect redirect(ss);
    eCAL::Logging::Log(log_level_info, log_message);
    std::string console_output = ss.str();
    EXPECT_TRUE(console_output.find(log_message) != std::string::npos);
  }

  eCAL::Finalize();
}

int getLogging(eCAL::Logging::SLogging& log_)
{
  std::this_thread::sleep_for(UDP_WAIT_TIME);

  return eCAL::Logging::GetLogging(log_);
}

TEST(logging_levels /*unused*/, all /*unused*/)
{
  const std::string logging_path = "./";
  const std::string unit_name    = "logging_levels_all_udp";
  const std::string log_message  = "Logging level all test for udp.";
  auto& ecal_config              = eCAL::GetConfiguration();

  ecal_config.logging.sinks.file.enable        = false;
  ecal_config.logging.sinks.console.enable     = false;
  ecal_config.logging.sinks.udp.enable         = true;
  ecal_config.logging.sinks.udp.filter_log_udp = log_level_all;

  eCAL::Initialize(ecal_config, unit_name.c_str(), eCAL::Init::Logging);

  eCAL::Logging::SLogging log;
 
  eCAL::Logging::Log(log_level_info, log_message);
  EXPECT_EQ(getLogging(log), 1);

  eCAL::Logging::Log(log_level_warning, log_message);
  EXPECT_EQ(getLogging(log), 1);

  eCAL::Logging::Log(log_level_error, log_message);
  EXPECT_EQ(getLogging(log), 1);
 
  eCAL::Logging::Log(log_level_debug1, log_message);
  EXPECT_EQ(getLogging(log), 1);

  eCAL::Logging::Log(log_level_debug1, log_message);
  EXPECT_EQ(getLogging(log), 1);

  eCAL::Logging::Log(log_level_debug2, log_message);
  EXPECT_EQ(getLogging(log), 1);

  eCAL::Logging::Log(log_level_debug3, log_message);
  EXPECT_EQ(getLogging(log), 1);

  eCAL::Logging::Log(log_level_debug4, log_message);
  EXPECT_EQ(getLogging(log), 1);

  eCAL::Finalize();
}

TEST(logging_levels /*unused*/, various /*unused*/)
{
  const std::string logging_path = "./";
  const std::string unit_name    = "logging_levels_various_udp";
  const std::string log_message  = "Logging level various test for udp.";
  auto& ecal_config              = eCAL::GetConfiguration();

  ecal_config.logging.sinks.file.enable        = false;
  ecal_config.logging.sinks.console.enable     = false;
  ecal_config.logging.sinks.udp.enable         = true;
  ecal_config.logging.sinks.udp.filter_log_udp = log_level_warning;

  eCAL::Initialize(ecal_config, unit_name.c_str(), eCAL::Init::Logging);

  eCAL::Logging::SLogging log;
  
  // logging not expected
  eCAL::Logging::Log(log_level_info, log_message);
  EXPECT_EQ(getLogging(log), 0);

  // logging expected
  eCAL::Logging::Log(log_level_warning, log_message);
  EXPECT_EQ(getLogging(log), 1);

  // change log filter for udp logging
  eCAL::Logging::SetUDPLogFilter(log_level_info | log_level_debug1);

  // logging not expected
  eCAL::Logging::Log(log_level_warning, log_message);
  EXPECT_EQ(getLogging(log), 0);

  // logging expected
  eCAL::Logging::Log(log_level_info, log_message);
  EXPECT_EQ(getLogging(log), 1);
  
  eCAL::Logging::Log(log_level_debug1, log_message);
  EXPECT_EQ(getLogging(log), 1);

  eCAL::Finalize();
}

TEST(logging_levels /*unused*/, none /*unused*/)
{
  const std::string logging_path = "./";
  const std::string unit_name    = "logging_levels_various_udp";
  const std::string log_message  = "Logging level none test for udp.";
  auto& ecal_config              = eCAL::GetConfiguration();

  ecal_config.logging.sinks.file.enable        = false;
  ecal_config.logging.sinks.console.enable     = false;
  ecal_config.logging.sinks.udp.enable         = true;
  ecal_config.logging.sinks.udp.filter_log_udp = log_level_none;

  eCAL::Initialize(ecal_config, unit_name.c_str(), eCAL::Init::Logging);

  eCAL::Logging::SLogging log;
 
  eCAL::Logging::Log(log_level_info, log_message);
  EXPECT_EQ(getLogging(log), 0);

  eCAL::Logging::Log(log_level_warning, log_message);
  EXPECT_EQ(getLogging(log), 0);

  eCAL::Logging::Log(log_level_error, log_message);
  EXPECT_EQ(getLogging(log), 0);
 
  eCAL::Logging::Log(log_level_debug1, log_message);
  EXPECT_EQ(getLogging(log), 0);

  eCAL::Logging::Log(log_level_debug1, log_message);
  EXPECT_EQ(getLogging(log), 0);

  eCAL::Logging::Log(log_level_debug2, log_message);
  EXPECT_EQ(getLogging(log), 0);

  eCAL::Logging::Log(log_level_debug3, log_message);
  EXPECT_EQ(getLogging(log), 0);

  eCAL::Logging::Log(log_level_debug4, log_message);
  EXPECT_EQ(getLogging(log), 0);

  eCAL::Finalize();
}

TEST(logging_disable /*unused*/, file /*unused*/)
{
  const std::string logging_path = "./";
  const std::string unit_name    = "logging_disable_file_test";
  const std::string log_message  = "Disabled logging test for file.";
  auto& ecal_config              = eCAL::GetConfiguration();

  ecal_config.logging.sinks.udp.enable           = false;
  ecal_config.logging.sinks.console.enable       = false;
  ecal_config.logging.sinks.file.enable          = false;
  ecal_config.logging.sinks.file.path            = logging_path;
  ecal_config.logging.sinks.file.filter_log_file = log_level_all;

  eCAL::Initialize(ecal_config, unit_name.c_str(), eCAL::Init::Logging);

  eCAL::Logging::Log(log_level_info, log_message);

  eCAL::Finalize();

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

  EXPECT_EQ(filepath, "");
}

TEST(logging_disable /*unused*/, udp /*unused*/)
{
  const std::string unit_name    = "logging_dsiable_udp_test";
  const std::string log_message  = "Disabled logging test for udp.";
  auto& ecal_config              = eCAL::GetConfiguration();

  ecal_config.logging.sinks.file.enable        = false;
  ecal_config.logging.sinks.console.enable     = false;
  ecal_config.logging.sinks.udp.enable         = false;
  ecal_config.logging.sinks.udp.filter_log_udp = log_level_all;

  eCAL::Initialize(ecal_config, unit_name.c_str(), eCAL::Init::Logging);  

  eCAL::Logging::Log(log_level_info, log_message);

  std::this_thread::sleep_for(UDP_WAIT_TIME);
  
  eCAL::Logging::SLogging log;
  eCAL::Logging::GetLogging(log);

  EXPECT_EQ(log.log_messages.size(), 0);
  
  eCAL::Finalize();
}

TEST(logging_disable /*unused*/, console /*unused*/)
{
  const std::string unit_name    = "logging_disable_console_test";
  const std::string log_message  = "Disabled logging test for console.";
  auto& ecal_config              = eCAL::GetConfiguration();

  ecal_config.logging.sinks.file.enable            = false;
  ecal_config.logging.sinks.udp.enable             = false;
  ecal_config.logging.sinks.console.enable         = false;
  ecal_config.logging.sinks.console.filter_log_con = log_level_all;

  eCAL::Initialize(ecal_config, unit_name.c_str(), eCAL::Init::Logging);

  {
    // Redirect the output stream to a stringstream in order to find log messages
    std::stringstream ss;
    CoutRedirect redirect(ss);
    eCAL::Logging::Log(log_level_info, log_message);
    std::string console_output = ss.str();
    EXPECT_TRUE(console_output.find(log_message) == std::string::npos);
  }

  eCAL::Finalize();
}