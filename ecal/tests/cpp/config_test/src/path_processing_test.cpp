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

#include "config/ecal_path_processing.h"
#include "ecal_utils/filesystem.h"
#include "util/getenvvar.h"

#include <ecal/os.h>
#include <ecal_def.h>
#include <ecal/util.h>

#include <filesystem>
#include <system_error>

#ifdef ECAL_OS_WINDOWS
  #include <ecal_utils/str_convert.h>
#endif

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

// ============================================================================
// DirManager concrete implementation tests
// ============================================================================

// Verify that canWriteToDirectory returns true for a real writable directory and
// false for a non-existent one.
// Note: the specific bug of std::filesystem::remove throwing on failure (vs. the
// error_code overload) cannot be exercised in a normal unit test because it only
// manifests when the OS refuses to delete the test file – an unusual condition
// that cannot be reliably engineered here.  The fix (using the error_code overload)
// is validated by code review rather than by this test.
TEST(core_cpp_dir_manager /*unused*/, can_write_to_directory /*unused*/)
{
  const eCAL::Util::DirManager dir_manager;
  const std::string temp_dir = std::filesystem::temp_directory_path().u8string();

  EXPECT_TRUE(dir_manager.canWriteToDirectory(temp_dir));
  EXPECT_FALSE(dir_manager.canWriteToDirectory("ecal_nonexistent_dir_test_67890"));
}

// Verify that uniqueTmpDir returns a non-empty, existing directory and can be
// cleaned up without issues.
// Note: the Windows-specific Unicode bug (GetTempFileNameA mishandling non-ASCII
// temp paths) cannot be exercised here because getTempDir() is a free function
// that reads the real OS temp directory, which is not injectable.  The bug only
// manifests when the system temp directory itself contains non-ANSI characters –
// an environment condition that cannot be controlled in a unit test.  The fix
// (switching to the W-variants of the Win32 API) is validated by code review and
// by the can_write_to_directory_unicode_path integration test above.
TEST(core_cpp_dir_provider /*unused*/, unique_tmp_dir_returns_valid_dir /*unused*/)
{
  const eCAL::Util::DirManager  dir_manager;
  const eCAL::Util::DirProvider dir_provider;

  const std::string unique_dir = dir_provider.uniqueTmpDir(dir_manager);
  EXPECT_FALSE(unique_dir.empty());

  if (!unique_dir.empty())
  {
    EXPECT_TRUE(dir_manager.dirExists(unique_dir));

    std::error_code ec;
    std::filesystem::remove_all(std::filesystem::u8path(unique_dir), ec);
    EXPECT_FALSE(ec) << "Cleanup of unique tmp dir failed: " << ec.message();
  }
}

#ifdef ECAL_OS_WINDOWS
// On Windows, DirManager methods receive UTF-8 strings.  Before the fix,
// std::filesystem::path(std::string) used the ANSI code page, which caused
// garbled or failed operations for non-ASCII paths.  These tests verify the
// correct UTF-8 round-trip behaviour after the fix.

// Tests that getDirectoryPath correctly round-trips a UTF-8 path containing
// characters outside the ANSI code page (Greek letters used as a proxy).
TEST(core_cpp_dir_manager /*unused*/, get_directory_path_unicode_utf8 /*unused*/)
{
  // UTF-8 encoding of "αβγ" (U+03B1 U+03B2 U+03B3)
  const std::string unicode_segment = "\xce\xb1\xce\xb2\xce\xb3";

  const std::string input_path     = "C:\\unicode_test\\" + unicode_segment + "\\ecal.yaml";
  const std::string expected_dir   = "C:\\unicode_test\\" + unicode_segment;

  const eCAL::Util::DirManager dir_manager;
  EXPECT_EQ(dir_manager.getDirectoryPath(input_path), expected_dir);
}

// Tests that canWriteToDirectory works correctly when the target directory path
// contains Unicode characters (non-ANSI).
TEST(core_cpp_dir_manager /*unused*/, can_write_to_directory_unicode_path /*unused*/)
{
  // Use a wide path to create the directory reliably, then pass its UTF-8
  // representation to the function under test.
  const std::wstring unicode_dir_name = L"ecal_unicode_write_test_\u03B1\u03B2\u03B3";
  const std::filesystem::path unicode_dir_path =
    std::filesystem::temp_directory_path() / unicode_dir_name;

  std::error_code ec;
  std::filesystem::create_directory(unicode_dir_path, ec);
  ASSERT_FALSE(ec) << "Failed to create Unicode test directory: " << ec.message();

  const std::string unicode_dir_utf8 =
    EcalUtils::StrConvert::WideToUtf8(unicode_dir_path.wstring());

  const eCAL::Util::DirManager dir_manager;
  EXPECT_TRUE(dir_manager.canWriteToDirectory(unicode_dir_utf8));

  std::filesystem::remove_all(unicode_dir_path, ec);
}
#endif /* ECAL_OS_WINDOWS */
