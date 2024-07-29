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

#include <ecal/ecal.h>

#include <gtest/gtest.h>

enum {
  CMN_MONITORING_TIMEOUT_MS   = (5000 + 100),
  CMN_REGISTRATION_REFRESH_MS = (1000)
};

TEST(core_cpp_util, ClientExpiration)
{
  // initialize eCAL API
  eCAL::Initialize(0, nullptr, "core_cpp_util");

  // enable loop back communication in the same process
  eCAL::Util::EnableLoopback(true);

  std::map<eCAL::Util::SServiceMethod, eCAL::SServiceMethodInformation> client_info_map;

  // create simple client and let it expire
  {
    // create client
    eCAL::SServiceMethodInformation service_method_info;
    service_method_info.request_type.name        = "foo::req_type";
    service_method_info.request_type.descriptor  = "foo::req_desc";
    service_method_info.response_type.name       = "foo::resp_type";
    service_method_info.response_type.descriptor = "foo::resp_desc";
    const eCAL::CServiceClient client("foo::service", { {"foo::method", service_method_info} });

    // let's register
    eCAL::Process::SleepMS(2 * CMN_REGISTRATION_REFRESH_MS);

    // get all clients
    eCAL::Util::GetClients(client_info_map);

    // check size
    EXPECT_EQ(client_info_map.size(), 1);

    // check client/method names
    std::set<eCAL::Util::SServiceMethod> client_method_names;
    eCAL::Util::GetClientMethodNames(client_method_names);
    EXPECT_EQ(client_method_names.size(), 1);
    for (const auto& name : client_method_names)
    {
      EXPECT_EQ(name.service_name, "foo::service");
      EXPECT_EQ(name.method_name,  "foo::method");
    }

    // let's wait a monitoring timeout long
    eCAL::Process::SleepMS(CMN_MONITORING_TIMEOUT_MS);

    // get all clients again, client should not be expired
    eCAL::Util::GetClients(client_info_map);

    // check size
    EXPECT_EQ(client_info_map.size(), 1);
  }

  // let's unregister
  eCAL::Process::SleepMS(2 * CMN_REGISTRATION_REFRESH_MS);

  // get all clients again, all clients 
  // should be removed from the map
  eCAL::Util::GetClients(client_info_map);

  // check size
  EXPECT_EQ(client_info_map.size(), 0);

  // finalize eCAL API
  eCAL::Finalize();
}

TEST(core_cpp_util, ClientEqualQualities)
{
  // initialize eCAL API
  eCAL::Initialize(0, nullptr, "core_cpp_util");

  // enable loop back communication in the same process
  eCAL::Util::EnableLoopback(true);

  std::map<eCAL::Util::SServiceMethod, eCAL::SServiceMethodInformation> client_info_map;

  // create 2 clients with the same quality of data type information
  {
    // create client 1
    eCAL::SServiceMethodInformation service_method_info1;
    service_method_info1.request_type.name        = "foo::req_type1";
    service_method_info1.request_type.descriptor  = "foo::req_desc1";
    service_method_info1.response_type.name       = "foo::resp_type1";
    service_method_info1.response_type.descriptor = "foo::resp_desc1";
    eCAL::CServiceClient client1("foo::service", { {"foo::method", service_method_info1} });

    // let's register
    eCAL::Process::SleepMS(2 * CMN_REGISTRATION_REFRESH_MS);

    // get all clients
    eCAL::Util::GetClients(client_info_map);

    // check size
    EXPECT_EQ(client_info_map.size(), 1);

    // check attributes
    std::string req_type, resp_type;
    std::string req_desc, resp_desc;

    eCAL::Util::GetClientTypeNames("foo::service", "foo::method", req_type, resp_type);
    EXPECT_EQ(req_type,  "foo::req_type1");
    EXPECT_EQ(resp_type, "foo::resp_type1");
    eCAL::Util::GetClientDescription("foo::service", "foo::method", req_desc, resp_desc);
    EXPECT_EQ(req_desc,  "foo::req_desc1");
    EXPECT_EQ(resp_desc, "foo::resp_desc1");

    // create client 2
    // this will not overwrite the attributes from client 1, because the quality is not higher
    eCAL::SServiceMethodInformation service_method_info2;
    service_method_info2.request_type.name        = "foo::req_type2";
    service_method_info2.request_type.descriptor  = "foo::req_desc2";
    service_method_info2.response_type.name       = "foo::resp_type2";
    service_method_info2.response_type.descriptor = "foo::resp_desc2";
    eCAL::CServiceClient client2("foo::service", { {"foo::method", service_method_info2} });

    // check attributes
    eCAL::Util::GetClientTypeNames("foo::service", "foo::method", req_type, resp_type);
    EXPECT_EQ(req_type,  "foo::req_type1");
    EXPECT_EQ(resp_type, "foo::resp_type1");
    eCAL::Util::GetClientDescription("foo::service", "foo::method", req_desc, resp_desc);
    EXPECT_EQ(req_desc,  "foo::req_desc1");
    EXPECT_EQ(resp_desc, "foo::resp_desc1");

    // check size it's client 1 only
    EXPECT_EQ(client_info_map.size(), 1);

    // let's wait a monitoring timeout long
    eCAL::Process::SleepMS(CMN_MONITORING_TIMEOUT_MS);

    // get all clients again, clients should not be expired
    eCAL::Util::GetClients(client_info_map);

    // check size
    EXPECT_EQ(client_info_map.size(), 1);

    // destroy client 1
    client1.Destroy();

    // let's register
    eCAL::Process::SleepMS(2 * CMN_REGISTRATION_REFRESH_MS);

    // check attributes, client 1 attributes should be replaced by client 2 attributes now
    eCAL::Util::GetClientTypeNames("foo::service", "foo::method", req_type, resp_type);
    EXPECT_EQ(req_type,  "foo::req_type2");
    EXPECT_EQ(resp_type, "foo::resp_type2");
    eCAL::Util::GetClientDescription("foo::service", "foo::method", req_desc, resp_desc);
    EXPECT_EQ(req_desc,  "foo::req_desc2");
    EXPECT_EQ(resp_desc, "foo::resp_desc2");
  }

  // let's unregister
  eCAL::Process::SleepMS(2 * CMN_REGISTRATION_REFRESH_MS);

  // get all clients again, all clients 
  // should be removed from the map
  eCAL::Util::GetClients(client_info_map);

  // check size
  EXPECT_EQ(client_info_map.size(), 0);

  // finalize eCAL API
  eCAL::Finalize();
}

