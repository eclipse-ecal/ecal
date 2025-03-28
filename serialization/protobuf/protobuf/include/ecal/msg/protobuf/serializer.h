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
#include <ecal/msg/exception.h>
#include <ecal/msg/protobuf/ecal_proto_hlp.h>

namespace eCAL
{
  namespace protobuf
  {
    namespace internal
    {
      template <typename T, typename DatatypeInformation>
      class Serializer
      {
      public:
        static DatatypeInformation GetDataTypeInformation()
        {
          static const DatatypeInformation datatype_info = []() {
            DatatypeInformation info{};
            T msg{};
            info.encoding = "proto";
            info.name = msg.GetTypeName();
            info.descriptor = protobuf::GetProtoMessageDescription(msg);
            return info;
          }();
          return datatype_info;
        }
        
        static bool AcceptsDataWithType(const DatatypeInformation& datatype_info_)
        {
          const auto& own_datatype_information = GetDataTypeInformation();
          return
               datatype_info_.encoding == own_datatype_information.encoding
            && datatype_info_.name == own_datatype_information.name;
        }

        static size_t MessageSize(const T& msg_)
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

        static T Deserialize(const void* buffer_, size_t size_, const DatatypeInformation& /*data_type_info_*/)
        {
          T msg;
          // we try to parse the message from the received buffer
          if (msg.ParseFromArray(buffer_, static_cast<int>(size_)))
          {
            return msg;
          }
          throw DeserializationException("Could not parse protobuf message");
        }
      };
    }
  }
}