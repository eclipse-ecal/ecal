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

#include <iostream>
#include <chrono>
#include <thread>

#include <google/protobuf/util/json_util.h>

// helper function to create a serialized google message out of a message descriptor, a message type and a JSON message
std::string GetSerialzedMessageFromJSON(const std::string& msg_desc_, const std::string& msg_type_, const std::string& msg_json_)
{
  // create file descriptor set
  google::protobuf::FileDescriptorSet req_pset;
  if (!req_pset.ParseFromString(msg_desc_))
  {
    std::cerr << "Could not create google file descriptor set." << std::endl;
    return "";
  }

  // create message object
  eCAL::protobuf::CProtoDynDecoder msg_decoder;
  std::string error_s;
  google::protobuf::Message* req_msg = msg_decoder.GetProtoMessageFromDescriptorSet(req_pset, msg_type_, error_s);
  if(!req_msg)
  {
    std::cerr << "Could not create google message object." << std::endl;
    return "";
  }

  // convert JSON request string into message
  google::protobuf::util::Status status = google::protobuf::util::JsonStringToMessage(msg_json_, req_msg);
  if (!status.ok())
  {
    std::cerr << "Could not convert JSON to google message." << std::endl;
    return "";
  }

  return req_msg->SerializeAsString();
}

// helper function to create a serialized google message out of a message descriptor, a message type and a JSON message
std::string GetJSONFromSerialzedMessage(const std::string& msg_desc_, const std::string& msg_type_, const std::string& msg_google_)
{
  // create file descriptor set
  google::protobuf::FileDescriptorSet req_pset;
  if (!req_pset.ParseFromString(msg_desc_))
  {
    std::cerr << "Could not create google file descriptor set." << std::endl;
    return "";
  }

  // create message object
  eCAL::protobuf::CProtoDynDecoder msg_decoder;
  std::string error_s;
  google::protobuf::Message* req_msg = msg_decoder.GetProtoMessageFromDescriptorSet(req_pset, msg_type_, error_s);
  if (!req_msg)
  {
    std::cerr << "Could not create google message object." << std::endl;
    return "";
  }

  if (!req_msg->ParseFromString(msg_google_))
  {
    std::cerr << "Could not parse google message content from string." << std::endl;
    return "";
  }

  std::string msg_json;
  google::protobuf::util::Status status = google::protobuf::util::MessageToJsonString(*req_msg, &msg_json);
  if (!status.ok())
  {
    std::cerr << "Could not convert google message to JSON." << std::endl;
    return "";
  }

  return msg_json;
}
  
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
  if (!eCAL::Util::GetServiceTypeNames(service_name, method_name, req_type, resp_type))
  {
    std::cerr << "Could not get service type names !" << std::endl;
  }

  // get service method type descriptions
  std::string req_desc, resp_desc;
  if (!eCAL::Util::GetServiceDescription(service_name, method_name, req_desc, resp_desc))
  {
    std::cerr << "Could not get service type descriptions !" << std::endl;
  }

  int cnt(0);
  while(eCAL::Ok())
  {
    if (ping_client.IsConnected())
    {
      //////////////////////////////////////
      // create JSON request
      //////////////////////////////////////
      std::string req_json     = "{\"message\": \"HELLO WORLD FROM DYNAMIC PING CLIENT (" + std::to_string(++cnt) + ")\"}";
      std::string ping_request = GetSerialzedMessageFromJSON(req_desc, req_type, req_json);

      if (!ping_request.empty())
      {
        //////////////////////////////////////
      // Ping service (blocking call)
      //////////////////////////////////////
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
              std::string resp_json = GetJSONFromSerialzedMessage(req_desc, req_type, service_response.response);
              std::cout << "Received response PingService / Ping (JSON) : " << resp_json << " from host " << service_response.host_name << std::endl;
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
    }

    // sleep a second
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }

  // finalize eCAL API
  eCAL::Finalize();

  return(0);
}