TEST(core_cpp_util, ClientDifferentQualities)
{
  // initialize eCAL API
  eCAL::Initialize(0, nullptr, "core_cpp_util");

  // enable loop back communication in the same process
  eCAL::Util::EnableLoopback(true);

  std::map<eCAL::Util::SServiceMethod, eCAL::SServiceMethodInformation> client_info_map;

  // create 2 clients with different qualities of data type information
  {
    // create client 1, response type name and reponse description are missing
    eCAL::SServiceMethodInformation service_method_info1;
    service_method_info1.request_type.name        = "foo::req_type1";
    service_method_info1.request_type.descriptor  = "foo::req_desc1";
    service_method_info1.response_type.name       = "";
    service_method_info1.response_type.descriptor = "";
    eCAL::CServiceClient client1("foo::service", { {"foo::method", service_method_info1} });

    // let's register
    eCAL::Process::SleepMS(2 * CMN_REGISTRATION_REFRESH_MS);

    // get all clients
    eCAL::Util::GetClients(client_info_map);

    // check size
    EXPECT_EQ(client_info_map.size(), 1);

    // check attributes
    std::string req_type, resp_type;
    std::string req_desc, resp_desc;

    eCAL::Util::GetClientTypeNames("foo::service", "foo::method", req_type, resp_type);
    EXPECT_EQ(req_type,  "foo::req_type1");
    EXPECT_EQ(resp_type, "");
    eCAL::Util::GetClientDescription("foo::service", "foo::method", req_desc, resp_desc);
    EXPECT_EQ(req_desc,  "foo::req_desc1");
    EXPECT_EQ(resp_desc, "");

    // create client 2, with higher quality than client 1
    eCAL::SServiceMethodInformation service_method_info2;
    service_method_info2.request_type.name        = "foo::req_type2";
    service_method_info2.request_type.descriptor  = "foo::req_desc2";
    service_method_info2.response_type.name       = "foo::resp_type2";
    service_method_info2.response_type.descriptor = "foo::resp_desc2";
    eCAL::CServiceClient client2("foo::service", { {"foo::method", service_method_info2} });

    // let's register
    eCAL::Process::SleepMS(2 * CMN_REGISTRATION_REFRESH_MS);

    // check attributes, we expect attributes from client 2 here
    eCAL::Util::GetClientTypeNames("foo::service", "foo::method", req_type, resp_type);
    EXPECT_EQ(req_type,  "foo::req_type2");
    EXPECT_EQ(resp_type, "foo::resp_type2");
    eCAL::Util::GetClientDescription("foo::service", "foo::method", req_desc, resp_desc);
    EXPECT_EQ(req_desc,  "foo::req_desc2");
    EXPECT_EQ(resp_desc, "foo::resp_desc2");

    // check size it's client 2 only
    EXPECT_EQ(client_info_map.size(), 1);
  }

  // let's unregister
  eCAL::Process::SleepMS(2 * CMN_REGISTRATION_REFRESH_MS);

  // get all clients again, all clients
  // should be removed from the map
  eCAL::Util::GetClients(client_info_map);

  // check size
  EXPECT_EQ(client_info_map.size(), 0);

  // finalize eCAL API
  eCAL::Finalize();
}
