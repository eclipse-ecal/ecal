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

#include <ecal/ecal.h>

#include <gtest/gtest.h>
#include <map>
#include <set>
#include <string>

enum {
  CMN_MONITORING_TIMEOUT_MS   = (5000 + 100),
  CMN_REGISTRATION_REFRESH_MS = (1000)
};

// struct to hold the test parameters
struct ServicesTestParams
{
  eCAL::Configuration configuration;
};

// test class that accepts TestParams as a parameter
class ServicesTestFixture : public ::testing::TestWithParam<ServicesTestParams>
{
protected:
  void SetUp() override
  {
    // set configuration from the test parameters
    auto params = GetParam();
    eCAL::Initialize(params.configuration, "core_cpp_registration_public", eCAL::Init::All);
  }

  void TearDown() override
  {
    // clean up
    eCAL::Finalize();
  }
};

TEST_P(ServicesTestFixture, ServiceExpiration)
{
  std::set<eCAL::SServiceId> id_set;
  bool get_server_ids_succeeded = false;

  // create simple service and let it expire
  {
    // create service
    eCAL::CServiceServer service("foo::service");
    service.SetMethodCallback({ "foo::method",  { "foo::req_type", "foo::req_desc" }, { "foo::resp_type", "foo::resp_desc" } }, eCAL::ServiceMethodCallbackT());

    // let's register
    eCAL::Process::SleepMS(2 * CMN_REGISTRATION_REFRESH_MS);

    // get all services
    get_server_ids_succeeded = eCAL::Registration::GetServerIDs(id_set);
    EXPECT_TRUE(get_server_ids_succeeded) << "GetServerIDs call failed";

    // check size
    EXPECT_EQ(id_set.size(), 1);

    // check service/method names
    std::set<eCAL::Registration::SServiceMethod> service_method_names;
    eCAL::Registration::GetServerMethodNames(service_method_names);
    EXPECT_EQ(service_method_names.size(), 1);
    for (const auto& name : service_method_names)
    {
      EXPECT_EQ(name.service_name, "foo::service");
      EXPECT_EQ(name.method_name,  "foo::method");
    }

    // let's wait a monitoring timeout long
    eCAL::Process::SleepMS(CMN_MONITORING_TIMEOUT_MS);

    // get all services again, service should not be expired
    get_server_ids_succeeded = eCAL::Registration::GetServerIDs(id_set);
    EXPECT_TRUE(get_server_ids_succeeded) << "GetServerIDs call failed";

    // check size
    EXPECT_EQ(id_set.size(), 1);
  }

  // let's unregister
  eCAL::Process::SleepMS(2 * CMN_REGISTRATION_REFRESH_MS);

  // get all services again, all services
  // should be removed from the map
  get_server_ids_succeeded = eCAL::Registration::GetServerIDs(id_set);
  EXPECT_TRUE(get_server_ids_succeeded) << "GetServerIDs call failed";

  // check size
  EXPECT_EQ(id_set.size(), 0);
}

TEST_P(ServicesTestFixture, GetServiceIDs)
{
  // create simple server
  {
    // create server
    eCAL::CServiceServer service("foo::service");

    // add method
    eCAL::SServiceMethodInformation service_method_info;
    service_method_info.method_name = "method";
    service_method_info.request_type.name        = "foo::req_type";
    service_method_info.request_type.descriptor  = "foo::req_desc";
    service_method_info.response_type.name       = "foo::resp_type";
    service_method_info.response_type.descriptor = "foo::resp_desc";
    service.SetMethodCallback(service_method_info, eCAL::ServiceMethodCallbackT());

    // let's register
    eCAL::Process::SleepMS(2 * CMN_REGISTRATION_REFRESH_MS);

    // get server
    std::set<eCAL::SServiceId> id_set;
    const auto success = eCAL::Registration::GetServerIDs(id_set);
    EXPECT_TRUE(success) << "GetServerIDs should be successfully executed";
    EXPECT_EQ(1, id_set.size()) << "There should be 1 server in the system";
    if (id_set.size() > 0)
    {
      eCAL::ServiceMethodInformationSetT methods;
      EXPECT_TRUE(eCAL::Registration::GetServerInfo(*id_set.begin(), methods));

      // check service/method names
      EXPECT_TRUE(methods.find(service_method_info) != methods.end());
    }
  }
}

INSTANTIATE_TEST_SUITE_P(
  core_cpp_registration_public_services,
  ServicesTestFixture,
  ::testing::Values(
    ServicesTestParams{[]() {
      // shm
      eCAL::Configuration config;
      config.registration.layer.shm.enable = true;
      config.registration.layer.udp.enable = false;
      return config;
    }() },
    ServicesTestParams{ []() {
      // shm + shm transport domain
      eCAL::Configuration config;
      config.registration.layer.shm.enable     = true;
      config.registration.layer.udp.enable     = false;
      config.registration.shm_transport_domain = "abc";
      return config;
    }() },
    ServicesTestParams{[]() {
      // udp
      eCAL::Configuration config;
      config.registration.layer.shm.enable = false;
      config.registration.layer.udp.enable = true;
      return config;
    }() }
      )
);
