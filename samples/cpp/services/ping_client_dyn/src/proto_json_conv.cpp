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

#include <ecal/protobuf/ecal_proto_dyn.h>
#include <google/protobuf/util/json_util.h>

#include <iostream>
#include <string>

std::string GetSerialzedMessageFromJSON(const std::string& msg_desc_, const std::string& msg_type_, const std::string& msg_json_)
{
  // create file descriptor set
  google::protobuf::FileDescriptorSet msg_pset;
  if (!msg_pset.ParseFromString(msg_desc_))
  {
    std::cerr << "Could not create google file descriptor set." << std::endl;
    return "";
  }

  // create message object
  eCAL::protobuf::CProtoDynDecoder msg_decoder;
  std::string error_s;
  google::protobuf::Message* msg_proto = msg_decoder.GetProtoMessageFromDescriptorSet(msg_pset, msg_type_, error_s);
  if (!msg_proto)
  {
    std::cerr << "Could not create google message object: " << error_s << std::endl;
    return "";
  }

  // convert JSON string into message
  google::protobuf::util::Status status = google::protobuf::util::JsonStringToMessage(msg_json_, msg_proto);
  if (!status.ok())
  {
    std::cerr << "Could not convert JSON to google message." << std::endl;
    return "";
  }

  return msg_proto->SerializeAsString();
}

std::string GetJSONFromSerialzedMessage(const std::string& msg_desc_, const std::string& msg_type_, const std::string& msg_ser_)
{
  // create file descriptor set
  google::protobuf::FileDescriptorSet msg_pset;
  if (!msg_pset.ParseFromString(msg_desc_))
  {
    std::cerr << "Could not create google file descriptor set." << std::endl;
    return "";
  }

  // create message object
  eCAL::protobuf::CProtoDynDecoder msg_decoder;
  std::string error_s;
  google::protobuf::Message* msg_proto = msg_decoder.GetProtoMessageFromDescriptorSet(msg_pset, msg_type_, error_s);
  if (!msg_proto)
  {
    std::cerr << "Could not create google message object: " << error_s << std::endl;
    return "";
  }

  if (!msg_proto->ParseFromString(msg_ser_))
  {
    std::cerr << "Could not parse google message content from string." << std::endl;
    return "";
  }

  std::string msg_json;
  google::protobuf::util::Status status = google::protobuf::util::MessageToJsonString(*msg_proto, &msg_json);
  if (!status.ok())
  {
    std::cerr << "Could not convert google message to JSON." << std::endl;
    return "";
  }

  return msg_json;
}
