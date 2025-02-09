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
#include <ecal/msg/protobuf/client.h>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4100 4505 4800)
#endif
#include <ecal/app/pb/sys/service.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <chrono>
#include <thread>
#include <iostream>

// callback for sys service response
void OnSysResponse(const struct eCAL::SServiceResponse& service_response_)
{
  const std::string& method_name = service_response_.service_method_information.method_name;
  const std::string& host_name   = service_response_.server_id.service_id.host_name;

  switch (service_response_.call_state)
  {
  // service successful executed
  case eCAL::eCallState::executed:
    {
      eCAL::pb::sys::Response response;
      response.ParseFromString(service_response_.response);
      std::cout << "SysService " << method_name << " called successfully on host " << host_name << std::endl;
    }
    break;
  // service execution failed
  case eCAL::eCallState::failed:
    {
      eCAL::pb::sys::Response response;
      response.ParseFromString(service_response_.response);
      std::cout << "SysService " << method_name << " failed with \"" << response.error() << "\" on host " << host_name << std::endl;
    }
    break;
  default:
    break;
  }
}

// main entry
int main()
{
  // initialize eCAL API
  eCAL::Initialize("ecalsys client");

  // create player service client
  eCAL::protobuf::CServiceClient<eCAL::pb::sys::Service> sys_service;

  // sleep for service matching
  std::this_thread::sleep_for(std::chrono::milliseconds(2000));

  // requests
  eCAL::pb::sys::TaskRequest trequest;

  // "StartTasks - All"
  std::cout << "eCALPB.Sys.Service:StartTasks()" << std::endl;
  trequest.set_all(true);
  sys_service.CallWithCallback("StartTasks", trequest, OnSysResponse);
  std::cout << trequest.DebugString() << std::endl; std::this_thread::sleep_for(std::chrono::milliseconds(5000));

  // "RestartTasks - All"
  std::cout << "eCALPB.Sys.Service:RestartTasks()" << std::endl;
  trequest.set_all(true);
  sys_service.CallWithCallback("RestartTasks", trequest, OnSysResponse);
  std::cout << trequest.DebugString() << std::endl; std::this_thread::sleep_for(std::chrono::milliseconds(5000));

  // "StopTasks - All"
  std::cout << "eCALPB.Sys.Service:StopTasks()" << std::endl;
  trequest.set_all(true);
  sys_service.CallWithCallback("StopTasks", trequest, OnSysResponse);
  std::cout << trequest.DebugString() << std::endl; std::this_thread::sleep_for(std::chrono::milliseconds(5000));

  // finalize eCAL API
  eCAL::Finalize();

  return(0);
}
