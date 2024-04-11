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

#include <gtest/gtest.h>

template<typename MEMBER, typename VALUE>
void SetValue(MEMBER& member, VALUE value)
{
  member = value;
}

TEST(core_cpp_config, user_config_passing)
{
  eCAL::Config::eCALConfig custom_config(0, nullptr);

  // Test value assignments from each category
  // How the user would utilize it
  
  // Transport layer options
  bool network_enabled   = true;
  std::string ip_address = "238.200.100.2";
  int upd_snd_buff       = (5242880 + 1024);

  custom_config.transport_layer_options.network_enabled   = network_enabled;
  custom_config.transport_layer_options.mc_options.group  = ip_address;
  custom_config.transport_layer_options.mc_options.sndbuf = upd_snd_buff;
  
  // Monitoring options
  unsigned int        mon_timeout                          = 6000U;
  std::string         mon_filter_excl                      = "_A.*";
  eCAL_Logging_Filter mon_log_filter_con                   = log_level_warning;
  eCAL::Config::eCAL_MonitoringMode_Filter monitoring_mode = eCAL::Config::MonitoringMode::udp_monitoring;
  
  custom_config.monitoring_options.monitoring_timeout = mon_timeout;
  custom_config.monitoring_options.filter_excl        = mon_filter_excl;
  custom_config.monitoring_options.monitoring_mode    = monitoring_mode;
  custom_config.logging_options.filter_log_con        = mon_log_filter_con;

  // Publisher options
  eCAL::TLayer::eSendMode pub_use_shm = eCAL::TLayer::eSendMode::smode_off;

  custom_config.publisher_options.use_shm = pub_use_shm;

  // Registration options
  unsigned int registration_timeout = 80000U;
  unsigned int registration_refresh = 2000U;
  eCAL::Config::RegistrationOptions registration_options = eCAL::Config::RegistrationOptions(registration_timeout, registration_refresh);

  custom_config.registration_options = registration_options;


  // Initialize ecal api with custom config
  EXPECT_EQ(0, eCAL::Initialize(custom_config, "User Config Passing Test", eCAL::Init::Default));

  // Test boolean assignment, default is false
  EXPECT_EQ(network_enabled, eCAL::Config::GetCurrentConfig().transport_layer_options.network_enabled);

  // Test IP address assignment, default is 239.0.0.1
  EXPECT_EQ(ip_address, static_cast<std::string>(eCAL::Config::GetCurrentConfig().transport_layer_options.mc_options.group));

  // Test UDP send buffer assignment, default is 5242880
  EXPECT_EQ(upd_snd_buff, static_cast<int>(eCAL::Config::GetCurrentConfig().transport_layer_options.mc_options.sndbuf));

  // Test monitoring timeout assignment, default is 5000U
  EXPECT_EQ(mon_timeout, eCAL::Config::GetCurrentConfig().monitoring_options.monitoring_timeout);

  // Test monitoring filter exclude assignment, default is "_.*"
  EXPECT_EQ(mon_filter_excl, eCAL::Config::GetCurrentConfig().monitoring_options.filter_excl);

  // Test monitoring console log assignment, default is (log_level_info | log_level_warning | log_level_error | log_level_fatal)
  EXPECT_EQ(mon_log_filter_con, eCAL::Config::GetCurrentConfig().logging_options.filter_log_con);

  // Test monitoring mode assignment, default iseCAL::Config::MonitoringMode::none
  EXPECT_EQ(monitoring_mode, eCAL::Config::GetCurrentConfig().monitoring_options.monitoring_mode);

  // Test publisher sendmode assignment, default is eCAL::TLayer::eSendMode::smode_auto
  EXPECT_EQ(pub_use_shm, eCAL::Config::GetCurrentConfig().publisher_options.use_shm);

  // Test registration option assignment, default timeout is 60000U and default refresh is 1000U
  EXPECT_EQ(registration_timeout, eCAL::Config::GetCurrentConfig().registration_options.getTimeoutMS());
  EXPECT_EQ(registration_refresh, eCAL::Config::GetCurrentConfig().registration_options.getRefreshMS());

  // Finalize eCAL API
  EXPECT_EQ(0, eCAL::Finalize());
}

