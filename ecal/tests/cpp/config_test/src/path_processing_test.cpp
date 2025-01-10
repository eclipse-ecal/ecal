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
#include <map>

// For disabling uninteresting call messages
using ::testing::NiceMock;

namespace
{
  // get the path separator from the current OS (win: "\\", unix: "/")
  const std::string path_separator(1, EcalUtils::Filesystem::NativeSeparator());
}

class MockDirManager : public eCAL::Util::IDirManager
{
  public:
    MOCK_METHOD(bool, dirExists, (const std::string& path_), (const, override));
    MOCK_METHOD(bool, dirExistsOrCreate, (const std::string& path_), (const, override));
    MOCK_METHOD(bool, createDir, (const std::string& path_), (const, override));
    MOCK_METHOD(bool, createEcalDirStructure, (const std::string& path_), (const, override));
    MOCK_METHOD(std::string, findFileInPaths, (const std::vector<std::string>& paths_, const std::string& file_name_), (const, override));
};

class MockDirProvider  : public eCAL::Util::IDirProvider
{
  public: 
    MOCK_METHOD(std::string, eCALEnvVar, (const std::string& var_), (const, override));
    MOCK_METHOD(std::string, eCALLocalUserDir, (), (const, override));
    MOCK_METHOD(std::string, eCALDataSystemDir, (const eCAL::Util::IDirManager& dir_manager_), (const, override));
    MOCK_METHOD(std::string, uniqueTmpDir, (const eCAL::Util::IDirManager& dir_manager_), (const, override));
};

using DirAvailabilityMap = std::map<std::string, bool>;

TEST(core_cpp_path_processing /*unused*/, ecal_data_log_env_vars /*unused*/)
{
  const std::string env_ecal_conf_value = "/path/to/conf";
  const std::string env_ecal_log_value = "/path/to/log";
  const std::string unique_tmp_dir = "/tmp/unique";

  const int expected_call_count = 5;
  const int expected_call_count_log_var = 3;

  MockDirProvider mock_dir_provider;
  NiceMock<MockDirManager> mock_dir_manager;

  // Disable all other calls right now, as we just want to test the env vars
  EXPECT_CALL(mock_dir_provider, eCALDataSystemDir(::testing::Ref(mock_dir_manager)))
    .Times(expected_call_count)
    .WillRepeatedly(testing::Return(""));
  EXPECT_CALL(mock_dir_provider, eCALLocalUserDir())
    .Times(expected_call_count)
    .WillRepeatedly(testing::Return(""));
  
  // set the tmp directory
  EXPECT_CALL(mock_dir_provider, uniqueTmpDir(::testing::Ref(mock_dir_manager)))
    .Times(1)
    .WillOnce(testing::Return(unique_tmp_dir));

  // let's assume all directories exist
  ON_CALL(mock_dir_manager, dirExists(testing::_)).WillByDefault(testing::Return(true));

  // mock the environment variables to be existent
  EXPECT_CALL(mock_dir_provider, eCALEnvVar(ECAL_DATA_VAR))
    .Times(expected_call_count)
    .WillOnce(testing::Return(env_ecal_conf_value))
    .WillOnce(testing::Return(env_ecal_conf_value))
    .WillOnce(testing::Return(env_ecal_conf_value))
    .WillRepeatedly(testing::Return(""));
  EXPECT_CALL(mock_dir_provider, eCALEnvVar(ECAL_LOG_VAR))
    .Times(expected_call_count_log_var)
    .WillOnce(testing::Return(env_ecal_log_value))
    .WillRepeatedly(testing::Return(""));

  EXPECT_EQ(eCAL::Config::GeteCALDataDirImpl(mock_dir_provider, mock_dir_manager), env_ecal_conf_value);
  EXPECT_EQ(eCAL::Config::GeteCALLogDirImpl(mock_dir_provider, mock_dir_manager), env_ecal_log_value);
  EXPECT_EQ(eCAL::Config::GeteCALLogDirImpl(mock_dir_provider, mock_dir_manager), env_ecal_conf_value);
  
  // Now nothing is set, GeteCALDataDirImpl should return empty string
  EXPECT_EQ(eCAL::Config::GeteCALDataDirImpl(mock_dir_provider, mock_dir_manager), "");
  
  // log dir is now the temporary one, this is what it should return
  EXPECT_EQ(eCAL::Config::GeteCALLogDirImpl(mock_dir_provider, mock_dir_manager), unique_tmp_dir);
}

