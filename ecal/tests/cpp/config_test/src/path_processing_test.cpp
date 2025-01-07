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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ecal_path_processing.h"
#include "ecal_utils/filesystem.h"
#include "util/getenvvar.h"

#include <ecal/ecal_os.h>
#include <ecal_def.h>
#include <ecal/ecal_util.h>

#include <cstdlib>

class MockEnvVar
{
  public:
    MOCK_METHOD(std::string, eCALDataEnvPath, (), ());
};

class MockFileSystem
{
  public:
    MOCK_METHOD(bool, dirExists, (const std::string& path), (const));
};

TEST(core_cpp_path_processing /*unused*/, ecal_data_log_env_vars /*unused*/)
{
  
  const std::string env_ecal_conf_value = "/path/to/conf";
  const std::string env_ecal_log_value = "/path/to/log";

  { // Check for config path
    MockEnvVar mock_env_var;
    EXPECT_CALL(mock_env_var, eCALDataEnvPath()).WillOnce(::testing::Return(env_ecal_conf_value));

    auto data_env_dir = eCAL::Config::eCALDataEnvPath();
    EXPECT_EQ(data_env_dir, env_ecal_conf_value);
    // EXPECT_EQ(eCAL::Config::eCALDataEnvPath(), env_ecal_conf_value);
    
    // EXPECT_CALL(mock_env_var, getEnvVar(::testing::_, "")).WillOnce(::testing::Return(env_ecal_log_value));
    // auto log_dir = eCAL::Config::eCALLogDir();
    // // NE because log dir is not created
    // EXPECT_NE(log_dir, env_ecal_log_value);
    // // not empty, because it should return a temp dir
    // EXPECT_NE(log_dir, "");
  }

}