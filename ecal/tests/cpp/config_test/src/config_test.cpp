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
#include "ini_file.h"

#include <gtest/gtest.h>

#include <stdexcept>
#include <fstream>
#include <string>
#include <cstdio>

#include "ecal_cmd_parser.h"
#include "configuration_reader.h"

template<typename MEMBER, typename VALUE>
void SetValue(MEMBER& member, VALUE value)
{
  member = value;
}

TEST(core_cpp_config, user_config_passing)
{
  eCAL::Configuration custom_config(0, nullptr);

  // Test value assignments from each category
  // How the user would utilize it
  
  // Transport layer options
  const bool network_enabled   = true;
  std::string ip_address = "238.200.100.2";
  const int upd_snd_buff       = (5242880 + 1024);

  // Monitoring options
  const unsigned int        mon_timeout                          = 6000U;
  const std::string         mon_filter_excl                      = "_A.*";
  const eCAL_Logging_Filter mon_log_filter_con                   = log_level_warning;
  const eCAL::Monitoring::Types::Mode monitoring_mode            = eCAL::Monitoring::Types::Mode::udp_monitoring;
  
  // Publisher options
  const bool pub_use_shm = true;

  // Registration options
  const unsigned int registration_timeout = 80000U;
  const unsigned int registration_refresh = 2000U;
  const eCAL::Registration::Configuration registration = eCAL::Registration::Configuration(registration_timeout, registration_refresh);

  try{
    custom_config.transport_layer.network_enabled   = network_enabled;
    custom_config.transport_layer.mc_options.group  = ip_address;
    custom_config.transport_layer.mc_options.sndbuf = upd_snd_buff;
    
    custom_config.monitoring.monitoring_timeout = mon_timeout;
    custom_config.monitoring.filter_excl        = mon_filter_excl;
    custom_config.monitoring.monitoring_mode    = monitoring_mode;
    custom_config.logging.filter_log_con        = mon_log_filter_con;

    custom_config.publisher.shm.enable = pub_use_shm;

    custom_config.registration = registration;
  }
  catch (std::invalid_argument& e)
  {
    throw std::runtime_error("Error while configuring Configuration: " + std::string(e.what()));
  }

  // Initialize ecal api with custom config
  EXPECT_EQ(0, eCAL::Initialize(custom_config, "User Config Passing Test", eCAL::Init::Default));

  // Test boolean assignment, default is false
  EXPECT_EQ(network_enabled, eCAL::GetConfiguration().transport_layer.network_enabled);

  // Test IP address assignment, default is 239.0.0.1
  EXPECT_EQ(ip_address, eCAL::GetConfiguration().transport_layer.mc_options.group);

  // Test UDP send buffer assignment, default is 5242880
  EXPECT_EQ(upd_snd_buff, eCAL::GetConfiguration().transport_layer.mc_options.sndbuf);

  // Test monitoring timeout assignment, default is 5000U
  EXPECT_EQ(mon_timeout, eCAL::GetConfiguration().monitoring.monitoring_timeout);

  // Test monitoring filter exclude assignment, default is "_.*"
  EXPECT_EQ(mon_filter_excl, eCAL::GetConfiguration().monitoring.filter_excl);

  // Test monitoring console log assignment, default is (log_level_info | log_level_warning | log_level_error | log_level_fatal)
  EXPECT_EQ(mon_log_filter_con, eCAL::GetConfiguration().logging.filter_log_con);

  // Test monitoring mode assignment, default is eCAL::Types::MonitoringMode::none
  EXPECT_EQ(monitoring_mode, eCAL::GetConfiguration().monitoring.monitoring_mode);

  // Test publisher sendmode assignment, default is eCAL::TLayer::eSendMode::smode_auto
  EXPECT_EQ(pub_use_shm, eCAL::GetConfiguration().publisher.shm.enable);

  // Test registration option assignment, default timeout is 60000U and default refresh is 1000U
  EXPECT_EQ(registration_timeout, eCAL::GetConfiguration().registration.getTimeoutMS());
  EXPECT_EQ(registration_refresh, eCAL::GetConfiguration().registration.getRefreshMS());

  // Finalize eCAL API
  EXPECT_EQ(0, eCAL::Finalize());
}

