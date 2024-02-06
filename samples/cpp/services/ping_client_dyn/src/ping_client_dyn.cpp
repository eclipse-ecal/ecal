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
#include <ecal/protobuf/ecal_proto_dyn.h>

#include <chrono>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <thread>

#include "proto_json_conv.h"

// main entry
int main(int argc, char **argv)
{
  // initialize eCAL API
  eCAL::Initialize(argc, argv, "ping client dynamic");

  // create ping service client
  const std::string service_name("ping service");
  const std::string method_name ("Ping");
  eCAL::CServiceClient ping_client(service_name);

  // waiting for service
  while (!ping_client.IsConnected())
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::cout << "Waiting for the service .." << std::endl;
  }

  // get service method type names
  std::string req_type, resp_type;
  if (!eCAL::Util::GetServiceTypeNames(service_name, method_name, req_type, resp_type))
  {
    throw std::runtime_error("Could not get service type names !");
  }

  // get service method type descriptions
  std::string req_desc, resp_desc;
  if (!eCAL::Util::GetServiceDescription(service_name, method_name, req_desc, resp_desc))
  {
    throw std::runtime_error("Could not get service type descriptions !");
  }

  // create dynamic protobuf message decoder to create request and response message objects
  std::string error_s;

  // create the request message object
  eCAL::protobuf::CProtoDynDecoder req_decoder;
  std::shared_ptr<google::protobuf::Message> req_msg(req_decoder.GetProtoMessageFromDescriptor(req_desc, req_type, error_s));
  if (!req_msg) throw std::runtime_error("Could not create request message object: " + error_s);

  // create the response message object
  eCAL::protobuf::CProtoDynDecoder resp_decoder;
  std::shared_ptr<google::protobuf::Message> resp_msg(resp_decoder.GetProtoMessageFromDescriptor(resp_desc, resp_type, error_s));
  if (!resp_msg) throw std::runtime_error("Could not create response message object: " + error_s);

  int cnt(0);
  while(eCAL::Ok() && ping_client.IsConnected())
  {
    // create JSON request
    std::string req_json = "{\"message\": \"HELLO WORLD FROM DYNAMIC PING CLIENT (" + std::to_string(++cnt) + ")\"}";
    std::string ping_request = GetSerialzedMessageFromJSON(req_msg.get(), req_json);

    if (!ping_request.empty())
    {
      // call Ping service method
      eCAL::ServiceResponseVecT service_response_vec;
      if (ping_client.Call("Ping", ping_request, -1, &service_response_vec))
      {
        std::cout << std::endl << "PingService::Ping method called with message (JSON) : " << req_json << std::endl;

        for (auto service_response : service_response_vec)
        {
          switch (service_response.call_state)
          {
          // service successful executed
          case call_state_executed:
          {
            std::string resp_json = GetJSONFromSerialzedMessage(resp_msg.get(), service_response.response);
            std::cout << "Received response PingService / Ping         (JSON) : " << resp_json << " from host " << service_response.host_name << std::endl;
          }
          break;
          // service execution failed
          case call_state_failed:
            std::cout << "Received error PingService / Ping           : " << service_response.error_msg << " from host " << service_response.host_name << std::endl;
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