TEST(core_cpp_path_processing /*unused*/, ecal_local_user_dir /*unused*/)
{
  const std::string ecal_local_user_dir = "/local/user/dir";
  const std::string ecal_local_user_log_dir = ecal_local_user_dir + path_separator + ECAL_FOLDER_NAME_LOG;
  const std::string unique_tmp_dir = "/tmp/unique";
  const int expected_call_count = 4;
  const int expected_call_count_log_var = 2;

  MockDirProvider mock_dir_provider;
  NiceMock<MockDirManager> mock_dir_manager;

  // mock the environment variables to be empty
  EXPECT_CALL(mock_dir_provider, eCALEnvVar(ECAL_DATA_VAR))
    .Times(expected_call_count)
    .WillRepeatedly(testing::Return(""));
  EXPECT_CALL(mock_dir_provider, eCALEnvVar(ECAL_LOG_VAR))
    .Times(expected_call_count_log_var)
    .WillRepeatedly(testing::Return(""));
  EXPECT_CALL(mock_dir_provider, eCALDataSystemDir(::testing::Ref(mock_dir_manager)))
    .Times(expected_call_count)
    .WillRepeatedly(testing::Return(""));
  
  // set the tmp directory
  EXPECT_CALL(mock_dir_provider, uniqueTmpDir(::testing::Ref(mock_dir_manager)))
    .Times(1)
    .WillOnce(testing::Return(unique_tmp_dir));

  // let's assume all directories exist
  ON_CALL(mock_dir_manager, dirExists(testing::_)).WillByDefault(testing::Return(true));

  // mock the local user dir to be existent
  EXPECT_CALL(mock_dir_provider, eCALLocalUserDir())
    .Times(expected_call_count)
    .WillOnce(testing::Return(ecal_local_user_dir))
    .WillOnce(testing::Return(ecal_local_user_dir))
    .WillRepeatedly(testing::Return(""));

  // Testing with eCALData and eCALLog -DirImpl
  EXPECT_EQ(eCAL::Config::GeteCALDataDirImpl(mock_dir_provider, mock_dir_manager), ecal_local_user_dir);
  EXPECT_EQ(eCAL::Config::GeteCALLogDirImpl(mock_dir_provider, mock_dir_manager),  ecal_local_user_log_dir);
  EXPECT_EQ(eCAL::Config::GeteCALDataDirImpl(mock_dir_provider, mock_dir_manager), "");
  
  // Log dir has tmp dir now
  EXPECT_EQ(eCAL::Config::GeteCALLogDirImpl(mock_dir_provider, mock_dir_manager), unique_tmp_dir);
}

