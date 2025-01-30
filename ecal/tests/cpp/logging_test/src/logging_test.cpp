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

#include <chrono>
#include <ecal/config.h>
#include <ecal/core.h>
#include <ecal/log.h>
#include <ecal/log_level.h>
#include <ecal/process.h>

#include <gtest/gtest.h>

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ostream>
#include <streambuf>
#include <string>
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

eCAL::Configuration GetUDPConfiguration()
{
  eCAL::Configuration config;
  config.logging.provider.file.enable     = false;
  config.logging.provider.console.enable  = false;
  config.logging.provider.udp.enable      = true;
  config.logging.receiver.enable          = true;
  config.logging.provider.udp.filter_log  = eCAL::Logging::log_level_all;
  return config;
}

eCAL::Configuration GetFileConfiguration(const std::string& path_)
{
  eCAL::Configuration config;
  config.logging.provider.udp.enable       = false;
  config.logging.provider.console.enable   = false;
  config.logging.provider.file.enable      = true;
  config.logging.provider.file_config.path = path_;
  config.logging.provider.file.filter_log  = eCAL::Logging::log_level_all;
  return config;
}

eCAL::Configuration GetConsoleConfiguration()
{
  eCAL::Configuration config;
  config.logging.provider.file.enable        = false;
  config.logging.provider.udp.enable         = false;
  config.logging.provider.console.enable     = true;
  config.logging.provider.console.filter_log = eCAL::Logging::log_level_all;
  return config;
}

TEST(logging_to /*unused*/, file /*unused*/)
{
  const std::string logging_path = "./";
  const std::string unit_name    = "logging_to_file_test";
  const std::string log_message  = "Logging to file test.";
  auto  ecal_config              = GetFileConfiguration(logging_path);

  eCAL::Initialize(ecal_config, unit_name, eCAL::Init::Logging);

  eCAL::Logging::Log(eCAL::Logging::log_level_info, log_message);

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
  const std::string unit_name    = "logging_to_udp_test";
  const std::string log_message  = "Logging to udp test.";
  auto  ecal_config              = GetUDPConfiguration();

  eCAL::Initialize(ecal_config, unit_name, eCAL::Init::Logging);  

  eCAL::Logging::Log(eCAL::Logging::eLogLevel::log_level_info, log_message);

  std::this_thread::sleep_for(UDP_WAIT_TIME);
  
  eCAL::Logging::SLogging log;
  eCAL::Logging::GetLogging(log);

  EXPECT_EQ(log.log_messages.size(), 1);
  
  // check whole log message for information
  // check before the size -> crashes the whole test if it's not checked before
  if (!log.log_messages.empty())
  {
    EXPECT_EQ(log.log_messages.front().host_name,  eCAL::Process::GetHostName());
    EXPECT_EQ(log.log_messages.front().process_id, eCAL::Process::GetProcessID());
    EXPECT_EQ(log.log_messages.front().unit_name,  unit_name);
    EXPECT_EQ(log.log_messages.front().level,      eCAL::Logging::eLogLevel::log_level_info);
    EXPECT_TRUE(log.log_messages.front().content.find(log_message) != std::string::npos);
  }

  eCAL::Finalize();
}

TEST(logging_to /*unused*/, console /*unused*/)
{
  const std::string unit_name    = "logging_to_console_test";
  const std::string log_message  = "Logging to console test.";
  auto  ecal_config              = GetConsoleConfiguration();

  eCAL::Initialize(ecal_config, unit_name, eCAL::Init::Logging);

  {
    // Redirect the output stream to a stringstream in order to find log messages
    std::stringstream ss;
    CoutRedirect redirect(ss);
    eCAL::Logging::Log(eCAL::Logging::eLogLevel::log_level_info, log_message);
    std::string console_output = ss.str();
    EXPECT_TRUE(console_output.find(log_message) != std::string::npos);
  }

  eCAL::Finalize();
}

