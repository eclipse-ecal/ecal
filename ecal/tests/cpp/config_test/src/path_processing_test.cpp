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

#include <ecal/os.h>
#include <ecal_def.h>
#include <ecal/util.h>

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
    MOCK_METHOD(bool, canWriteToDirectory, (const std::string& path_), (const, override));
    MOCK_METHOD(std::string, getDirectoryPath, (const std::string& file_path_), (const, override));
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
  const std::string env_ecal_conf_log_value = env_ecal_conf_value + path_separator + ECAL_FOLDER_NAME_LOG;
  const std::string env_ecal_log_value = "/path/to/log";
  const std::string unique_tmp_dir = "/tmp/unique";

  const int expected_call_count_data_dir = 2;
  const int expected_call_count_log_dir = 3;
  const int expected_call_count = expected_call_count_data_dir + expected_call_count_log_dir;

  const MockDirProvider mock_dir_provider;
  const NiceMock<MockDirManager> mock_dir_manager;

  // Disable all other calls right now, as we just want to test the env vars
  EXPECT_CALL(mock_dir_provider, eCALDataSystemDir(::testing::Ref(mock_dir_manager)))
    .Times(expected_call_count_data_dir)
    .WillRepeatedly(testing::Return(""));
  EXPECT_CALL(mock_dir_provider, eCALLocalUserDir())
    .Times(expected_call_count_data_dir)
    .WillRepeatedly(testing::Return(""));
  
  // set the tmp directory
  EXPECT_CALL(mock_dir_provider, uniqueTmpDir(::testing::Ref(mock_dir_manager)))
    .Times(1)
    .WillOnce(testing::Return(unique_tmp_dir));

  // let's assume all directories exist
  ON_CALL(mock_dir_manager, dirExists(testing::_)).WillByDefault(testing::Return(true));
  ON_CALL(mock_dir_manager, canWriteToDirectory(testing::_)).WillByDefault(testing::Return(true));

  // mock the environment variables to be existent
  EXPECT_CALL(mock_dir_provider, eCALEnvVar(ECAL_DATA_VAR))
    .Times(expected_call_count)
    .WillOnce(testing::Return(env_ecal_conf_value))
    .WillOnce(testing::Return(env_ecal_conf_value))
    .WillOnce(testing::Return(env_ecal_conf_value))
    .WillRepeatedly(testing::Return(""));
  EXPECT_CALL(mock_dir_provider, eCALEnvVar(ECAL_LOG_VAR))
    .Times(expected_call_count_log_dir)
    .WillOnce(testing::Return(env_ecal_log_value))
    .WillRepeatedly(testing::Return(""));

  EXPECT_EQ(eCAL::Config::GeteCALDataDirImpl(mock_dir_provider, mock_dir_manager), env_ecal_conf_value);
  EXPECT_EQ(eCAL::Config::GeteCALLogDirImpl(mock_dir_provider, mock_dir_manager), env_ecal_log_value);
  EXPECT_EQ(eCAL::Config::GeteCALLogDirImpl(mock_dir_provider, mock_dir_manager), env_ecal_conf_log_value);
  
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

  const int expected_call_count_data_dir = 2;

  const MockDirProvider mock_dir_provider;
  const NiceMock<MockDirManager> mock_dir_manager;

  // mock the environment variables to be empty
  EXPECT_CALL(mock_dir_provider, eCALEnvVar(ECAL_DATA_VAR))
    .Times(expected_call_count_data_dir)
    .WillRepeatedly(testing::Return(""));
  EXPECT_CALL(mock_dir_provider, eCALEnvVar(ECAL_LOG_VAR))
    .Times(0);
  EXPECT_CALL(mock_dir_provider, eCALDataSystemDir(::testing::Ref(mock_dir_manager)))
    .Times(expected_call_count_data_dir)
    .WillRepeatedly(testing::Return(""));
  
  // set the tmp directory
  EXPECT_CALL(mock_dir_provider, uniqueTmpDir(::testing::Ref(mock_dir_manager)))
    .Times(0);

  // let's assume all directories exist
  ON_CALL(mock_dir_manager, dirExists(testing::_)).WillByDefault(testing::Return(true));
  ON_CALL(mock_dir_manager, canWriteToDirectory(testing::_)).WillByDefault(testing::Return(true));

  // mock the local user dir to be existent
  EXPECT_CALL(mock_dir_provider, eCALLocalUserDir())
    .Times(expected_call_count_data_dir)
    .WillOnce(testing::Return(ecal_local_user_dir))
    .WillRepeatedly(testing::Return(""));

  // Testing with eCALData and eCALLog -DirImpl
  EXPECT_EQ(eCAL::Config::GeteCALDataDirImpl(mock_dir_provider, mock_dir_manager), ecal_local_user_dir);
  EXPECT_EQ(eCAL::Config::GeteCALDataDirImpl(mock_dir_provider, mock_dir_manager), "");
}

