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

#include <iostream>
#include <chrono>
#include <thread>

// callback for service response
void OnServiceResponse(const struct eCAL::SServiceResponse& service_response_)
{
  switch (service_response_.call_state)
  {
    // service successful executed
  case call_state_executed:
  {
    std::cout << "Received response for method " << service_response_.method_name << " : " << service_response_.response << " from host " << service_response_.host_name << std::endl;
  }
  break;
  // service execution failed
  case call_state_failed:
    std::cout << "Received error for method " << service_response_.method_name << " : " << service_response_.error_msg << " from host " << service_response_.host_name << std::endl;
    break;
  default:
    break;
  }
}

// main entry
int main(int argc, char **argv)
{
  // initialize eCAL API
  eCAL::Initialize(argc, argv, "minimal client");

  // create minimal service client
  eCAL::CServiceClient minimal_client("service1");
  minimal_client.AddResponseCallback(OnServiceResponse);

  while(eCAL::Ok())
  {
    eCAL::SServiceResponse service_info;
    std::string method_name("echo");
    std::string request("Hello");

    //////////////////////////////////////
    // Service call (blocking)
    //////////////////////////////////////
    eCAL::ServiceResponseVecT service_response_vec;
    if (minimal_client.Call(method_name, request, -1, &service_response_vec))
    {
      for (auto service_response : service_response_vec)
      {

        std::cout << std::endl << "Method 'echo' called with message : " << request << std::endl;
        switch (service_info.call_state)
        {
          // service successful executed
        case call_state_executed:
        {
          std::cout << "Received response : " << service_response.response << " from host " << service_info.host_name << std::endl;
        }
        break;
        // service execution failed
        case call_state_failed:
          std::cout << "Received error : " << service_info.error_msg << " from host " << service_info.host_name << std::endl;
          break;
        default:
          break;
        }
      }
    }
    else
    {
      std::cout << "Method blocking call failed .." << std::endl << std::endl;
    }

    //////////////////////////////////////
    // Service call (with callback)
    //////////////////////////////////////
    if(minimal_client.Call(method_name, request))
    {
      std::cout << std::endl << "Method 'echo' called with message : " << request << std::endl;
    }
    else
    {
      std::cout << "Method callback call failed .." << std::endl << std::endl;
    }

    // sleep a second
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }

  // finalize eCAL API
  eCAL::Finalize();

  return(0);
}