TEST(ConfigDeathTest, user_config_death_test)
{
  eCAL::Configuration custom_config(0, nullptr);

  // Test the IpAddressV4 class with wrong values
  ASSERT_THROW(
    SetValue(custom_config.transport_layer.mc_options.group, "42"),
    std::invalid_argument);

  // Test the IpAddressV4 class with invalid addresses
  ASSERT_THROW(
    SetValue(custom_config.transport_layer.mc_options.group, "256.0.0.0"),
    std::invalid_argument);
  ASSERT_THROW(
    SetValue(custom_config.transport_layer.mc_options.group, "127.0.0.1"),
    std::invalid_argument);
  ASSERT_THROW(
    SetValue(custom_config.transport_layer.mc_options.group, "255.255.255.255"),
    std::invalid_argument);

  ASSERT_THROW(
    SetValue(custom_config.transport_layer.mc_options.group, "FFF.FF.FF.FF"),
    std::invalid_argument);
  ASSERT_THROW(
    SetValue(custom_config.transport_layer.mc_options.group, "FF.FF.FF.FF"),
    std::invalid_argument);
  ASSERT_THROW(
    SetValue(custom_config.transport_layer.mc_options.group, "Ff.fF.ff.Ff"),
    std::invalid_argument);
  ASSERT_THROW(
    SetValue(custom_config.transport_layer.mc_options.group, "7f.0.0.1"),
    std::invalid_argument);

  ASSERT_THROW(
    SetValue(custom_config.transport_layer.mc_options.group, "0.0.0.0"),
    std::invalid_argument);
  ASSERT_THROW(
    SetValue(custom_config.transport_layer.mc_options.group, "00.00.00.00"),
    std::invalid_argument);
  ASSERT_THROW(
    SetValue(custom_config.transport_layer.mc_options.group, "000.000.000.000"),
    std::invalid_argument);
  ASSERT_THROW(
    SetValue(custom_config.transport_layer.mc_options.group, "0.00.000.0"),
    std::invalid_argument);

  // Test the ConstrainedInteger class with wrong values. Default are MIN = 5242880, STEP = 1024
  // Value below MIN
  ASSERT_THROW(
    SetValue(custom_config.transport_layer.mc_options.sndbuf, 42),
    std::invalid_argument);
  
  // Wrong step. Default STEP = 1024
  ASSERT_THROW(
    SetValue(custom_config.transport_layer.mc_options.sndbuf, (5242880 + 512)),
    std::invalid_argument);

  // Value exceeds MAX. Default MAX = 100
  ASSERT_THROW(
    SetValue(custom_config.transport_layer.shm_options.memfile_reserve, 150),
    std::invalid_argument);

  // Test the registration option limits
  // Refresh timeout > registration timeout
  ASSERT_THROW(
    eCAL::Registration::Configuration(2000U, 3000U), std::invalid_argument);

  // Refresh timeout = registration timeout
  ASSERT_THROW(
    eCAL::Registration::Configuration(2000U, 2000U), std::invalid_argument);
}

TEST(core_cpp_config, config_custom_datatypes_tests)
{
  // test custom datatype assignment operators
  eCAL::Types::IpAddressV4 ip1;
  eCAL::Types::IpAddressV4 ip2;
  EXPECT_EQ(ip1, ip2);

  ip1 = "192.168.0.2";
  ip2 = ip1;
  EXPECT_EQ(ip1, ip2);

  eCAL::Types::ConstrainedInteger<0,1,10> s1;
  eCAL::Types::ConstrainedInteger<0,1,10> s2;
  EXPECT_EQ(s1, s2);

  s1 = 5;
  s2 = s1;
  EXPECT_EQ(s1, s2);

  // test copy method for config structure
  eCAL::Configuration config1(0, nullptr);
  eCAL::Configuration config2(0, nullptr);
  std::string testValue = "234.0.3.2";
  config2.transport_layer.mc_options.group = testValue;
  auto& config2ref = config2;
  config1 = config2ref;

  EXPECT_EQ(config1.transport_layer.mc_options.group, testValue);
}