TEST(core_cpp_path_processing /*unused*/, ecal_data_system_dir /*unused*/)
{
  const std::string ecal_data_system_dir = "/data/system/dir";
  const std::string ecal_data_system_log_dir = ecal_data_system_dir + path_separator + ECAL_FOLDER_NAME_LOG;
  const std::string unique_tmp_dir = "/tmp/unique";
  
  const int expected_call_count = 2;

  const MockDirProvider mock_dir_provider;
  const NiceMock<MockDirManager> mock_dir_manager;

  // mock the environment variables to be empty
  EXPECT_CALL(mock_dir_provider, eCALEnvVar(ECAL_DATA_VAR))
    .Times(expected_call_count)
    .WillRepeatedly(testing::Return(""));
  EXPECT_CALL(mock_dir_provider, eCALEnvVar(ECAL_LOG_VAR))
    .Times(0);
  EXPECT_CALL(mock_dir_provider, eCALLocalUserDir())
    .Times(expected_call_count)
    .WillRepeatedly(testing::Return(""));

  // set the tmp directory
  EXPECT_CALL(mock_dir_provider, uniqueTmpDir(::testing::Ref(mock_dir_manager)))
    .Times(0);
  
  // let's assume all directories exist
  ON_CALL(mock_dir_manager, dirExists(testing::_)).WillByDefault(testing::Return(true));
  ON_CALL(mock_dir_manager, canWriteToDirectory(testing::_)).WillByDefault(testing::Return(true));
  
  // mock the data system dir to be existent
  EXPECT_CALL(mock_dir_provider, eCALDataSystemDir(::testing::Ref(mock_dir_manager)))
    .Times(expected_call_count)
    .WillOnce(testing::Return(ecal_data_system_dir))
    .WillRepeatedly(testing::Return(""));

  // Testing with eCALData and eCALLog -DirImpl
  EXPECT_EQ(eCAL::Config::GeteCALDataDirImpl(mock_dir_provider, mock_dir_manager), ecal_data_system_dir);
  EXPECT_EQ(eCAL::Config::GeteCALDataDirImpl(mock_dir_provider, mock_dir_manager), "");
}

