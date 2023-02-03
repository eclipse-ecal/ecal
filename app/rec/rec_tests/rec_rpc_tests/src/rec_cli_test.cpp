/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2019 Continental Corporation
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

#include <ecal/ecal.h>

#include <string>
#include <chrono>
#include <thread>
#include <cstdint>

#include <gtest/gtest.h>

#include <iostream>

#include "external_ecal_rec.h"

struct TestCase
{
  std::string cli_cmdline_remotecontrol;
  eCAL::rec_server::RecServerConfig expected_config;
};

std::vector<TestCase> getRpcConfigTestcases()
{
  std::vector<TestCase> testcases;

  eCAL::rec_server::RecServerConfig config;

  // --set-client
  {
    testcases.emplace_back();

    config.enabled_clients_config_["A"].host_filter_.insert("A");

    testcases.back().cli_cmdline_remotecontrol = "--set-client A:A";
    testcases.back().expected_config = config;
  }

  // --set-client
  {
    testcases.emplace_back();

    config.enabled_clients_config_["A"].host_filter_.erase("A");
    config.enabled_clients_config_["A"].host_filter_.insert("B");
    config.enabled_clients_config_["A"].host_filter_.insert("C");
    config.enabled_clients_config_["B"];

    testcases.back().cli_cmdline_remotecontrol = "--set-client A:B,C --set-client B";
    testcases.back().expected_config = config;
  }

  {
    testcases.emplace_back();

    config.enabled_clients_config_["A"].enabled_addons_.insert("A1");
    config.enabled_clients_config_["A"].enabled_addons_.insert("A2");

    config.enabled_clients_config_["B"].enabled_addons_.insert("B1");

    testcases.back().cli_cmdline_remotecontrol = "--set-addons A:A1,A2 --set-addons B:B1";
    testcases.back().expected_config = config;
  }

  {
    testcases.emplace_back();

    config.enabled_clients_config_.erase("B");
    config.enabled_clients_config_["C"];

    testcases.back().cli_cmdline_remotecontrol = "--remove-client B --set-client C";
    testcases.back().expected_config = config;
  }

  {
    testcases.emplace_back();

    config.listed_topics_.insert("t1");
    config.listed_topics_.insert("t2");
    config.listed_topics_.insert("t3");
    config.record_mode_ = eCAL::rec::RecordMode::Blacklist;

    testcases.back().cli_cmdline_remotecontrol = "--blacklist t1,t2,t3";
    testcases.back().expected_config = config;
  }

  {
    testcases.emplace_back();

    config.listed_topics_.clear();
    config.listed_topics_.insert("mytopic");
    config.record_mode_ = eCAL::rec::RecordMode::Whitelist;

    testcases.back().cli_cmdline_remotecontrol = "--whitelist mytopic";
    testcases.back().expected_config = config;
  }

  {
    testcases.emplace_back();

    config.root_dir_      = "root_dir";
    config.meas_name_     = "my_meas_name";
    config.max_file_size_ = 1337;
    config.description_   = "HelloWorld";

    testcases.back().cli_cmdline_remotecontrol = "--meas-root-dir root_dir --meas-name my_meas_name --max-file-size 1337 --description HelloWorld";
    testcases.back().expected_config = config;
  }

  {
    testcases.emplace_back();

    config.one_file_per_topic_ = true;

    testcases.back().cli_cmdline_remotecontrol = "--enable-one-file-per-topic yes";
    testcases.back().expected_config = config;
  }

  {
    testcases.emplace_back();

    config.upload_config_.type_      = eCAL::rec_server::UploadConfig::Type::FTP;
    config.upload_config_.username_  = "user";
    config.upload_config_.password_  = "pass";
    config.upload_config_.host_      = "host";
    config.upload_config_.port_      = 2121;
    config.upload_config_.root_path_ = "/ftp/path";
    config.upload_config_.delete_after_upload_ = true;

    testcases.back().cli_cmdline_remotecontrol = "--ftp-server ftp://user:pass@host:2121/ftp/path --delete-after-upload";
    testcases.back().expected_config = config;
  }

  return testcases;
}

TEST(EcalRecCli, NoDefault)
{
  eCAL::rec_server::RecServerConfig config;

  auto rec = ExternalEcalRecInstance();

  EXPECT_EQ(rec.GetConfigViaRpc(config), eCAL::rec::Error::OK);
  EXPECT_EQ(config, eCAL::rec_server::RecServerConfig());
}

TEST(EcalRecCli, SetConfigViaRPC)
{
  auto test_cases = getRpcConfigTestcases();
  auto rec = ExternalEcalRecInstance();

  for (const auto& testcase : test_cases)
  {
    std::string command_line = ECAL_REC_CLI_PATH + std::string(" --remote-control ") + testcase.cli_cmdline_remotecontrol;
    std::cout << "Running command line: " << command_line << std::endl;

    system((std::string(ECAL_REC_CLI_PATH) + " --remote-control " + testcase.cli_cmdline_remotecontrol).c_str());

    eCAL::rec_server::RecServerConfig config_result;
    EXPECT_EQ(rec.GetConfigViaRpc(config_result), eCAL::rec::Error::OK);
    EXPECT_EQ(config_result, testcase.expected_config);
  }
}

TEST(EcalRecGui, SetConfigViaRPC)
{
  auto test_cases = getRpcConfigTestcases();
  auto rec = ExternalEcalRecInstance(true);

  for (const auto& testcase : test_cases)
  {
    std::string command_line = ECAL_REC_CLI_PATH + std::string(" --remote-control ") + testcase.cli_cmdline_remotecontrol;
    std::cout << "Running command line: " << command_line << std::endl;

    system(command_line.c_str());

    eCAL::rec_server::RecServerConfig config_result;
    EXPECT_EQ(rec.GetConfigViaRpc(config_result), eCAL::rec::Error::OK);
    EXPECT_EQ(config_result, testcase.expected_config);
  }
}