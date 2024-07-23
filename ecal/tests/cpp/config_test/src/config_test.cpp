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
  // Registration options
  const unsigned int        registration_timeout        = 80000U;
  const unsigned int        registration_refresh        = 2000U;

  // Transport layer options
  const bool                drop_out_of_order_messages  = true;
  std::string               ip_address                  = "238.200.100.2";
  const int                 upd_snd_buff                = (5242880 + 1024);

  // Monitoring options
  const unsigned int        mon_timeout                 = 6000U;
  const std::string         mon_filter_excl             = "_A.*";
  const eCAL_Logging_Filter mon_log_filter_con          = log_level_warning;
  
  // Publisher options
  const bool                pub_use_shm                 = false;

  eCAL::Configuration custom_config(0, nullptr);
  try
  {
    custom_config.subscriber.drop_out_of_order_messages       = drop_out_of_order_messages;
    custom_config.transport_layer.udp.network.group           = ip_address;
    custom_config.transport_layer.udp.send_buffer             = upd_snd_buff;
    
    custom_config.monitoring.timeout                          = mon_timeout;
    custom_config.monitoring.filter_excl                      = mon_filter_excl;
    custom_config.logging.sinks.console.filter_log_con        = mon_log_filter_con;

    custom_config.publisher.layer.shm.enable                  = pub_use_shm;

    custom_config.registration.registration_refresh           = registration_refresh;
    custom_config.registration.registration_timeout           = registration_timeout;
  }
  catch (std::invalid_argument& e)
  {
    FAIL() << "Error while configuring Configuration: " << std::string(e.what());
    return;
  }

  // Initialize ecal api with custom config
  EXPECT_EQ(0, eCAL::Initialize(custom_config, "User Config Passing Test", eCAL::Init::Default));

  // Test boolean assignment, default is false
  EXPECT_EQ(drop_out_of_order_messages, eCAL::GetConfiguration().subscriber.drop_out_of_order_messages);

  // Test IP address assignment, default is 239.0.0.1
  EXPECT_EQ(ip_address, eCAL::GetConfiguration().transport_layer.udp.network.group);

  // Test UDP send buffer assignment, default is 5242880
  EXPECT_EQ(upd_snd_buff, eCAL::GetConfiguration().transport_layer.udp.send_buffer);

  // Test monitoring timeout assignment, default is 5000U
  EXPECT_EQ(mon_timeout, eCAL::GetConfiguration().monitoring.timeout);

  // Test monitoring filter exclude assignment, default is "_.*"
  EXPECT_EQ(mon_filter_excl, eCAL::GetConfiguration().monitoring.filter_excl);

  // Test monitoring console log assignment, default is (log_level_info | log_level_warning | log_level_error | log_level_fatal)
  EXPECT_EQ(mon_log_filter_con, eCAL::GetConfiguration().logging.sinks.console.filter_log_con);

  // Test publisher sendmode assignment, default is eCAL::TLayer::eSendMode::smode_auto
  EXPECT_EQ(pub_use_shm, eCAL::GetConfiguration().publisher.layer.shm.enable);

  // Test registration option assignment, default timeout is 60000U and default refresh is 1000U
  EXPECT_EQ(registration_timeout, eCAL::GetConfiguration().registration.registration_timeout);
  EXPECT_EQ(registration_refresh, eCAL::GetConfiguration().registration.registration_refresh);

  // Finalize eCAL API
  EXPECT_EQ(0, eCAL::Finalize());
}

TEST(ConfigDeathTest, user_config_death_test)
{
  eCAL::Configuration custom_config(0, nullptr);

  // Test the IpAddressV4 class with wrong values
  ASSERT_THROW(
    SetValue(custom_config.transport_layer.udp.network.group, "42"),
    std::invalid_argument);

  // Test the IpAddressV4 class with invalid addresses
  ASSERT_THROW(
    SetValue(custom_config.transport_layer.udp.network.group, "256.0.0.0"),
    std::invalid_argument);
  ASSERT_THROW(
    SetValue(custom_config.transport_layer.udp.network.group, "127.0.0.1"),
    std::invalid_argument);
  ASSERT_THROW(
    SetValue(custom_config.transport_layer.udp.network.group, "255.255.255.255"),
    std::invalid_argument);

  ASSERT_THROW(
    SetValue(custom_config.transport_layer.udp.network.group, "FFF.FF.FF.FF"),
    std::invalid_argument);
  ASSERT_THROW(
    SetValue(custom_config.transport_layer.udp.network.group, "FF.FF.FF.FF"),
    std::invalid_argument);
  ASSERT_THROW(
    SetValue(custom_config.transport_layer.udp.network.group, "Ff.fF.ff.Ff"),
    std::invalid_argument);
  ASSERT_THROW(
    SetValue(custom_config.transport_layer.udp.network.group, "7f.0.0.1"),
    std::invalid_argument);

  ASSERT_THROW(
    SetValue(custom_config.transport_layer.udp.network.group, "0.0.0.0"),
    std::invalid_argument);
  ASSERT_THROW(
    SetValue(custom_config.transport_layer.udp.network.group, "00.00.00.00"),
    std::invalid_argument);
  ASSERT_THROW(
    SetValue(custom_config.transport_layer.udp.network.group, "000.000.000.000"),
    std::invalid_argument);
  ASSERT_THROW(
    SetValue(custom_config.transport_layer.udp.network.group, "0.00.000.0"),
    std::invalid_argument);

  // Test the ConstrainedInteger class with wrong values. Default are MIN = 5242880, STEP = 1024
  // Value below MIN
  ASSERT_THROW(
    SetValue(custom_config.transport_layer.udp.send_buffer, 42),
    std::invalid_argument);
  
  // Wrong step. Default STEP = 1024
  ASSERT_THROW(
    SetValue(custom_config.transport_layer.udp.send_buffer, (5242880 + 512)),
    std::invalid_argument);

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
  config2.transport_layer.udp.network.group = testValue;
  auto& config2ref = config2;
  config1 = config2ref;

  EXPECT_EQ(config1.transport_layer.udp.network.group, testValue);
}