TEST(core_cpp_path_processing /*unused*/, ecal_config_log_dir /*unused*/)
{
  const std::string ecal_log_dir = "/some/path/to/ecal";
  const std::string unique_tmp_dir = "/tmp/unique";
  const int expected_call_count = 2;

  const MockDirProvider mock_dir_provider;
  const NiceMock<MockDirManager> mock_dir_manager;

  // mock the environment variables to be empty
  EXPECT_CALL(mock_dir_provider, eCALEnvVar(ECAL_DATA_VAR))
    .Times(expected_call_count)
    .WillRepeatedly(testing::Return(""));
  EXPECT_CALL(mock_dir_provider, eCALEnvVar(ECAL_LOG_VAR))
    .Times(expected_call_count)
    .WillRepeatedly(testing::Return(""));
  EXPECT_CALL(mock_dir_provider, eCALLocalUserDir())
    .Times(0);
  EXPECT_CALL(mock_dir_provider, eCALDataSystemDir(::testing::Ref(mock_dir_manager)))
    .Times(0);

  // set the tmp directory
  EXPECT_CALL(mock_dir_provider, uniqueTmpDir(::testing::Ref(mock_dir_manager)))
    .Times(1)
    .WillOnce(testing::Return(unique_tmp_dir));
  
  // let's assume all directories exist
  ON_CALL(mock_dir_manager, dirExists(testing::_)).WillByDefault(testing::Return(true));
  ON_CALL(mock_dir_manager, canWriteToDirectory(testing::_)).WillByDefault(testing::Return(true));
  
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
  // This test checks multiple scenarios for retrieving the log directory.
  // Every variable will be set from the beginning to a value. Now, compared to the other tests,
  // the return value of "dirExists" can also return false.
  // So the first call will return first value. Value is set and direxists.
  // The second call will return the second value, even if the variable is still set, because dirExists will return false.
  // The next call will return the third value, and so on.
  // This continues until the last: UniqueTmpDir call
  // Alltogether the eCALLogDirImpl function will be called 7 times.

  const std::string ecal_log_env_var = "/ecal/log/env";
  const std::string ecal_data_env_var = "/ecal/data/env";
  const std::string ecal_data_env_log_var = ecal_data_env_var + path_separator + ECAL_FOLDER_NAME_LOG;
  const std::string ecal_config_logging_dir = "/config/logging/dir";
  const std::string ecal_yaml_dir = "/dir/to/current/yaml";
  const std::string ecal_yaml_log_dir = ecal_yaml_dir + path_separator + ECAL_FOLDER_NAME_LOG;
  const std::string unique_tmp_dir = "/tmp/unique";

  const MockDirProvider mock_dir_provider;
  const NiceMock<MockDirManager> mock_dir_manager;

  ::testing::Sequence seq_env_log, seq_env_data, yaml_dir;

  // this gets called twice with path set, 2nd time path is set but dirExists returns false
  EXPECT_CALL(mock_dir_provider, eCALEnvVar(ECAL_LOG_VAR))
    .Times(2)
    .InSequence(seq_env_log)
    .WillRepeatedly(testing::Return(ecal_log_env_var));
  EXPECT_CALL(mock_dir_provider, eCALEnvVar(ECAL_LOG_VAR))
    .Times(5)
    .InSequence(seq_env_log)
    .WillRepeatedly(testing::Return(""));
  EXPECT_CALL(mock_dir_manager, dirExists(ecal_log_env_var))
    .Times(2)
    .WillOnce(testing::Return(true))
    .WillRepeatedly(testing::Return(false));

  // this will get called 4 times with path set, because 2 times with log subfolder
  EXPECT_CALL(mock_dir_provider, eCALEnvVar(ECAL_DATA_VAR))
    .Times(4)
    .InSequence(seq_env_data)
    .WillRepeatedly(testing::Return(ecal_data_env_var));
  EXPECT_CALL(mock_dir_provider, eCALEnvVar(ECAL_DATA_VAR))
    .Times(3)
    .InSequence(seq_env_data)
    .WillRepeatedly(testing::Return(""));
  EXPECT_CALL(mock_dir_manager, dirExists(ecal_data_env_log_var))
    .Times(3)
    .WillOnce(testing::Return(true))
    .WillRepeatedly(testing::Return(false));
  EXPECT_CALL(mock_dir_manager, dirExists(ecal_data_env_var))
    .Times(2)
    .WillOnce(testing::Return(true))
    .WillRepeatedly(testing::Return(false));

  // logdir from config is the third one
  EXPECT_CALL(mock_dir_manager, dirExists(ecal_config_logging_dir))
    .Times(1)
    .WillOnce(testing::Return(true));

  // used ecal.yaml directory
  EXPECT_CALL(mock_dir_manager, getDirectoryPath(testing::_))
    .Times(6)
    .InSequence(yaml_dir)
    .WillRepeatedly(testing::Return(ecal_yaml_dir));
  EXPECT_CALL(mock_dir_manager, getDirectoryPath(testing::_))
    .Times(1)
    .InSequence(yaml_dir)
    .WillRepeatedly(testing::Return(""));
  EXPECT_CALL(mock_dir_manager, dirExists(ecal_yaml_log_dir))
    .Times(2)
    .WillOnce(testing::Return(true))
    .WillRepeatedly(testing::Return(false));
  EXPECT_CALL(mock_dir_manager, dirExists(ecal_yaml_dir))
    .Times(1)
    .WillOnce(testing::Return(true));

  EXPECT_CALL(mock_dir_provider, uniqueTmpDir(::testing::Ref(mock_dir_manager)))
    .Times(1)
    .WillRepeatedly(testing::Return(unique_tmp_dir));

  auto config = eCAL::GetConfiguration();
  config.logging.provider.file_config.path = ecal_config_logging_dir;
  
  // let's assume all directories exist
  ON_CALL(mock_dir_manager, dirExists(testing::_)).WillByDefault(testing::Return(false));
  ON_CALL(mock_dir_manager, canWriteToDirectory(testing::_)).WillByDefault(testing::Return(true));
  
  // Testing with eCALData and eCALLog -DirImpl
  EXPECT_EQ(eCAL::Config::GeteCALLogDirImpl(mock_dir_provider, mock_dir_manager, config), ecal_log_env_var);
  EXPECT_EQ(eCAL::Config::GeteCALLogDirImpl(mock_dir_provider, mock_dir_manager, config), ecal_data_env_log_var);
  EXPECT_EQ(eCAL::Config::GeteCALLogDirImpl(mock_dir_provider, mock_dir_manager, config), ecal_data_env_var);
  EXPECT_EQ(eCAL::Config::GeteCALLogDirImpl(mock_dir_provider, mock_dir_manager, config), ecal_config_logging_dir);
  config.logging.provider.file_config.path = "";
  EXPECT_EQ(eCAL::Config::GeteCALLogDirImpl(mock_dir_provider, mock_dir_manager, config), ecal_yaml_log_dir);
  EXPECT_EQ(eCAL::Config::GeteCALLogDirImpl(mock_dir_provider, mock_dir_manager, config), ecal_yaml_dir);
  EXPECT_EQ(eCAL::Config::GeteCALLogDirImpl(mock_dir_provider, mock_dir_manager, config), unique_tmp_dir);
}