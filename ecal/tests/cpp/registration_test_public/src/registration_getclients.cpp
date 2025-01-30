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
struct ClientsTestParams
{
  eCAL::Configuration configuration;
};

// test class that accepts TestParams as a parameter
class ClientsTestFixture : public ::testing::TestWithParam<ClientsTestParams>
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
TEST_P(ClientsTestFixture, ClientExpiration)
{
  std::set<eCAL::SServiceId> id_set;
  bool get_client_ids_succeeded = false;

  // create simple client and let it expire
  {
    // create client
    eCAL::SServiceMethodInformation service_method_info;
    service_method_info.method_name              = "foo::method";
    service_method_info.request_type.name        = "foo::req_type";
    service_method_info.request_type.descriptor  = "foo::req_desc";
    service_method_info.response_type.name       = "foo::resp_type";
    service_method_info.response_type.descriptor = "foo::resp_desc";
    const eCAL::CServiceClient client("foo::service", { service_method_info });

    // let's register
    eCAL::Process::SleepMS(2 * CMN_REGISTRATION_REFRESH_MS);

    // get all clients
    get_client_ids_succeeded = eCAL::Registration::GetClientIDs(id_set);
    EXPECT_TRUE(get_client_ids_succeeded) << "GetClientIDs call failed";

    // check size
    EXPECT_EQ(id_set.size(), 1);

    // check client/method names
    std::set<eCAL::Registration::SServiceMethod> client_method_names;
    eCAL::Registration::GetClientMethodNames(client_method_names);
    EXPECT_EQ(client_method_names.size(), 1);
    for (const auto& name : client_method_names)
    {
      EXPECT_EQ(name.service_name, "foo::service");
      EXPECT_EQ(name.method_name,  "foo::method");
    }

    // let's wait a monitoring timeout long
    eCAL::Process::SleepMS(CMN_MONITORING_TIMEOUT_MS);

    // get all clients again, client should not be expired
    get_client_ids_succeeded = eCAL::Registration::GetClientIDs(id_set);
    EXPECT_TRUE(get_client_ids_succeeded) << "GetClientIDs call failed";

    // check size
    EXPECT_EQ(id_set.size(), 1);
  }

  // let's unregister
  eCAL::Process::SleepMS(2 * CMN_REGISTRATION_REFRESH_MS);

  // get all clients again, all clients 
  // should be removed from the map
  get_client_ids_succeeded = eCAL::Registration::GetClientIDs(id_set);
  EXPECT_TRUE(get_client_ids_succeeded) << "GetClientIDs call failed";

  // check size
  EXPECT_EQ(id_set.size(), 0);
}

TEST_P(ClientsTestFixture, GetClientIDs)
{
  // create simple client
  {
    // create client
    eCAL::SServiceMethodInformation service_method_info;
    service_method_info.method_name          = "foo::method";
    service_method_info.request_type.name        = "foo::req_type";
    service_method_info.request_type.descriptor  = "foo::req_desc";
    service_method_info.response_type.name       = "foo::resp_type";
    service_method_info.response_type.descriptor = "foo::resp_desc";
    const eCAL::CServiceClient client("foo::service", { service_method_info });

    // let's register
    eCAL::Process::SleepMS(2 * CMN_REGISTRATION_REFRESH_MS);

    // get client
    std::set<eCAL::SServiceId> id_set;
    auto call_successful = eCAL::Registration::GetClientIDs(id_set);
    EXPECT_TRUE(call_successful);
    EXPECT_EQ(1, id_set.size());
    if (id_set.size() > 0)
    {
      eCAL::ServiceMethodInformationSetT info;
      EXPECT_TRUE(eCAL::Registration::GetClientInfo(*id_set.begin(), info));

      // check service/method names
      EXPECT_TRUE(info.find(service_method_info) != info.end());
    }
  }
}

INSTANTIATE_TEST_SUITE_P(
  core_cpp_registration_public_clients,
  ClientsTestFixture,
  ::testing::Values(
    ClientsTestParams{ []() {
      // shm
      eCAL::Configuration config;
      config.registration.layer.shm.enable = true;
      config.registration.layer.udp.enable = false;
      return config;
    }() },
    ClientsTestParams{ []() {
      // shm + shm transport domain
      eCAL::Configuration config;
      config.registration.layer.shm.enable     = true;
      config.registration.layer.udp.enable     = false;
      config.registration.shm_transport_domain = "abc";
      return config;
    }() },
      ClientsTestParams{ []() {
      // udp
      eCAL::Configuration config;
      config.registration.layer.shm.enable = false;
      config.registration.layer.udp.enable = true;
      return config;
    }() }
      )
);
