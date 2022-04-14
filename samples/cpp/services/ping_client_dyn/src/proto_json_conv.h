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

#pragma once

#include <string>

/**
  * @brief Create a serialized protobuf message from a JSON string.
  *
  * @param  msg_desc_   Protobuf message descriptor.
  * @param  msg_type_   Protobuf message type name.
  * @param  msg_json_   JSON string.
  * 
  * @return serialized google message string or empty string if conversion failed
**/
std::string GetSerialzedMessageFromJSON(const std::string& msg_desc_, const std::string& msg_type_, const std::string& msg_json_);

/**
  * @brief Create a JSON string from a serialized protobuf message.
  *
  * @param  msg_desc_   Protobuf message descriptor.
  * @param  msg_type_   Protobuf message type name.
  * @param  msg_ser_    Serialized google protobuf message.
  *
  * @return JSON string or empty string if conversion failed
**/
std::string GetJSONFromSerialzedMessage(const std::string& msg_desc_, const std::string& msg_type_, const std::string& msg_ser_);
