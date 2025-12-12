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

#include <ecal/core.h>
#include <ecal/config.h>

#include <gtest/gtest.h>

#include <stdexcept>
#include <string>
#include <vector>

#ifdef ECAL_CORE_CONFIGURATION
  #include "config/configuration_reader.h"
#endif
#include "default_configuration.h"
#include "ecal_def.h"

template<typename MEMBER, typename VALUE>
void SetValue(MEMBER& member, VALUE value)
{
  member = value;
}

TEST(core_cpp_config /*unused*/, user_config_passing /*unused*/)
{
  // Registration options
  const unsigned int        registration_timeout        = 80000U;
  const unsigned int        registration_refresh        = 2000U;

  // Transport layer options
  const bool                drop_out_of_order_messages  = true;
  std::string               ip_address                  = "238.200.100.2";
  const int                 upd_snd_buff                = (5242880 + 1024);

  // Monitoring options
  const eCAL::Logging::Filter mon_log_filter_con = eCAL::Logging::log_level_warning;
  
  // Publisher options
  const bool                pub_use_shm                 = false;

  eCAL::Configuration custom_config;
  try
  {
    custom_config.subscriber.drop_out_of_order_messages       = drop_out_of_order_messages;
    custom_config.transport_layer.udp.network.group           = ip_address;
    custom_config.transport_layer.udp.send_buffer             = upd_snd_buff;
    
    custom_config.logging.provider.console.log_level         = mon_log_filter_con;

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
  EXPECT_EQ(true, eCAL::Initialize(custom_config, "User Config Passing Test", eCAL::Init::Default));

  // Test boolean assignment, default is false
  EXPECT_EQ(drop_out_of_order_messages, eCAL::GetConfiguration().subscriber.drop_out_of_order_messages);

  // Test IP address assignment, default is 239.0.0.1
  EXPECT_EQ(ip_address, eCAL::GetConfiguration().transport_layer.udp.network.group);

  // Test UDP send buffer assignment, default is 5242880
  EXPECT_EQ(upd_snd_buff, eCAL::GetConfiguration().transport_layer.udp.send_buffer);

  // Test monitoring console log assignment, default is (log_level_info | log_level_warning | log_level_error | log_level_fatal)
  EXPECT_EQ(mon_log_filter_con, eCAL::GetConfiguration().logging.provider.console.log_level);

  // Test publisher sendmode assignment
  EXPECT_EQ(pub_use_shm, eCAL::GetConfiguration().publisher.layer.shm.enable);

  // Test registration option assignment, default timeout is 10000U and default refresh is 1000U
  EXPECT_EQ(registration_timeout, eCAL::GetConfiguration().registration.registration_timeout);
  EXPECT_EQ(registration_refresh, eCAL::GetConfiguration().registration.registration_refresh);

  // Finalize eCAL API
  EXPECT_EQ(true, eCAL::Finalize());
}

TEST(core_cpp_config /*unused*/, user_config_death_test /*unused*/)
{
  eCAL::Configuration custom_config;

  // Test the IpAddressV4 class with wrong values
  ASSERT_THROW(
    SetValue(custom_config.transport_layer.udp.network.group, "42"),
    std::invalid_argument);

  // Test the IpAddressV4 class with invalid addresses
  ASSERT_THROW(
    SetValue(custom_config.transport_layer.udp.network.group, "256.0.0.0"),
    std::invalid_argument);
  
  ASSERT_THROW(
    SetValue(custom_config.transport_layer.udp.network.group, "FFF.FF.FF.FF"),
    std::invalid_argument);
}

TEST(core_cpp_config /*unused*/, config_custom_datatypes_tests /*unused*/)
{
  // test custom datatype assignment operators
  eCAL::Types::IpAddressV4 ip1 { "192.168.0.1" };
  eCAL::Types::IpAddressV4 ip2 { "192.168.0.1" };
  EXPECT_EQ(ip1, ip2);

  ip1 = "192.168.0.2";
  ip2 = ip1;
  EXPECT_EQ(ip1, ip2);

  // test copy method for config structure
  eCAL::Configuration config1;
  eCAL::Configuration config2;
  std::string testValue = "234.0.3.2";
  config2.transport_layer.udp.network.group = testValue;
  auto& config2ref = config2;
  config1 = config2ref;

  EXPECT_EQ(config1.transport_layer.udp.network.group, testValue);
}

TEST(core_cpp_config /*unused*/, config_custom_ipv4_datatype /*unused*/)
{
  EXPECT_THROW(eCAL::Types::IpAddressV4("192.168.256.0"), std::invalid_argument);
  EXPECT_THROW(eCAL::Types::IpAddressV4("192.168.1.1:50"), std::invalid_argument);
  EXPECT_THROW(eCAL::Types::IpAddressV4("192.168.1"), std::invalid_argument);
  EXPECT_THROW(eCAL::Types::IpAddressV4("192.168"), std::invalid_argument);
  EXPECT_THROW(eCAL::Types::IpAddressV4("192"), std::invalid_argument);
  EXPECT_NO_THROW(eCAL::Types::IpAddressV4("192.168.1.0"));
  EXPECT_NO_THROW(eCAL::Types::IpAddressV4("255.255.255.240"));
  EXPECT_NO_THROW(eCAL::Types::IpAddressV4("255.255.255.255"));
  EXPECT_NO_THROW(eCAL::Types::IpAddressV4("0.0.0.0"));
  EXPECT_THROW(eCAL::Types::IpAddressV4("FF.FF.FF.F0"), std::invalid_argument);
}
