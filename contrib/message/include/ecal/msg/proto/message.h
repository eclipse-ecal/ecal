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

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100 4127 4146 4505 4800 4189 4592) // disable proto warnings
#endif
#include <google/protobuf/message.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <ecal/protobuf/ecal_proto_hlp.h>
#include <ecal/msg/message.h>

namespace eCAL
{
  namespace message
  {
    // unfortunately, we need an actual object for this :/
    inline std::string GetTypeName(const google::protobuf::Message& message)
    {
      return("proto:" + message.GetTypeName());
    }

    // unfortunately, we need an actual object for this :/
    inline std::string GetDescription(const google::protobuf::Message& message)
    {
      return eCAL::protobuf::GetProtoMessageDescription(message);
    }

    inline bool Serialize(const google::protobuf::Message& message, std::string& buffer)
    {
      return message.SerializeToString(&buffer);
    }

    inline bool Deserialize(const std::string& buffer, google::protobuf::Message& message)
    {
      return message.ParseFromString(buffer);
    }
  }
}