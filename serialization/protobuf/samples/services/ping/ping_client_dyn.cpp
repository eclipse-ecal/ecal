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
#include <ecal/msg/protobuf/ecal_proto_dyn.h>

#include <chrono>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <thread>

#include "proto_json_conv.h"

// main entry
int main()
{
  // initialize eCAL API
  eCAL::Initialize("ping client dynamic");

  // create ping service client
  const std::string service_name("ping service");
  const std::string method_name ("Ping");
  eCAL::CServiceClient ping_client(service_name);

  // waiting for service
  while (eCAL::Ok() && !ping_client.IsConnected())
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::cout << "Waiting for the service .." << '\n';
  }

  // get service method type names
  std::string req_type;
  std::string req_desc;
  std::string resp_type;
  std::string resp_desc;

  std::set<eCAL::SServiceId> service_ids;
  eCAL::Registration::GetServerIDs(service_ids);
  bool service_info_found(false);
  for (const auto& service_id : service_ids)
  {
    if (service_id.service_name == service_name)
    {
      eCAL::ServiceMethodInformationSetT methods;
      eCAL::Registration::GetServerInfo(service_id, methods);

      for (const auto& method : methods)
      {
        if (method.method_name == method_name)
        {
          req_type = method.request_type.name;
          req_desc = method.request_type.descriptor;
          resp_type = method.response_type.name;
          resp_desc = method.response_type.descriptor;
          service_info_found = true;
          break;
        }
      }
    }
  }
  if (!service_info_found)
  {
    throw std::runtime_error("Could not get service type names and service type descriptions!");
  }

  // create dynamic protobuf message decoder to create request and response message objects
  std::string error_s;

  // create the request message object
  eCAL::protobuf::CProtoDynDecoder req_decoder;
  const std::shared_ptr<google::protobuf::Message> req_msg(req_decoder.GetProtoMessageFromDescriptor(req_desc, req_type, error_s));
  if (!req_msg) throw std::runtime_error("Could not create request message object: " + error_s);

  // create the response message object
  eCAL::protobuf::CProtoDynDecoder resp_decoder;
  const std::shared_ptr<google::protobuf::Message> resp_msg(resp_decoder.GetProtoMessageFromDescriptor(resp_desc, resp_type, error_s));
  if (!resp_msg) throw std::runtime_error("Could not create response message object: " + error_s);

  int cnt(0);
  while(eCAL::Ok() && ping_client.IsConnected())
  {
    // create JSON request
    const std::string req_json = R"({"message": "HELLO WORLD FROM DYNAMIC PING CLIENT ()" + std::to_string(++cnt) + ")\"}";
    const std::string ping_request = GetSerialzedMessageFromJSON(req_msg.get(), req_json);

    if (!ping_request.empty())
    {
      // call Ping service method
      eCAL::ServiceResponseVecT service_response_vec;
      if (ping_client.CallWithResponse("Ping", ping_request, service_response_vec))
      {
        std::cout << '\n' << "PingService::Ping method called with message (JSON) : " << req_json << '\n';

        for (const auto& service_response : service_response_vec)
        {
          switch (service_response.call_state)
          {
          // service successful executed
          case eCAL::eCallState::executed:
          {
            const std::string resp_json = GetJSONFromSerialzedMessage(resp_msg.get(), service_response.response);
            std::cout << "Received response PingService / Ping         (JSON) : " << resp_json << " from host " << service_response.server_id.service_id.host_name << '\n';
          }
          break;
          // service execution failed
          case eCAL::eCallState::failed:
            std::cout << "Received error PingService / Ping           : " << service_response.error_msg << " from host " << service_response.server_id.service_id.host_name << '\n';
            break;
          default:
            break;
          }
        }
      }
      else
      {
        std::cout << "PingService::Ping method call failed .." << '\n' << '\n';
      }
    }

    // sleep a second
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }

  // finalize eCAL API
  eCAL::Finalize();

  return(0);
}
