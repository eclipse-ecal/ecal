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

std::string GetSerialzedMessageFromJSON(google::protobuf::Message* msg_proto_, const std::string& msg_json_)
{
  if (!msg_proto_)
  {
    std::cerr << "Google message pointer empty." << std::endl;
    return "";
  }

  // read JSON string into message object
  auto status = google::protobuf::util::JsonStringToMessage(msg_json_, msg_proto_);
  if (!status.ok())
  {
    std::cerr << "Could not convert JSON to google message." << std::endl;
    return "";
  }

  return msg_proto_->SerializeAsString();
}

std::string GetJSONFromSerialzedMessage(google::protobuf::Message* msg_proto_, const std::string& msg_ser_)
{
  if (!msg_proto_)
  {
    std::cerr << "Google message pointer empty." << std::endl;
    return "";
  }

  // read serialized message string into message object
  if (!msg_proto_->ParseFromString(msg_ser_))
  {
    std::cerr << "Could not parse google message content from string." << std::endl;
    return "";
  }

  // convert message object to JSON string
  std::string msg_json;
  auto status = google::protobuf::util::MessageToJsonString(*msg_proto_, &msg_json);
  if (!status.ok())
  {
    std::cerr << "Could not convert google message to JSON." << std::endl;
    return "";
  }

  return msg_json;
}
