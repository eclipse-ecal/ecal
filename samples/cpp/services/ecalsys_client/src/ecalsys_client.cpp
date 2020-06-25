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
#include <ecal/msg/protobuf/client.h>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4100 4505 4800)
#endif
#include <ecal/pb/sys/service.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <chrono>
#include <thread>
#include <iostream>

// callback for sys service response
void OnSysResponse(const struct eCAL::SServiceInfo& service_info_, const std::string& response_)
{
  switch (service_info_.call_state)
  {
  // service successful executed
  case call_state_executed:
    {
      eCAL::pb::sys::Response response;
      response.ParseFromString(response_);
      std::cout << "SysService " << service_info_.method_name << " called successfully on host " << service_info_.host_name << std::endl;
    }
    break;
  // service execution failed
  case call_state_failed:
    {
      eCAL::pb::sys::Response response;
      response.ParseFromString(response_);
      std::cout << "SysService " << service_info_.method_name << " failed with \"" << response.error() << "\" on host " << service_info_.host_name << std::endl;
    }
    break;
  default:
    break;
  }
}

// main entry
int main(int argc, char **argv)
{
  // initialize eCAL API
  eCAL::Initialize(argc, argv, "ecalsys client");

  // create player service client
  eCAL::protobuf::CServiceClient<eCAL::pb::sys::Service> sys_service;
  sys_service.AddResponseCallback(OnSysResponse);

  // sleep for service matching
  std::this_thread::sleep_for(std::chrono::milliseconds(2000));

  // requests
  eCAL::pb::sys::TaskRequest  trequest;
  eCAL::pb::sys::CloseRequest crequest;

  // "StartTasks - All"
  std::cout << "eCALPB.Sys.Service:StartTasks()" << std::endl;
  trequest.set_all(true);
  sys_service.Call("StartTasks", trequest);
  std::cout << trequest.DebugString() << std::endl; std::this_thread::sleep_for(std::chrono::milliseconds(5000));

  // "RestartTasks - All"
  std::cout << "eCALPB.Sys.Service:RestartTasks()" << std::endl;
  trequest.set_all(true);
  sys_service.Call("RestartTasks", trequest);
  std::cout << trequest.DebugString() << std::endl; std::this_thread::sleep_for(std::chrono::milliseconds(5000));

  // "StopTasks - All"
  std::cout << "eCALPB.Sys.Service:StopTasks()" << std::endl;
  trequest.set_all(true);
  sys_service.Call("StopTasks", trequest);
  std::cout << trequest.DebugString() << std::endl; std::this_thread::sleep_for(std::chrono::milliseconds(5000));

  // "Close"
  std::cout << "eCALPB.Sys.Service:Close()" << std::endl;
  sys_service.Call("Close", crequest);
  std::cout << crequest.DebugString() << std::endl; std::this_thread::sleep_for(std::chrono::milliseconds(2000));

  // finalize eCAL API
  eCAL::Finalize();

  return(0);
}
