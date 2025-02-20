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

/**
 * @file   publisher.h
 * @brief  eCAL publisher interface for google::protobuf message definitions
**/

#pragma once

#include <cstddef>
#include <ecal/msg/protobuf/ecal_proto_hlp.h>

namespace eCAL
{
  namespace protobuf
  {
    namespace internal
    {
      template <typename T>
      class Serializer
      {
      public:
        static SDataTypeInformation GetDataTypeInformation()
        {
          SDataTypeInformation topic_info;
          static T msg{};
          topic_info.encoding = "proto";
          topic_info.name = msg.GetTypeName();
          topic_info.descriptor = protobuf::GetProtoMessageDescription(msg);
          return topic_info;
        }

        static bool MessageSize(const T& msg_)
        {
#if GOOGLE_PROTOBUF_VERSION >= 3001000
          size_t size = static_cast<size_t>(msg_.ByteSizeLong());
#else
          size_t size = static_cast<size_t>(msg_.ByteSize());
#endif
          return(size);
        }

        static bool Serialize(const T& msg_, void* buffer_, size_t size_)
        {
          return msg_.SerializeToArray(buffer_, static_cast<int>(size_));
        }

        static bool Deserialize(T& msg_, const void* buffer_, size_t size_)
        {
          // we try to parse the message from the received buffer
          if (msg_.ParseFromArray(buffer_, static_cast<int>(size_)))
          {
            return(true);
          }
          return(false);
        }
      };
    }
  }
}