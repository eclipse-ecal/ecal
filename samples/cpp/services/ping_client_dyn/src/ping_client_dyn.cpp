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

#include <google/protobuf/descriptor.h>
#include <google/protobuf/dynamic_message.h>

// main entry
int main(int argc, char **argv)
{
  // initialize eCAL API
  eCAL::Initialize(argc, argv, "ping client dynamic");

  // create ping service client
  const std::string service_name("ping service");
  const std::string method_name("Ping");
  eCAL::CServiceClient ping_client(service_name);

  // waiting for service
  while (!ping_client.IsConnected())
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::cout << "Waiting for the service .." << std::endl;
  }
  std::cout << std::endl;

  // get service method type names
  std::string req_type, resp_type;
  if (eCAL::Util::GetServiceTypeNames(service_name, method_name, req_type, resp_type))
  {
    std::cout << "Service request  type        : " << req_type  << std::endl;
    std::cout << "Service response type        : " << resp_type << std::endl;
  }
  else
  {
    std::cerr << "Could not get service type names !" << std::endl;
  }

  // get service method type descriptions
  std::string req_desc, resp_desc;
  if (eCAL::Util::GetServiceDescription(service_name, method_name, req_desc, resp_desc))
  {
    std::cout << "Service request  desc length : " << req_desc.size()  << std::endl;
    std::cout << "Service response desc length : " << resp_desc.size() << std::endl;
  }
  else
  {
    std::cerr << "Could not get service type descriptions !" << std::endl;
  }

  // TODO:
  // Create a google protobuf message object 'request_msg' based on the descriptor 'req_desc'
  // Set the attributes of that message and Call 'Ping' method

  while(eCAL::Ok())
  {
    if (ping_client.IsConnected())
    {
      //ping_client.Call(method_name, request_msg);
    }

    // sleep a second
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }

  // finalize eCAL API
  eCAL::Finalize();

  return(0);
}
