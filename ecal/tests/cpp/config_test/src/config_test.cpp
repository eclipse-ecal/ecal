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
#include <ecal/types/ecal_config_types.h>

#include <gtest/gtest.h>

template<typename MEMBER, typename VALUE>
void SetValue(MEMBER& member, VALUE value)
{
  member = value;
}

TEST(core_cpp_config, user_config_passing)
{
  eCAL::Config::eCALConfig custom_config;

  custom_config.transport_layer_options.network_enabled = true;
  // initialize ecal api with custom config
  EXPECT_EQ(0, eCAL::Initialize(0, nullptr, "user_config_passing test", eCAL::Init::Default, &custom_config));

  // test if created value is accessible, default was false
  EXPECT_EQ(true, eCAL::Config::GetCurrentConfig()->transport_layer_options.network_enabled);

  // finalize eCAL API
  EXPECT_EQ(0, eCAL::Finalize());
}

TEST(ConfigDeathTest, user_config_death_test)
{
  eCAL::Config::eCALConfig custom_config;

  // Test the IpAddressV4 class with wrong values
  EXPECT_EXIT(
    SetValue(custom_config.transport_layer_options.mc_options.group, std::string("42")),
    ::testing::ExitedWithCode(EXIT_FAILURE), "IpAddressV4");

  // Test the LimitSize class with wrong values. Default are MIN = 5242880, STEP = 1024
  EXPECT_EXIT(
    SetValue(custom_config.transport_layer_options.mc_options.sndbuf, 42),
    ::testing::ExitedWithCode(EXIT_FAILURE), "LimitSize");
}