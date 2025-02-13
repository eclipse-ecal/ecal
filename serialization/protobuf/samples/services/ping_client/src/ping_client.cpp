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

#include <iostream>
#include <chrono>
#include <thread>

#include "ping.pb.h"

// main entry
int main()
{
  // initialize eCAL API
  eCAL::Initialize("ping client");

  // create ping service client
  const eCAL::protobuf::CServiceClientTyped<PingService> ping_client("ping service");

  // waiting for service
  while (eCAL::Ok() && !ping_client.IsConnected())
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::cout << "Waiting for the service .." << std::endl;
  }

  while(eCAL::Ok())
  {
    if (ping_client.IsConnected())
    {
      //////////////////////////////////////
      // Ping service (blocking call)
      //////////////////////////////////////
      PingRequest               ping_request;
      ping_request.set_message("PING");
      auto ping_response = ping_client.CallWithResponse<PingRequest, PingResponse>("Ping", ping_request);
      if (ping_response.first)
      {
        std::cout << std::endl << "PingService::Ping method called with message : " << ping_request.message() << std::endl;

        for (const auto& service_response : ping_response.second)
        {
          switch (service_response.call_state)
          {
            // service successful executed
          case eCAL::eCallState::executed:
          {
            std::cout << "Received response PingService / Ping : " << service_response.response.answer() << " from host " << service_response.server_id.service_id.host_name << std::endl;
          }
          break;
          // service execution failed
          case eCAL::eCallState::failed:
            std::cout << "Received error PingService / Ping : " << service_response.error_msg << " from host " << service_response.server_id.service_id.host_name << std::endl;
            break;
          default:
            break;
          }
        }
      }
      else
      {
        std::cout << "PingService::Ping method call failed .." << std::endl << std::endl;
      }
    }

    // sleep a second
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }

  // finalize eCAL API
  eCAL::Finalize();

  return(0);
}
