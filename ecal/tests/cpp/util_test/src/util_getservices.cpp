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

TEST(core_cpp_util, ServiceExpiration)
{
  // initialize eCAL API
  eCAL::Initialize(0, nullptr, "core_cpp_util");

  // enable loop back communication in the same process
  eCAL::Util::EnableLoopback(true);

  std::map<eCAL::Util::SServiceMethod, eCAL::SServiceMethodInformation> service_info_map;

  // create simple service and let it expire
  {
    // create service
    eCAL::CServiceServer service("foo::service");
    service.AddDescription("foo::method", "foo::req_type", "foo::req_desc", "foo::resp_type", "foo::resp_desc");

    // let's register
    eCAL::Process::SleepMS(2 * CMN_REGISTRATION_REFRESH_MS);

    // get all services
    eCAL::Util::GetServices(service_info_map);

    // check size
    EXPECT_EQ(service_info_map.size(), 1);

    // check service/method names
    std::set<eCAL::Util::SServiceMethod> service_method_names;
    eCAL::Util::GetServiceMethodNames(service_method_names);
    EXPECT_EQ(service_method_names.size(), 1);
    for (const auto& name : service_method_names)
    {
      EXPECT_EQ(name.service_name, "foo::service");
      EXPECT_EQ(name.method_name,  "foo::method");
    }

    // let's wait a monitoring timeout long
    eCAL::Process::SleepMS(CMN_MONITORING_TIMEOUT_MS);

    // get all services again, service should not be expired
    eCAL::Util::GetServices(service_info_map);

    // check size
    EXPECT_EQ(service_info_map.size(), 1);
  }

  // let's unregister
  eCAL::Process::SleepMS(2 * CMN_REGISTRATION_REFRESH_MS);

  // get all services again, all services
  // should be removed from the map
  eCAL::Util::GetServices(service_info_map);

  // check size
  EXPECT_EQ(service_info_map.size(), 0);

  // finalize eCAL API
  eCAL::Finalize();
}

TEST(core_cpp_util, ServiceEqualQualities)
{
  // initialize eCAL API
  eCAL::Initialize(0, nullptr, "core_cpp_util");

  // enable loop back communication in the same process
  eCAL::Util::EnableLoopback(true);

  std::map<eCAL::Util::SServiceMethod, eCAL::SServiceMethodInformation> service_info_map;

  // create 2 services with the same quality of data type information
  {
    // create service 1
    eCAL::CServiceServer service1("foo::service");
    service1.AddDescription("foo::method", "foo::req_type1", "foo::req_desc1", "foo::resp_type1", "foo::resp_desc1");

    // let's register
    eCAL::Process::SleepMS(2 * CMN_REGISTRATION_REFRESH_MS);

    // get all services
    eCAL::Util::GetServices(service_info_map);

    // check size
    EXPECT_EQ(service_info_map.size(), 1);

    // check attributes
    std::string req_type, resp_type;
    std::string req_desc, resp_desc;

    eCAL::Util::GetServiceTypeNames("foo::service", "foo::method", req_type, resp_type);
    EXPECT_EQ(req_type,  "foo::req_type1");
    EXPECT_EQ(resp_type, "foo::resp_type1");
    eCAL::Util::GetServiceDescription("foo::service", "foo::method", req_desc, resp_desc);
    EXPECT_EQ(req_desc,  "foo::req_desc1");
    EXPECT_EQ(resp_desc, "foo::resp_desc1");

    // create service 2
    // this will not overwrite the attributes from service 1, because the quality is not higher
    eCAL::CServiceServer service2("foo::service");
    service2.AddDescription("foo::method", "foo::req_type2", "foo::req_desc2", "foo::resp_type2", "foo::resp_desc2");

    // check attributes
    eCAL::Util::GetServiceTypeNames("foo::service", "foo::method", req_type, resp_type);
    EXPECT_EQ(req_type,  "foo::req_type1");
    EXPECT_EQ(resp_type, "foo::resp_type1");
    eCAL::Util::GetServiceDescription("foo::service", "foo::method", req_desc, resp_desc);
    EXPECT_EQ(req_desc,  "foo::req_desc1");
    EXPECT_EQ(resp_desc, "foo::resp_desc1");

    // check size it's service 1 only
    EXPECT_EQ(service_info_map.size(), 1);

    // let's wait a monitoring timeout long
    eCAL::Process::SleepMS(CMN_MONITORING_TIMEOUT_MS);

    // get all services again, services should not be expired
    eCAL::Util::GetServices(service_info_map);

    // check size
    EXPECT_EQ(service_info_map.size(), 1);

    // destroy service 1
    service1.Destroy();

    // let's register
    eCAL::Process::SleepMS(2 * CMN_REGISTRATION_REFRESH_MS);

    // check attributes, service 1 attributes should be replaced by service 2 attributes now
    eCAL::Util::GetServiceTypeNames("foo::service", "foo::method", req_type, resp_type);
    EXPECT_EQ(req_type,  "foo::req_type2");
    EXPECT_EQ(resp_type, "foo::resp_type2");
    eCAL::Util::GetServiceDescription("foo::service", "foo::method", req_desc, resp_desc);
    EXPECT_EQ(req_desc,  "foo::req_desc2");
    EXPECT_EQ(resp_desc, "foo::resp_desc2");
  }

  // let's unregister
  eCAL::Process::SleepMS(2 * CMN_REGISTRATION_REFRESH_MS);

  // get all services again, all services 
  // should be removed from the map
  eCAL::Util::GetServices(service_info_map);

  // check size
  EXPECT_EQ(service_info_map.size(), 0);

  // finalize eCAL API
  eCAL::Finalize();
}

