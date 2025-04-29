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

/*
  Helper function to print the service response.
*/
void printServiceResponse(const eCAL::SServiceResponse& service_response_)
{
  std::string call_state;
  switch (service_response_.call_state)
  {
    case eCAL::eCallState::executed:
      call_state = "EXECUTED";
      break;
    case eCAL::eCallState::failed:
      call_state = "FAILED";
      break;
    default:
      call_state = "UNKNOWN";
      break;
  }

  std::cout << "Received service response in C++: " << call_state                         << "\n";
  std::cout << "Method    : " << service_response_.service_method_information.method_name << "\n";
  std::cout << "Response  : " << service_response_.response                               << "\n";
  std::cout << "Server ID : " << service_response_.server_id.service_id.entity_id         << "\n";
  std::cout << "Host      : " << service_response_.server_id.service_id.host_name         << "\n";
  std::cout << "\n";
}

int main()
{
  std::cout << "--------------------" << "\n";
  std::cout << " C++: MIRROR CLIENT"  << "\n";
  std::cout << "--------------------" << "\n";

  /*
    As always: initialize the eCAL API and give your process a name.
  */
  eCAL::Initialize("mirror client c++");

  std::cout << "eCAL " << eCAL::GetVersionString() << " (" << eCAL::GetVersionDateString() << ")" << "\n";
  eCAL::Process::SetState(eCAL::Process::eSeverity::warning, eCAL::Process::eSeverityLevel::level1, "Waiting for a service ...");

  /*
    Create a client that connects to a "mirror" server.
    It may call the methods "echo" and "reverse"
  */
  const eCAL::CServiceClient mirror_client("mirror", { {"echo", {}, {} }, {"reverse", {}, {} } });

  /*
    This lambda serves as a callback which will be executed when we receive a response from a server.
  */
  auto service_response_callback = [](const eCAL::SServiceResponse& service_response_) {
    printServiceResponse(service_response_);
  };

  /*
    We wait until the client is connected to a server,
    so we don't call methods that are not available.
  */
  while (!mirror_client.IsConnected())
  {
    std::cout << "Waiting for a service ..." << "\n";

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }

  /*
    Now that we are connected, we can set the process state to "healthy" and communicate the connection.
  */
  eCAL::Process::SetState(eCAL::Process::eSeverity::healthy, eCAL::Process::eSeverityLevel::level1, "Connected!");

  /*
    Allow to alternate between the two methods "echo" and "reverse".
  */
  unsigned int i = 0;
  std::vector<std::string> methods = { "echo", "reverse" };
  bool calls_ok = false;;

  while(eCAL::Ok())
  {
    /* 
      Alternate between the two methods "echo" and "reverse".
      Create the request payload.
    */
    std::string method_name = methods[i++ % methods.size()];
    std::string request("stressed");

    calls_ok = !mirror_client.GetClientInstances().empty();
    
    /*
      We iterate now over all client instances and call the methods by name.
      With this approach we have the option to filter out client instances that we don't want to call.
      If you want to call either way all instances, then you can use

      mirror_client.CallWithResponse(...)
      mirror_client.CallWithCallback(...)

      instead of the loop.
    */
    for (auto& client_instance : mirror_client.GetClientInstances())
    {
      /*
        Service call: blocking
        We leave the default timeout value (infinite) for the blocking call.
        You can change this for a specified timeout in ms.
      */
      const auto service_response = client_instance.CallWithResponse(method_name, request, eCAL::CClientInstance::DEFAULT_TIME_ARGUMENT);
      if (std::get<0>(service_response))
      {
        const auto& response_content = std::get<1>(service_response);
        
        printServiceResponse(response_content);
      }
      else
      {
        std::cout << "Method blocking call failed." << "\n";
        calls_ok = false;
      }

      /*
        Service call: with callback
        The callback will be executed when the server has processed the request and sent a response.
        You can again set a timeout value for an internal waiting time. By default, we wait infinitely.
      */
      if (!client_instance.CallWithCallback(method_name, request, service_response_callback, eCAL::CClientInstance::DEFAULT_TIME_ARGUMENT))
      {
        std::cout << "Method callback call failed." << "\n";
        calls_ok = false;
      }
      break;
    }

    /*
      Now we set the process state according to the result of the service calls.
      You will see the state in the eCAL Monitor or the eCAL Sys application.
    */
    if (calls_ok)
    {
      eCAL::Process::SetState(eCAL::Process::eSeverity::healthy, eCAL::Process::eSeverityLevel::level1, "Connected!");
    }
    else
    {
      eCAL::Process::SetState(eCAL::Process::eSeverity::critical, eCAL::Process::eSeverityLevel::level3, "Calls failed!");
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }

  /*
    After we are done, as always, finalize the eCAL API.
  */
  eCAL::Finalize();

  return(0);
}