TEST(core_cpp_path_processing /*unused*/, ecal_data_system_dir /*unused*/)
{
  const std::string ecal_data_system_dir = "/data/system/dir";
  const std::string ecal_data_system_log_dir = ecal_data_system_dir + path_separator + ECAL_FOLDER_NAME_LOG;
  const std::string unique_tmp_dir = "/tmp/unique";
  const int expected_call_count = 4;
  const int expected_call_count_log_var = 2;

  MockDirProvider mock_dir_provider;
  NiceMock<MockDirManager> mock_dir_manager;

  // mock the environment variables to be empty
  EXPECT_CALL(mock_dir_provider, eCALEnvVar(ECAL_DATA_VAR))
    .Times(expected_call_count)
    .WillRepeatedly(testing::Return(""));
  EXPECT_CALL(mock_dir_provider, eCALEnvVar(ECAL_LOG_VAR))
    .Times(expected_call_count_log_var)
    .WillRepeatedly(testing::Return(""));
  EXPECT_CALL(mock_dir_provider, eCALLocalUserDir())
    .Times(expected_call_count)
    .WillRepeatedly(testing::Return(""));

  // set the tmp directory
  EXPECT_CALL(mock_dir_provider, uniqueTmpDir(::testing::Ref(mock_dir_manager)))
    .Times(1)
    .WillOnce(testing::Return(unique_tmp_dir));
  
  // let's assume all directories exist
  ON_CALL(mock_dir_manager, dirExists(testing::_)).WillByDefault(testing::Return(true));
  
  // mock the data system dir to be existent
  EXPECT_CALL(mock_dir_provider, eCALDataSystemDir(::testing::Ref(mock_dir_manager)))
    .Times(expected_call_count)
    .WillOnce(testing::Return(ecal_data_system_dir))
    .WillOnce(testing::Return(ecal_data_system_dir))
    .WillRepeatedly(testing::Return(""));

  // Testing with eCALData and eCALLog -DirImpl
  EXPECT_EQ(eCAL::Config::GeteCALDataDirImpl(mock_dir_provider, mock_dir_manager), ecal_data_system_dir);
  EXPECT_EQ(eCAL::Config::GeteCALLogDirImpl(mock_dir_provider, mock_dir_manager), ecal_data_system_log_dir);
  EXPECT_EQ(eCAL::Config::GeteCALDataDirImpl(mock_dir_provider, mock_dir_manager), "");
  
  // Log dir has tmp dir now
  EXPECT_EQ(eCAL::Config::GeteCALLogDirImpl(mock_dir_provider, mock_dir_manager), unique_tmp_dir);
}

TEST(core_cpp_path_processing /*unused*/, ecal_config_log_dir /*unused*/)
{
  const std::string ecal_log_dir = "/some/path/to/ecal";
  const std::string unique_tmp_dir = "/tmp/unique";
  const int expected_call_count = 2;

  MockDirProvider mock_dir_provider;
  NiceMock<MockDirManager> mock_dir_manager;

  // mock the environment variables to be empty
  EXPECT_CALL(mock_dir_provider, eCALEnvVar(ECAL_DATA_VAR))
    .Times(expected_call_count)
    .WillRepeatedly(testing::Return(""));
  EXPECT_CALL(mock_dir_provider, eCALEnvVar(ECAL_LOG_VAR))
    .Times(expected_call_count)
    .WillRepeatedly(testing::Return(""));
  EXPECT_CALL(mock_dir_provider, eCALLocalUserDir())
    .Times(expected_call_count)
    .WillRepeatedly(testing::Return(""));
  EXPECT_CALL(mock_dir_provider, eCALDataSystemDir(::testing::Ref(mock_dir_manager)))
    .Times(expected_call_count)
    .WillRepeatedly(testing::Return(""));

  // set the tmp directory
  EXPECT_CALL(mock_dir_provider, uniqueTmpDir(::testing::Ref(mock_dir_manager)))
    .Times(1)
    .WillOnce(testing::Return(unique_tmp_dir));
  
  // let's assume all directories exist
  ON_CALL(mock_dir_manager, dirExists(testing::_)).WillByDefault(testing::Return(true));
  
  auto config = eCAL::GetConfiguration();
  config.logging.provider.file_config.path = ecal_log_dir;

  // Testing with eCALData and eCALLog -DirImpl
  EXPECT_EQ(eCAL::Config::GeteCALLogDirImpl(mock_dir_provider, mock_dir_manager, config), ecal_log_dir);
  
  config.logging.provider.file_config.path = "";

  // Log dir has tmp dir now
  EXPECT_EQ(eCAL::Config::GeteCALLogDirImpl(mock_dir_provider, mock_dir_manager), unique_tmp_dir);
}

