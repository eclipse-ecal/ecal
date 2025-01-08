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

class MockEnvProvider  : public eCAL::Util::IEnvProvider
{
  public: 
    MOCK_METHOD(std::string, eCALEnvVar, (const std::string& var_), (const, override));
};

class MockDirManager : public eCAL::Util::IDirManager
{
  public:
    MOCK_METHOD(bool, dirExists, (const std::string& path_), (const, override));
    MOCK_METHOD(bool, dirExistsOrCreate, (const std::string& path_), (const, override));
    MOCK_METHOD(bool, createDir, (const std::string& path_), (const, override));
    MOCK_METHOD(bool, createEcalDirStructure, (const std::string& path_), (const, override));
};

TEST(core_cpp_path_processing /*unused*/, ecal_data_log_env_vars /*unused*/)
{
  const std::string env_ecal_conf_value = "/path/to/conf";
  const std::string env_ecal_log_value = "/path/to/log";

  MockEnvProvider mock_env_provider;

  EXPECT_CALL(mock_env_provider, eCALEnvVar(ECAL_DATA_VAR)).WillOnce(testing::Return(env_ecal_conf_value));

  MockDirManager mock_dir_manager;

  EXPECT_CALL(mock_dir_manager, dirExists(env_ecal_conf_value)).WillOnce(testing::Return(true));

  std::string ecal_data_dir = eCAL::Util::GeteCALDataDir();
  EXPECT_EQ(ecal_data_dir, env_ecal_conf_value);
}