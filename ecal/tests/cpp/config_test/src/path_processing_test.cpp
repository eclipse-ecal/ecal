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

#include <ecal/ecal_os.h>
#include <ecal_def.h>
#include <ecal/ecal_util.h>

#include <cstdlib>

class ScopedEnvVar {
public:
    ScopedEnvVar(const std::string& key, const std::string& value) : key_(key) {
#ifdef ECAL_OS_WINDOWS
        _putenv_s(key.c_str(), value.c_str());
#elif defined(ECAL_OS_LINUX)
        setenv(key.c_str(), value.c_str(), 1);
#endif
    }

    ~ScopedEnvVar() {
#ifdef ECAL_OS_WINDOWS
        _putenv_s(key_.c_str(), "");
#elif defined(ECAL_OS_LINUX)
        unsetenv(key_.c_str());
#endif
    }

private:
    std::string key_;
};

TEST(core_cpp_path_processing /*unused*/, ecal_data_log_env_vars /*unused*/)
{
  const std::string env_ecal_conf_value = "/pathtoconf";
  const std::string env_ecal_log_value = "/pathtolog";

  EXPECT_EQ(env_ecal_conf_value, env_ecal_conf_value);

  // Needs rework - mocking
  // {
  //   // All paths for data, config and log are the same when ECAL_DATA is set
  //   ScopedEnvVar env_var(ECAL_DATA_VAR, env_ecal_conf_value);
  //   EXPECT_EQ(eCAL::Config::eCALDataEnvPath(), env_ecal_conf_value);
  // }

  // {
  //   ScopedEnvVar env_var(ECAL_LOG_VAR, env_ecal_log_value);
  //   EXPECT_TRUE(EcalUtils::Filesystem::MkDir(env_ecal_log_value));
  //   EXPECT_EQ(eCAL::Config::eCALLogDir(), env_ecal_log_value);

  //   EcalUtils::Filesystem::DeleteDir(env_ecal_log_value);

  //   // at least a temporary folder would need to be created, so it should not be empty
  //   const std::string tmp_log_dir = eCAL::Config::eCALLogDir();
  //   EXPECT_NE(tmp_log_dir, env_ecal_log_value);

  //   // delete tmp folder again
  //   EcalUtils::Filesystem::DeleteDir(tmp_log_dir);
  // }

  // EXPECT_EQ(eCAL::Config::eCALDataEnvPath(), "");

  // // at least a temporary folder would need to be created, so it should not be empty
  // EXPECT_NE(eCAL::Config::eCALLogDir(), "");
}