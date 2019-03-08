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

#include <iostream>
#include <chrono>
#include <thread>

#include "ping.pb.h"

// main entry
int main(int argc, char **argv)
{
  // initialize eCAL API
  eCAL::Initialize(argc, argv, "ping client");

  // create ping service client
  eCAL::protobuf::CServiceClient<PingService> ping_service;

  while(eCAL::Ok())
  {
    //////////////////////////////////////
    // Ping service (blocking call)
    //////////////////////////////////////
    eCAL::SServiceInfo service_info;
    PingRequest        ping_request;
    PingResponse       ping_response;
    ping_request.set_message("PING");
    if (ping_service.Call("", "Ping", ping_request, service_info, ping_response))
    {
      std::cout << std::endl << "PingService::Ping method called with message : " << ping_request.message() << std::endl;
      switch (service_info.call_state)
      {
      // service successful executed
      case call_state_executed:
      {
        std::cout << "Received response PingService / Ping : " << ping_response.answer() << " from host " << service_info.host_name << std::endl;
      }
      break;
      // service execution failed
      case call_state_failed:
        std::cout << "Received error PingService / Ping : " << service_info.error_msg << " from host " << service_info.host_name << std::endl;
        break;
      default:
        break;
      }
    }
    else
    {
      std::cout << "PingService::Ping method call failed .." << std::endl << std::endl;
    }

    // sleep a second
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }

  // finalize eCAL API
  eCAL::Finalize();

  return(0);
}
