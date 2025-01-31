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

#include <iostream>
#include <chrono>
#include <thread>

// main entry
int main()
{
  // initialize eCAL API
  eCAL::Initialize("minimal client");

  // create minimal service client
  const eCAL::CServiceClient minimal_client("service1", { {"echo", {}, {} } });

  // callback for service response
  auto service_response_callback = [](const eCAL::SServiceResponse& service_response_) {
    switch (service_response_.call_state)
    {
      // service successful executed
    case eCAL::eCallState::executed:
    {
      std::cout << "Received response for method " << service_response_.service_method_information.method_name << " : " << service_response_.response << " from service id " << service_response_.server_id.service_id.entity_id << " from host " << service_response_.server_id.service_id.host_name << std::endl;
    }
    break;
    // service execution failed
    case eCAL::eCallState::failed:
      std::cout << "Received error for method " << service_response_.service_method_information.method_name << " : " << service_response_.error_msg << " from service id " << service_response_.server_id.service_id.entity_id << " from host " << service_response_.server_id.service_id.host_name << std::endl;
      break;
    default:
      break;
    }
    };

  // are we connected to at least one service?
  while (!minimal_client.IsConnected())
  {
    std::cout << "Waiting for a service .." << std::endl;

    // sleep a second
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }

  while(eCAL::Ok())
  {
    std::string method_name("echo");
    std::string request("Hello");

    // call all existing services
    for (auto& client_instance : minimal_client.GetClientInstances())
    {
      //////////////////////////////////////
      // Service call (blocking)
      //////////////////////////////////////
      const auto service_response = client_instance.CallWithResponse(method_name, request, -1);
      if (std::get<0>(service_response))
      {
        auto& response_content = std::get<1>(service_response);
        std::cout << std::endl << "Method 'echo' called with message : " << request << std::endl;
        auto& server_host_name = response_content.server_id.service_id.host_name;
        switch (response_content.call_state)
        {
          // service successful executed
        case eCAL::eCallState::executed:
        {
          std::cout << "Received response : " << response_content.response << " from service id " << client_instance.GetClientID() << " from host " << server_host_name << std::endl;
        }
        break;
        // service execution failed
        case eCAL::eCallState::failed:
          std::cout << "Received error : " << response_content.error_msg << " from service id " << client_instance.GetClientID() << " from host " << server_host_name << std::endl;
          break;
        default:
          break;
        }
      }
      else
      {
        std::cout << "Method blocking call failed .." << std::endl << std::endl;
      }

      //////////////////////////////////////
      // Service call (with callback)
      //////////////////////////////////////
      if (client_instance.CallWithCallback(method_name, request, service_response_callback))
      {
        std::cout << std::endl << "Method 'echo' called with message : " << request << std::endl;
      }
      else
      {
        std::cout << "Method callback call failed .." << std::endl << std::endl;
      }
    }

    // sleep a second
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }

  // finalize eCAL API
  eCAL::Finalize();

  return(0);
}