int getLogging(eCAL::Logging::SLogging& log_)
{
  std::this_thread::sleep_for(UDP_WAIT_TIME);

  eCAL::Logging::GetLogging(log_);
  return static_cast<int>(log_.log_messages.size());
}

TEST(logging_levels /*unused*/, all /*unused*/)
{
  const std::string unit_name    = "logging_levels_all_udp";
  const std::string log_message  = "Logging level all test for udp.";
  auto  ecal_config              = GetUDPConfiguration();

  eCAL::Initialize(ecal_config, unit_name, eCAL::Init::Logging);

  eCAL::Logging::SLogging log;
 
  eCAL::Logging::Log(eCAL::Logging::log_level_info, log_message);
  
  EXPECT_EQ(getLogging(log), 1);

  eCAL::Logging::Log(eCAL::Logging::eLogLevel::log_level_warning, log_message);
  EXPECT_EQ(getLogging(log), 1);

  eCAL::Logging::Log(eCAL::Logging::eLogLevel::log_level_error, log_message);
  EXPECT_EQ(getLogging(log), 1);
 
  eCAL::Logging::Log(eCAL::Logging::eLogLevel::log_level_debug1, log_message);
  EXPECT_EQ(getLogging(log), 1);

  eCAL::Logging::Log(eCAL::Logging::eLogLevel::log_level_debug1, log_message);
  EXPECT_EQ(getLogging(log), 1);

  eCAL::Logging::Log(eCAL::Logging::eLogLevel::log_level_debug2, log_message);
  EXPECT_EQ(getLogging(log), 1);

  eCAL::Logging::Log(eCAL::Logging::eLogLevel::log_level_debug3, log_message);
  EXPECT_EQ(getLogging(log), 1);

  eCAL::Logging::Log(eCAL::Logging::eLogLevel::log_level_debug4, log_message);
  EXPECT_EQ(getLogging(log), 1);

  eCAL::Finalize();
}

TEST(logging_levels /*unused*/, log_warning_vs_info /*unused*/)
{
  const std::string unit_name    = "logging_levels_warning_vs_info_udp";
  const std::string log_message  = "Logging level warning vs info test for udp.";
  auto  ecal_config              = GetUDPConfiguration();

  ecal_config.logging.provider.udp.filter_log = eCAL::Logging::eLogLevel::log_level_warning;

  eCAL::Initialize(ecal_config, unit_name, eCAL::Init::Logging);

  eCAL::Logging::SLogging log;
  
  // logging not expected
  eCAL::Logging::Log(eCAL::Logging::eLogLevel::log_level_info, log_message);
  EXPECT_EQ(getLogging(log), 0);

  // logging expected
  eCAL::Logging::Log(eCAL::Logging::eLogLevel::log_level_warning, log_message);
  EXPECT_EQ(getLogging(log), 1);

  eCAL::Finalize();
}

TEST(logging_levels /*unused*/, none /*unused*/)
{
  const std::string unit_name    = "logging_levels_various_udp";
  const std::string log_message  = "Logging level none test for udp.";
  auto  ecal_config              = GetUDPConfiguration();

   ecal_config.logging.provider.udp.filter_log = eCAL::Logging::eLogLevel::log_level_none;

  eCAL::Initialize(ecal_config, unit_name, eCAL::Init::Logging);

  eCAL::Logging::SLogging log;
 
  eCAL::Logging::Log(eCAL::Logging::eLogLevel::log_level_info, log_message);
  EXPECT_EQ(getLogging(log), 0);

  eCAL::Logging::Log(eCAL::Logging::eLogLevel::log_level_warning, log_message);
  EXPECT_EQ(getLogging(log), 0);

  eCAL::Logging::Log(eCAL::Logging::eLogLevel::log_level_error, log_message);
  EXPECT_EQ(getLogging(log), 0);
 
  eCAL::Logging::Log(eCAL::Logging::eLogLevel::log_level_debug1, log_message);
  EXPECT_EQ(getLogging(log), 0);

  eCAL::Logging::Log(eCAL::Logging::eLogLevel::log_level_debug1, log_message);
  EXPECT_EQ(getLogging(log), 0);

  eCAL::Logging::Log(eCAL::Logging::eLogLevel::log_level_debug2, log_message);
  EXPECT_EQ(getLogging(log), 0);

  eCAL::Logging::Log(eCAL::Logging::eLogLevel::log_level_debug3, log_message);
  EXPECT_EQ(getLogging(log), 0);

  eCAL::Logging::Log(eCAL::Logging::eLogLevel::log_level_debug4, log_message);
  EXPECT_EQ(getLogging(log), 0);

  eCAL::Finalize();
}

