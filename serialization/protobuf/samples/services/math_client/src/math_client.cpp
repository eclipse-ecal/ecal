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

#include "math.pb.h"

// Client state callback: prints connection, disconnection, or timeout information.
void OnClientState(const eCAL::SServiceId& service_id_, const eCAL::SClientEventCallbackData& data_)
{
  switch (data_.type)
  {
  case eCAL::eClientEvent::connected:
    std::cout << "---------------------------------" << std::endl;
    std::cout << "Client connected to service      " << std::endl;
    std::cout << "---------------------------------" << std::endl;
    break;
  case eCAL::eClientEvent::disconnected:
    std::cout << "---------------------------------" << std::endl;
    std::cout << "Client disconnected from service " << std::endl;
    std::cout << "---------------------------------" << std::endl;
    break;
  case eCAL::eClientEvent::timeout:
    std::cout << "---------------------------------" << std::endl;
    std::cout << "Client request timeouted         " << std::endl;
    std::cout << "---------------------------------" << std::endl;
    break;
  default:
    std::cout << "Unknown client event." << std::endl;
    break;
  }

  std::cout << "Server Hostname      : " << service_id_.service_id.host_name << std::endl;
  std::cout << "Server Name          : " << service_id_.service_name << std::endl;
  std::cout << "Server PID           : " << service_id_.service_id.process_id << std::endl;
  std::cout << "---------------------------------" << std::endl << std::endl;
}

// Callback for math service responses (for the callback variants).
void OnMathResponse(const eCAL::protobuf::SMsgServiceResponse<SFloat>& service_response_)
{
  const std::string& method_name = service_response_.service_method_information.method_name;
  const std::string& host_name   = service_response_.server_id.service_id.host_name;
  const int32_t&     process_id  = service_response_.server_id.service_id.process_id;

  if (service_response_.call_state == eCAL::eCallState::executed)
  {
    std::cout << "Callback: Received response MathService / " << method_name
      << " : " << service_response_.response.out()
      << " from host " << host_name << " with pid " << process_id << std::endl;
  }
  else
  {
    std::cout << "Callback: Received error MathService / " << method_name
      << " : " << service_response_.error_msg
      << " from host " << host_name << " with pid " << process_id << std::endl;
  }
}

int main()
{
  // Initialize the eCAL API.
  eCAL::Initialize("math client");

  // Create 2 math services client using the protobuf service type MathService.
  const eCAL::protobuf::CServiceClientTyped<MathService> math_client(OnClientState);

  // Wait until the client is connected to at least one service.
  while (eCAL::Ok() && !math_client.IsConnected())
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::cout << "Waiting for the service .." << std::endl;
  }

  // Prepare a request message.
  SFloatTuple math_request;
  int counter = 0;

  while (eCAL::Ok())
  {
    if (math_client.IsConnected())
    {
      math_request.set_inp1(counter);
      math_request.set_inp2(counter + 1);

      // --- CallWithCallback variant using the top-level service client ---
      {
        std::cout << std::endl << "Calling MathService::Add (callback) with      : " << math_request.inp1() << " and " << math_request.inp2() << std::endl;
        if (!math_client.CallWithCallback<SFloatTuple, SFloat>("Add", math_request, OnMathResponse))
        {
          std::cout << "MathService::Add method call (callback) failed .." << std::endl;
        }
      }

      // --- CallWithResponse variant using the top-level service client ---
      {
        std::cout << std::endl << "Calling MathService::Multiply (blocking) with : " << math_request.inp1() << " and " << math_request.inp2() << std::endl;
        auto multiply_response = math_client.CallWithResponse<SFloatTuple, SFloat>("Multiply", math_request);
        if (multiply_response.first)
        {
          // Iterate over all responses
          for (const auto& resp : multiply_response.second)
          {
            std::cout << "Blocking: Received response MathService / Multiply : " << resp.response.out() << std::endl;
          }
        }
        else
        {
          std::cout << "Blocking: MathService::Multiply call failed!" << std::endl;
        }
      }

      // --- Iterative CallWithCallback variant calling each client instance individually ---
      {
        std::cout << std::endl << "Iterating over client instances to call MathService::Divide" << std::endl;
        auto instances = math_client.GetClientInstances();
        for (auto& instance : instances)
        {
          std::cout << std::endl << "Calling MathService::Divide (callback inst) with : " << math_request.inp1() << " and " << math_request.inp2() << std::endl;
          if (!instance.CallWithCallback<SFloatTuple, SFloat>("Divide", math_request, OnMathResponse))
          {
            std::cout << "MathService::Divide call on an instance failed." << std::endl;
          }
        }
      }

      // --- Iterative CallWithResponse variant calling each client instance individually ---
      {
        std::cout << std::endl << "Iterating over client instances to call MathService::Divide" << std::endl;
        auto instances = math_client.GetClientInstances();
        for (auto& instance : instances)
        {
          std::cout << std::endl << "Calling MathService::Divide (blocking inst) with : " << math_request.inp1() << " and " << math_request.inp2() << std::endl;
          auto divide_response = instance.CallWithResponse<SFloatTuple, SFloat>("Divide", math_request);
          if (divide_response.first)
          {
            std::cout << "Blocking: Received response MathService / Divide : " << divide_response.second.response.out() << std::endl;
          }
          else
          {
            std::cout << "Blocking: MathService::Divide call failed: " << divide_response.second.error_msg << std::endl;
          }
        }
      }
    }
    else
    {
      std::cout << "Waiting for the service .." << std::endl;
    }

    counter++;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }

  // Finalize the eCAL API.
  eCAL::Finalize();
  return 0;
}
