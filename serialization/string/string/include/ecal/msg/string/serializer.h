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

namespace eCAL
{
  namespace string
  {
    namespace internal
    {
      template <typename T>
      class Serializer
      {
      public:
        static SDataTypeInformation GetDataTypeInformation()
        {
          SDataTypeInformation data_type_info;
          data_type_info.encoding = "base";
          data_type_info.name = "std::string";
          // empty descriptor
          return data_type_info;
        }

        static size_t MessageSize(const T& msg_)
        {
          return(msg_.size());
        }

        static bool Serialize(const T& msg_, void* buffer_, size_t size_)
        {
          if (msg_.size() <= size_)
          {
            memcpy(buffer_, msg_.c_str(), msg_.size());
            return(true);
          }
          return(false);
        }

        static bool Deserialize(T& msg_, const void* buffer_, size_t size_)
        {
          msg_ = std::string(static_cast<const char*>(buffer_), size_);
          return true;
        }
      };
    }
  }
}