TEST(logging_disable /*unused*/, file /*unused*/)
{
  const std::string logging_path = "./";
  const std::string unit_name    = "logging_disable_file_test";
  const std::string log_message  = "Disabled logging test for file.";
  auto  ecal_config              = GetFileConfiguration(logging_path);
  
  ecal_config.logging.provider.file.enable = false;
  eCAL::Initialize(ecal_config, unit_name, eCAL::Init::Logging);

  eCAL::Logging::Log(eCAL::Logging::eLogLevel::log_level_info, log_message);

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
  const std::string unit_name    = "logging_disable_udp_test";
  const std::string log_message  = "Disabled logging test for udp.";
  auto  ecal_config              = GetUDPConfiguration();

  ecal_config.logging.provider.udp.enable = false;
  eCAL::Initialize(ecal_config, unit_name, eCAL::Init::Logging);  

  eCAL::Logging::Log(eCAL::Logging::eLogLevel::log_level_info, log_message);

  std::this_thread::sleep_for(UDP_WAIT_TIME);
  
  eCAL::Logging::SLogging log;
  eCAL::Logging::GetLogging(log);

  EXPECT_EQ(log.log_messages.size(), 0);
  
  eCAL::Finalize();
}

TEST(logging_disable /*unused*/, udp_receive /*unused*/)
{
  const std::string unit_name    = "logging_disable_receive_udp_test";
  const std::string log_message  = "Disabled receive logging test for udp.";
  auto  ecal_config              = GetUDPConfiguration();

  ecal_config.logging.receiver.enable = false;
  eCAL::Initialize(ecal_config, unit_name.c_str(), eCAL::Init::Logging);  

  eCAL::Logging::Log(eCAL::Logging::eLogLevel::log_level_info, log_message);

  std::this_thread::sleep_for(UDP_WAIT_TIME);
  
  eCAL::Logging::SLogging log;
  eCAL::Logging::GetLogging(log);

  EXPECT_EQ(log.log_messages.size(), 0);
  
  eCAL::Finalize();
}

TEST(logging_disable /*unused*/, udp_different_receive_port /*unused*/)
{
  const std::string unit_name    = "no_logging_different_udp_port";
  const std::string log_message  = "No log receiving possible - different udp port.";
  auto  ecal_config              = GetUDPConfiguration();

  ecal_config.logging.receiver.enable = true;
  ecal_config.logging.receiver.udp_config.port   = 14009;

  eCAL::Initialize(ecal_config, unit_name.c_str(), eCAL::Init::Logging);

  eCAL::Logging::Log(eCAL::Logging::eLogLevel::log_level_info, log_message);

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
  auto  ecal_config              = GetConsoleConfiguration();

  ecal_config.logging.provider.console.enable = false;
  eCAL::Initialize(ecal_config, unit_name, eCAL::Init::Logging);

  {
    // Redirect the output stream to a stringstream in order to find log messages
    std::stringstream ss;
    CoutRedirect redirect(ss);
    eCAL::Logging::Log(eCAL::Logging::eLogLevel::log_level_info, log_message);
    std::string console_output = ss.str();
    EXPECT_TRUE(console_output.find(log_message) == std::string::npos);
  }

  eCAL::Finalize();
}