TEST(core_cpp_util, ServiceDifferentQualities)
{
  // initialize eCAL API
  eCAL::Initialize(0, nullptr, "core_cpp_util");

  // enable loop back communication in the same process
  eCAL::Util::EnableLoopback(true);

  std::map<eCAL::Util::SServiceMethod, eCAL::SServiceMethodInformation> service_info_map;

  // create 2 services with different qualities of data type information
  {
    // create service 1, response type name and response description are missing
    eCAL::CServiceServer service1("foo::service");
    service1.AddDescription("foo::method", "foo::req_type1", "foo::req_desc1", "", "");

    // let's register
    eCAL::Process::SleepMS(2 * CMN_REGISTRATION_REFRESH_MS);

    // get all services
    eCAL::Util::GetServices(service_info_map);

    // check size
    EXPECT_EQ(service_info_map.size(), 1);

    // check attributes
    std::string req_type, resp_type;
    std::string req_desc, resp_desc;

    eCAL::Util::GetServiceTypeNames("foo::service", "foo::method", req_type, resp_type);
    EXPECT_EQ(req_type,  "foo::req_type1");
    EXPECT_EQ(resp_type, "");
    eCAL::Util::GetServiceDescription("foo::service", "foo::method", req_desc, resp_desc);
    EXPECT_EQ(req_desc,  "foo::req_desc1");
    EXPECT_EQ(resp_desc, "");

    // create service 2, with higher quality than service 1
    eCAL::CServiceServer service2("foo::service");
    service2.AddDescription("foo::method", "foo::req_type2", "foo::req_desc2", "foo::resp_type2", "foo::resp_desc2");

    // let's register
    eCAL::Process::SleepMS(2 * CMN_REGISTRATION_REFRESH_MS);

    // check attributes, we expect attributes from service 2 here
    eCAL::Util::GetServiceTypeNames("foo::service", "foo::method", req_type, resp_type);
    EXPECT_EQ(req_type,  "foo::req_type2");
    EXPECT_EQ(resp_type, "foo::resp_type2");
    eCAL::Util::GetServiceDescription("foo::service", "foo::method", req_desc, resp_desc);
    EXPECT_EQ(req_desc,  "foo::req_desc2");
    EXPECT_EQ(resp_desc, "foo::resp_desc2");

    // check size it's service 2 only
    EXPECT_EQ(service_info_map.size(), 1);
  }

  // let's unregister
  eCAL::Process::SleepMS(2 * CMN_REGISTRATION_REFRESH_MS);

  // get all services again, all services
  // should be removed from the map
  eCAL::Util::GetServices(service_info_map);

  // check size
  EXPECT_EQ(service_info_map.size(), 0);

  // finalize eCAL API
  eCAL::Finalize();
}
