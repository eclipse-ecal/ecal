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
#include <ecal/ecal_types.h>

#include <gtest/gtest.h>

#define CMN_REGISTRATION_REFRESH 1000
#define CMN_MONITORING_TIMEOUT   5000

TEST(core_cpp_util, GetClients)
{
  // initialize eCAL API
  eCAL::Initialize(0, nullptr, "util_getclients");

  std::map<std::tuple<std::string, std::string>, eCAL::SServiceMethodInformation> client_info_map;
  
  // add and expire simple client
  {
    // create client
    eCAL::SServiceMethodInformation service_method_info;
    service_method_info.request_type.name        = "foo::req_type1";
    service_method_info.request_type.descriptor  = "foo::req_desc1";
    service_method_info.response_type.name       = "foo::resp_type1";
    service_method_info.response_type.descriptor = "foo::resp_desc1";
    eCAL::CServiceClient client("foo::service", { {"foo::method", service_method_info} });

    // get all clients
    eCAL::Util::GetClients(client_info_map);

    // check size
    EXPECT_EQ(client_info_map.size(), 1);

    // let's wait a monitoring timeout long
    eCAL::Process::SleepMS(CMN_MONITORING_TIMEOUT);

    // get all clients again, client should not be expired
    eCAL::Util::GetClients(client_info_map);

    // check size
    EXPECT_EQ(client_info_map.size(), 1);
  }

  // get all clients again, all clients 
  // should be removed from the map
  eCAL::Util::GetClients(client_info_map);

  // check size
  EXPECT_EQ(client_info_map.size(), 0);

  // create 2 clients
  {
    // create client 1
    eCAL::SServiceMethodInformation service_method_info1;
    service_method_info1.request_type.name        = "foo::req_type1";
    service_method_info1.request_type.descriptor  = "foo::req_desc1";
    service_method_info1.response_type.name       = "foo::resp_type1";
    service_method_info1.response_type.descriptor = "foo::resp_desc1";
    eCAL::CServiceClient client1("foo::service", { {"foo::method", service_method_info1} });

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

    // check client/method names
    std::vector<std::tuple<std::string, std::string>> client_method_names;
    eCAL::Util::GetClientMethodNames(client_method_names);
    EXPECT_EQ(client_method_names.size(), 1);
    for (const auto& method_name : client_method_names)
    {
      EXPECT_EQ(std::get<0>(method_name), "foo::service");
      EXPECT_EQ(std::get<1>(method_name), "foo::method");
    }

    // create client 2
    // this will not overwrite the attributes from client1
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

    // check size (client1 replaced by client2)
    EXPECT_EQ(client_info_map.size(), 1);

    // let's wait a monitoring timeout long
    eCAL::Process::SleepMS(CMN_MONITORING_TIMEOUT);

    // get all clients again, client should not be expired
    eCAL::Util::GetClients(client_info_map);

    // check size
    EXPECT_EQ(client_info_map.size(), 1);

    // destroy client 1
    client1.Destroy();

    // check attributes (client 1 should be replaced by client 2 immediately)
    eCAL::Util::GetClientTypeNames("foo::service", "foo::method", req_type, resp_type);
    EXPECT_EQ(req_type, "foo::req_type2");
    EXPECT_EQ(resp_type, "foo::resp_type2");
  }

  // get all clients again, all clients
  // should be removed from the map
  eCAL::Util::GetClients(client_info_map);

  // check size
  EXPECT_EQ(client_info_map.size(), 0);

  // finalize eCAL API
  eCAL::Finalize();
}