TEST(CmdParserTest, config_cmd_parser_test)
{
  const std::string some_file_name = "someFileName.yml";

  eCAL::Config::CmdParser parser;

  EXPECT_EQ(parser.getUserIni(), "");
  EXPECT_EQ(parser.getDumpConfig(), false);

  std::vector<std::string> arguments;

  arguments.push_back("test_config_cmd_parser_test");
  // set a file name as ini file
  arguments.push_back("--ecal-config-file " + some_file_name);
  // set the dump config flag
  arguments.push_back("--ecal-dump-config");

  parser.parseArguments(arguments);

  EXPECT_EQ(parser.getUserIni(), some_file_name);
  EXPECT_EQ(parser.getDumpConfig(), true);
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
    FAIL() << "Error opening file for ini writing";
    return;
  }

  eCAL::Configuration config;
  EXPECT_NO_THROW(eCAL::Config::YamlFileToConfig(ini_file_name, config));

  EXPECT_EQ(true, eCAL::Config::ConfigToYamlFile("myTest.yml", config));

  remove(ini_file_name.data());
  remove("myTest.yml");
}

TEST(YamlConfigReaderTest, parse_values_test)
{
  eCAL::Configuration config;
  EXPECT_NO_THROW(eCAL::Config::YamlStringToConfig(ini_file_as_string_yaml, config));

  // Check string 
  EXPECT_EQ(config.application.startup.terminal_emulator, "myTestTerminal");

  // Check equality of IpAddressV4
  EXPECT_EQ(config.transport_layer.udp.network.group, "239.5.0.1");

  // Check constrained Integer
  EXPECT_EQ(config.transport_layer.udp.port, 14010);

  // Check boolean
  EXPECT_EQ(config.transport_layer.udp.npcap_enabled, true);

  // Check unsigned size_t
  EXPECT_EQ(config.transport_layer.tcp.max_reconnections, 7);

  // Check unsigned int
  EXPECT_EQ(config.publisher.layer.shm.acknowledge_timeout_ms, 346U);
} 

TEST(YamlConfigReaderTest, yaml_node_merger)
{
  YAML::Node node_1;
  YAML::Node node_2;

  node_1["test"] = 1;
  node_2["test"] = 2;
  node_2[3] = "I have an int key!";

  node_1["test2"] = 3;
  
  node_1["firstLayer1"]["secondLayer1"] = "192.168.0.2";
  node_2["firstLayer1"]["secondLayer1"] = "192.168.0.5";

  // try also with a sequence
  node_2["firstLayer2"]["secondLayer2"] = YAML::Load("[1, 2, 3]");
  
  eCAL::Config::MergeYamlNodes(node_1, node_2);

  EXPECT_EQ(node_1["test"], node_2["test"]);
  EXPECT_EQ(node_1[3], node_2[3]);
  EXPECT_EQ(node_1["3"], node_2["3"]);  
  EXPECT_EQ(node_1["firstLayer1"]["secondLayer1"], node_2["firstLayer1"]["secondLayer1"]);
  EXPECT_EQ(node_1["firstLayer2"]["secondLayer2"], node_2["firstLayer2"]["secondLayer2"]);  
}

TEST(YamlConfigReaderTest, yaml_to_config_merger)
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
    FAIL() << "Error opening file for ini writing";
    return;
  }

  eCAL::Configuration config;

  EXPECT_TRUE(config.publisher.layer.shm.enable);

  eCAL::Config::MergeYamlIntoConfiguration(ini_file_name, config);

  EXPECT_FALSE(config.publisher.layer.shm.enable);

  remove(ini_file_name.data());
}