TEST(ConfigDeathTest, user_config_death_test)
{
  eCAL::Config::eCALConfig custom_config(0, nullptr);

  // Test the IpAddressV4 class with wrong values
  EXPECT_EXIT(
    SetValue(custom_config.transport_layer_options.mc_options.group, std::string("42")),
    ::testing::ExitedWithCode(EXIT_FAILURE), "IpAddressV4");

  // Test the IpAddressV4 class with invalid addresses
  EXPECT_EXIT(
    SetValue(custom_config.transport_layer_options.mc_options.group, std::string("256.0.0.0")),
    ::testing::ExitedWithCode(EXIT_FAILURE), "IpAddressV4");
  EXPECT_EXIT(
    SetValue(custom_config.transport_layer_options.mc_options.group, std::string("127.0.0.1")),
    ::testing::ExitedWithCode(EXIT_FAILURE), "IpAddressV4");
  EXPECT_EXIT(
    SetValue(custom_config.transport_layer_options.mc_options.group, std::string("255.255.255.255")),
    ::testing::ExitedWithCode(EXIT_FAILURE), "IpAddressV4");

  EXPECT_EXIT(
    SetValue(custom_config.transport_layer_options.mc_options.group, std::string("FFF.FF.FF.FF")),
    ::testing::ExitedWithCode(EXIT_FAILURE), "IpAddressV4");
  EXPECT_EXIT(
    SetValue(custom_config.transport_layer_options.mc_options.group, std::string("FF.FF.FF.FF")),
    ::testing::ExitedWithCode(EXIT_FAILURE), "IpAddressV4");
  EXPECT_EXIT(
    SetValue(custom_config.transport_layer_options.mc_options.group, std::string("Ff.fF.ff.Ff")),
    ::testing::ExitedWithCode(EXIT_FAILURE), "IpAddressV4");
  EXPECT_EXIT(
    SetValue(custom_config.transport_layer_options.mc_options.group, std::string("7f.0.0.1")),
    ::testing::ExitedWithCode(EXIT_FAILURE), "IpAddressV4");

  EXPECT_EXIT(
    SetValue(custom_config.transport_layer_options.mc_options.group, std::string("0.0.0.0")),
    ::testing::ExitedWithCode(EXIT_FAILURE), "IpAddressV4");
  EXPECT_EXIT(
    SetValue(custom_config.transport_layer_options.mc_options.group, std::string("00.00.00.00")),
    ::testing::ExitedWithCode(EXIT_FAILURE), "IpAddressV4");
  EXPECT_EXIT(
    SetValue(custom_config.transport_layer_options.mc_options.group, std::string("000.000.000.000")),
    ::testing::ExitedWithCode(EXIT_FAILURE), "IpAddressV4");
  EXPECT_EXIT(
    SetValue(custom_config.transport_layer_options.mc_options.group, std::string("0.00.000.0")),
    ::testing::ExitedWithCode(EXIT_FAILURE), "IpAddressV4");

  // Test the LimitSize class with wrong values. Default are MIN = 5242880, STEP = 1024
  // Value below MIN
  EXPECT_EXIT(
    SetValue(custom_config.transport_layer_options.mc_options.sndbuf, 42),
    ::testing::ExitedWithCode(EXIT_FAILURE), "LimitSize");
  
  // Wrong step. Default STEP = 1024
  EXPECT_EXIT(
    SetValue(custom_config.transport_layer_options.mc_options.sndbuf, (5242880 + 512)),
    ::testing::ExitedWithCode(EXIT_FAILURE), "LimitSize");

  // Value exceeds MAX. Default MAX = 100
  EXPECT_EXIT(
    SetValue(custom_config.transport_layer_options.shm_options.memfile_reserve, 150),
    ::testing::ExitedWithCode(EXIT_FAILURE), "LimitSize");
}

TEST(core_cpp_config, config_custom_datatypes)
{
  // test custom datatype assignment operators
  eCAL::Config::IpAddressV4 ip1;
  eCAL::Config::IpAddressV4 ip2;
  EXPECT_EQ(static_cast<std::string>(ip1), static_cast<std::string>(ip2));

  ip1 = "192.168.0.2";
  ip2 = ip1;
  EXPECT_EQ(static_cast<std::string>(ip1), static_cast<std::string>(ip2));

  eCAL::Config::LimitSize<0,1,10> s1;
  eCAL::Config::LimitSize<0,1,10> s2;
  EXPECT_EQ(static_cast<int>(s1), static_cast<int>(s2));

  s1 = 5;
  s2 = s1;
  EXPECT_EQ(static_cast<int>(s1), static_cast<int>(s2));

  // test copy method for config structure
  eCAL::Config::eCALConfig config1(0, nullptr);
  eCAL::Config::eCALConfig config2(0, nullptr);
  std::string testValue = std::string("234.0.3.2");
  config2.transport_layer_options.mc_options.group = testValue;
  auto& config2ref = config2;
  config1 = config2ref;

  EXPECT_EQ(static_cast<std::string>(config1.transport_layer_options.mc_options.group), testValue);
}