TEST(core_cpp_path_processing /*unused*/, ecal_log_order_test /*unused*/)
{
  const std::string ecal_data_env_var = "/ecal/data/env";
  const std::string ecal_log_env_var = "/ecal/log/env";
  const std::string ecal_data_system_dir = "/data/system/dir";
  const std::string ecal_local_user_dir = "/local/user/dir";
  const std::string ecal_config_logging_dir = "/config/logging/dir";
  const std::string unique_tmp_dir = "/tmp/unique";
  const std::string ecal_local_user_log_dir = ecal_local_user_dir + path_separator + ECAL_FOLDER_NAME_LOG;
  const std::string ecal_data_system_log_dir = ecal_data_system_dir + path_separator + ECAL_FOLDER_NAME_LOG;

  MockDirProvider mock_dir_provider;
  NiceMock<MockDirManager> mock_dir_manager;

  // mock the environment variables to be empty
  EXPECT_CALL(mock_dir_provider, eCALEnvVar(ECAL_LOG_VAR))
    .Times(1)
    .WillRepeatedly(testing::Return(ecal_log_env_var));
  EXPECT_CALL(mock_dir_provider, eCALEnvVar(ECAL_DATA_VAR))
    .Times(2)
    .WillRepeatedly(testing::Return(ecal_data_env_var));
  // log is the third one
  EXPECT_CALL(mock_dir_provider, eCALLocalUserDir())
    .Times(4)
    .WillRepeatedly(testing::Return(ecal_local_user_dir));
  EXPECT_CALL(mock_dir_provider, eCALDataSystemDir(::testing::Ref(mock_dir_manager)))
    .Times(5)
    .WillRepeatedly(testing::Return(ecal_data_system_dir));

  EXPECT_CALL(mock_dir_provider, uniqueTmpDir(::testing::Ref(mock_dir_manager)))
    .Times(1)
    .WillOnce(testing::Return(unique_tmp_dir));

  auto config = eCAL::GetConfiguration();
  config.logging.provider.file_config.path = ecal_config_logging_dir;
  // let's assume all directories exist
  ON_CALL(mock_dir_manager, dirExists(testing::_)).WillByDefault(testing::Return(true));
  
  // Testing with eCALData and eCALLog -DirImpl
  EXPECT_EQ(eCAL::Config::GeteCALLogDirImpl(mock_dir_provider, mock_dir_manager, config),  ecal_log_env_var);
  
  EXPECT_CALL(mock_dir_provider, eCALEnvVar(ECAL_LOG_VAR))
    .Times(5)
    .WillRepeatedly(testing::Return(""));
  
  EXPECT_EQ(eCAL::Config::GeteCALLogDirImpl(mock_dir_provider, mock_dir_manager, config), ecal_data_env_var);

  EXPECT_CALL(mock_dir_provider, eCALEnvVar(ECAL_DATA_VAR))
    .Times(4)
    .WillRepeatedly(testing::Return(""));

  EXPECT_EQ(eCAL::Config::GeteCALLogDirImpl(mock_dir_provider, mock_dir_manager, config), ecal_config_logging_dir);

  config.logging.provider.file_config.path = "";

  EXPECT_EQ(eCAL::Config::GeteCALLogDirImpl(mock_dir_provider, mock_dir_manager, config), ecal_local_user_log_dir);

  EXPECT_CALL(mock_dir_provider, eCALLocalUserDir())
    .Times(2)
    .WillRepeatedly(testing::Return(""));

  EXPECT_EQ(eCAL::Config::GeteCALLogDirImpl(mock_dir_provider, mock_dir_manager, config), ecal_data_system_log_dir);
  
  EXPECT_CALL(mock_dir_provider, eCALDataSystemDir(::testing::Ref(mock_dir_manager)))
    .Times(1)
    .WillRepeatedly(testing::Return(""));

  EXPECT_EQ(eCAL::Config::GeteCALLogDirImpl(mock_dir_provider, mock_dir_manager, config), unique_tmp_dir);
}