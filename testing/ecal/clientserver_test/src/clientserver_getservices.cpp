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

#include <gtest/gtest.h>

#define CMN_MONITORING_TIMEOUT 5000

TEST(IO, GetServices)
{
  // initialize eCAL API
  eCAL::Initialize(0, nullptr, "clientserver_getservices");

  std::map<std::tuple<std::string, std::string>, eCAL::Util::SServiceMethodInfo> service_info_map;
  
  // add and expire simple service
  {
    // create server
    eCAL::CServiceServer server("foo::service");

    // add a description only
    server.AddDescription("foo::method1", "foo::req_type1", "foo::req_desc1", "foo::resp_type1", "foo::resp_desc1");

    // get all service
    eCAL::Util::GetServices(service_info_map);

    // check size
    EXPECT_EQ(service_info_map.size(), 1);

    // let's wait a monitoring timeout long
    eCAL::Process::SleepMS(CMN_MONITORING_TIMEOUT);

    // get all services again, service should not be expired
    eCAL::Util::GetServices(service_info_map);

    // check size
    EXPECT_EQ(service_info_map.size(), 1);
  }
  // let's unregister them
  eCAL::Process::SleepMS(CMN_MONITORING_TIMEOUT + 1000);

  // get all services again, now all services 
  // should be removed from the map
  eCAL::Util::GetServices(service_info_map);

  // check size
  EXPECT_EQ(service_info_map.size(), 0);

  // add a service with method callback
  {
    // create server
    eCAL::CServiceServer server("foo::service");

    auto method_callback = [&](const std::string& /*method_*/, const std::string& /*req_type_*/, const std::string& /*resp_type_*/, const std::string& /*request_*/, std::string& /*response_*/) -> int
    {
      return 42;
    };

    // add method callback
    server.AddMethodCallback("foo::method1", "foo::req_type1", "foo::resp_type1", method_callback);

    // get all service
    eCAL::Util::GetServices(service_info_map);

    // check size
    EXPECT_EQ(service_info_map.size(), 1);

    // check attributes
    std::string req_type, resp_type;
    std::string req_desc, resp_desc;

    eCAL::Util::GetServiceTypeNames("foo::service", "foo::method1", req_type, resp_type);
    EXPECT_EQ(req_type,  "foo::req_type1");
    EXPECT_EQ(resp_type, "foo::resp_type1");
    eCAL::Util::GetServiceDescription("foo::service", "foo::method1", req_desc, resp_desc);
    EXPECT_EQ(req_desc, "");
    EXPECT_EQ(resp_desc, "");

    // change attributes
    bool ret1 = server.AddDescription("foo::method1", "foo::req_type1-1", "foo::req_desc1-1", "foo::resp_type1-1", "foo::resp_desc1-1");
    EXPECT_EQ(ret1, true);

    // check attributes
    eCAL::Util::GetServiceTypeNames("foo::service", "foo::method1", req_type, resp_type);
    EXPECT_EQ(req_type, "foo::req_type1-1");
    EXPECT_EQ(resp_type, "foo::resp_type1-1");
    eCAL::Util::GetServiceDescription("foo::service", "foo::method1", req_desc, resp_desc);
    EXPECT_EQ(req_desc, "foo::req_desc1-1");
    EXPECT_EQ(resp_desc, "foo::resp_desc1-1");

    // change attributes again (this will not overwrite the attributes anymore)
    bool ret2 = server.AddDescription("foo::method1", "foo::req_type1-2", "foo::req_desc1-2", "foo::resp_type1-2", "foo::resp_desc1-2");
    EXPECT_EQ(ret2, false);

    // check attributes
    eCAL::Util::GetServiceTypeNames("foo::service", "foo::method1", req_type, resp_type);
    EXPECT_EQ(req_type, "foo::req_type1-1");
    EXPECT_EQ(resp_type, "foo::resp_type1-1");
    eCAL::Util::GetServiceDescription("foo::service", "foo::method1", req_desc, resp_desc);
    EXPECT_EQ(req_desc, "foo::req_desc1-1");
    EXPECT_EQ(resp_desc, "foo::resp_desc1-1");

    // let's wait a monitoring timeout long
    eCAL::Process::SleepMS(CMN_MONITORING_TIMEOUT);

    // get all services again, service should not be expired
    eCAL::Util::GetServices(service_info_map);

    // check size
    EXPECT_EQ(service_info_map.size(), 1);
  }
  // let's unregister them
  eCAL::Process::SleepMS(CMN_MONITORING_TIMEOUT + 1000);

  // get all services again, now all services 
  // should be removed from the map
  eCAL::Util::GetServices(service_info_map);

  // check size
  EXPECT_EQ(service_info_map.size(), 0);

  // finalize eCAL API
  eCAL::Finalize();
}