TEST(core_cpp_config, config_cmd_parser)
{
  // create a custom ini file
  std::string ini_file_name = "customIni.ini";
  std::ofstream custom_ini_file(ini_file_name);

  if (custom_ini_file.is_open())
  {
    custom_ini_file << ini_file_as_string_deprecated;
    custom_ini_file.close();
  }
  else 
  {
    std::cerr << "Error opening file for ini writing" << "\n";
    return;
  }

  eCAL::Config::CmdParser parser;

  std::vector<std::string> arguments;

  const std::string set_config_key = "--ecal-set-config-key ";
  const std::string sep_slash = "/";
  const std::string sep_col = ":";

  const std::string network = "network";
  const std::string host_group_name = "host_group_name";
  const std::string config_test_machine = "ConfigTestMachine";
  const std::string network_enabled = "network_enabled";
  const std::string is_network_enabled = "true";
  
  const std::string common = "common";
  const std::string registration_timeout = "registration_timeout";
  const std::string registration_refresh = "registration_refresh";
  const std::string reg_to_value = "6000";
  const std::string reg_rf_value = "1000";

  arguments.push_back("test_config_cmd_parser");
  arguments.push_back("--ecal-ini-file customIni.ini");
  std::string host_group_string = set_config_key + network + sep_slash + host_group_name + sep_col + config_test_machine;
  arguments.push_back(host_group_string);
  std::string network_enabled_string = set_config_key + network + sep_slash + network_enabled + sep_col + is_network_enabled;
  arguments.push_back(network_enabled_string);
  std::string registration_to_string = set_config_key + common + sep_slash + registration_timeout + sep_col + reg_to_value;
  arguments.push_back(registration_to_string);
  std::string registration_rf_string = set_config_key + common + sep_slash + registration_refresh + sep_col + reg_rf_value;
  arguments.push_back(registration_rf_string);

  try
  {
    parser.parseArguments(arguments);
  }
  catch(const std::runtime_error& e)
  {
    std::cerr << e.what() << '\n';
  }
  
  // Expect a valid ini file
  EXPECT_NE(parser.getUserIni(), std::string(""));

  // Expect a proper key-value map in the config key map
  EXPECT_EQ(parser.getConfigKeysMap()[network][host_group_name], config_test_machine);
  EXPECT_EQ(parser.getConfigKeysMap()[network][network_enabled], is_network_enabled);
  EXPECT_EQ(parser.getConfigKeysMap()[common][registration_timeout], reg_to_value);
  EXPECT_EQ(parser.getConfigKeysMap()[common][registration_refresh], reg_rf_value);

  remove(ini_file_name.data());
}

TEST(CmdParserDeathTest, config_cmd_parser_death_test)
{
  eCAL::Config::CmdParser parser;

  std::vector<std::string> arguments;

  arguments.push_back("test_config_cmd_parser_death_test");
  arguments.push_back("--ecal-ini-file someNotValidFileName.ini");

  ASSERT_THROW(
    parser.parseArguments(arguments),
    std::runtime_error
  );
}

TEST(YamlConfigReaderTest, read_write_file_test)
{
  // create a custom ini file
  std::string ini_file_name = "customIni.yml";
  std::ofstream custom_ini_file(ini_file_name);

  if (custom_ini_file.is_open())
  {
    custom_ini_file << ini_file_as_string_yaml;
    custom_ini_file.close();
  }
  else 
  {
    std::cerr << "Error opening file for ini writing" << "\n";
    return;
  }

  eCAL::Configuration config;
  EXPECT_NO_THROW(config = eCAL::Config::ParseYamlFromFile(ini_file_name));

  EXPECT_EQ(true, eCAL::Config::WriteConfigurationToYaml("myTest.yml", config));

  remove(ini_file_name.data());
  remove("myTest.yml");
}

TEST(YamlConfigReaderTest, parse_values_test)
{
  eCAL::Configuration config;
  EXPECT_NO_THROW(config = eCAL::Config::ParseYamlFromString(ini_file_as_string_yaml));

  // Check string 
  EXPECT_EQ(config.application.startup.terminal_emulator, "myTestTerminal");

  // Check equality of IpAddressV4
  EXPECT_EQ(config.transport_layer.mc_options.group, "239.5.0.1");

  // Check constrained Integer
  EXPECT_EQ(config.transport_layer.mc_options.port, 14010);

  // Check boolean
  EXPECT_EQ(config.transport_layer.mc_options.npcap_enabled, true);

  // Check unsigned size_t
  EXPECT_EQ(config.transport_layer.tcp_options.max_reconnections, 7);

  // Check unsigned int
  EXPECT_EQ(config.publisher.shm.acknowledge_timeout_ms, 346U);
}