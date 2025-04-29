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

/*
  We define a callback function that will be called when the an event happens on the client, like connected or disconnected.
*/
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

void printResponseInformation(const eCAL::protobuf::SMsgServiceResponse<SFloat>& service_response_)
{
  const std::string& method_name = service_response_.service_method_information.method_name;
  const std::string& host_name   = service_response_.server_id.service_id.host_name;
  const int32_t&     process_id  = service_response_.server_id.service_id.process_id;
  std::string        state;

  switch(service_response_.call_state)
  {
    case eCAL::eCallState::executed:
      state = "EXECUTED";
      break;
    case eCAL::eCallState::timeouted:
      state = "TIMEOUTED";
      break;
    case eCAL::eCallState::failed:
      state = "FAILED";
      break;
    default:
      state = "UNKNOWN";
      break;
  }
  std::cout << "State    :" << state << "\n";
  std::cout << "Method   :" << method_name << "\n";
  std::cout << "Response :" << service_response_.response.out() << "\n";
  std::cout << "Host     :" << host_name << "\n";
  std::cout << "PID      :" << process_id << "\n";
  std::cout << "\n";
}

/*
  Callback for math service responses (for the callback variants).
*/
void OnMathCallbackResponse(const eCAL::protobuf::SMsgServiceResponse<SFloat>& service_response_)
{
  std::cout << "Callback: Received response MathService: " << "\n";
  printResponseInformation(service_response_);
}

/*
  Function for math service responses (for the blocking variants).
*/
void onMathBlockingResponse(const eCAL::protobuf::SMsgServiceResponse<SFloat>& service_response_)
{
  std::cout << "Blocking: Received response MathService: " << "\n";
  printResponseInformation(service_response_);
}

int main()
{
  /*
    As always: initialize the eCAL API and give your process a name.
  */
  eCAL::Initialize("math client");

  /*
    Create a math service client using the protobuf service type MathService.
  */
  const eCAL::protobuf::CServiceClient<MathService> math_client(OnClientState);

  /*
    Wait until the client is connected to at least one service.
  */
  while (eCAL::Ok() && !math_client.IsConnected())
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::cout << "Waiting for the service .." << std::endl;
  }

  /*
    Now we prepare the request message based on protobuf defined message SFloatTuple.
  */
  SFloatTuple math_request;
  int counter = 0;

  /*
    Create a vector of method names that we want to call later alternatively.
  */
  std::vector<std::string> method_names = { "Add", "Multiply", "Divide" };

  while (eCAL::Ok())
  {
    if (math_client.IsConnected())
    {
      math_request.set_inp1(counter);
      math_request.set_inp2(counter + 1);

      /*
        As in the binary examples, we now iterate over the client instances and call the service methods.
        If you either way want to call all instances, you can use the following high level methods:

        math_client.CallWithResponse<SFloatTuple, SFloat>("Divide", math_request, 1000);
        math_client.CallWithCallback<SFloatTuple, SFloat>("Divide", math_request, OnMathCallbackResponse, 1000);

        But using the iterative approach you have more control over the instances you want to call.
      */
      auto instances = math_client.GetClientInstances();
      for (auto& instance : instances)
      {
        /*
          Service call: Callback
        */
        if (!instance.CallWithCallback<SFloatTuple, SFloat>(method_names[counter % 3], math_request, OnMathCallbackResponse))
        {
          std::cout << "MathService::Divide call on an instance failed." << std::endl;
        }

        /*
          Service call: Blocking
        */
        auto divide_response = instance.CallWithResponse<SFloatTuple, SFloat>(method_names[counter % 3], math_request);
        if (divide_response.first)
        {
          onMathBlockingResponse(divide_response.second);
        }
        else
        {
          std::cout << "Blocking: MathService::Divide call failed: " << divide_response.second.error_msg << std::endl;
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

  /*
    Cleanup: finalize the eCAL API.
  */
  eCAL::Finalize();

  return 0;
}
