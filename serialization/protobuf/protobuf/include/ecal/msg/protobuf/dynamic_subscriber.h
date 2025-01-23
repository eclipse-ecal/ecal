/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2024 Continental Corporation
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
 * @file   dynamic_subscriber.h
 * @brief  dynamic protobuf message subscriber
**/

#pragma once

#include <ecal/ecal.h>
#include <ecal/deprecate.h>
#include <ecal/msg/dynamic.h>
#include <ecal/msg/protobuf/ecal_proto_dyn.h>

#include <exception>
#include <functional>
#include <memory>
#include <ostream>
#include <sstream>
#include <string>

#ifdef _MSC_VER
#pragma warning(push, 0) // disable proto warnings
#endif
#include <google/protobuf/message.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

namespace eCAL
{
    namespace internal
    {
      class ProtobufDynamicDeserializer
      {
      public:
        std::shared_ptr<google::protobuf::Message> Deserialize(const void* buffer_, size_t size_, const SDataTypeInformation& datatype_info_)
        {
          auto message_prototype = GetMessagePointer(datatype_info_);
          // for some reason cannot use std::make_shared, however should be ok in this context.
          std::shared_ptr<google::protobuf::Message> message_with_content(message_prototype->New());
          message_with_content->CopyFrom(*message_prototype);

          try
          {
            message_with_content->ParseFromArray(buffer_, (int)size_);
            return message_with_content;
          }
          catch (...)
          {
            throw new DynamicReflectionException("Error deserializing Protobuf data.");
          }
        }

      private:
        std::shared_ptr<google::protobuf::Message> GetMessagePointer(const SDataTypeInformation& datatype_info_)
        {
          auto schema = m_message_map.find(datatype_info_);
          if (schema == m_message_map.end())
          {
            m_message_map[datatype_info_] = CreateMessagePointer(datatype_info_);
          }
          return m_message_map[datatype_info_];
        }

        std::shared_ptr<google::protobuf::Message> CreateMessagePointer(const SDataTypeInformation& topic_info_)
        {
          // get topic type
          std::string topic_type{ topic_info_.name };
          topic_type = topic_type.substr(topic_type.find_last_of('.') + 1, topic_type.size());
          if (StrEmptyOrNull(topic_type))
          {
            throw DynamicReflectionException("ProtobufDynamicDeserializer: Could not get type");
          }

          std::string topic_desc = topic_info_.descriptor;
          if (StrEmptyOrNull(topic_desc))
          {
            throw DynamicReflectionException("ProtobufDynamicDeserializer: Could not get description for type" + std::string(topic_type));
          }

          google::protobuf::FileDescriptorSet proto_desc;
          proto_desc.ParseFromString(topic_desc);
          std::string error_s;
          std::shared_ptr<google::protobuf::Message> proto_msg_ptr(m_dynamic_decoder.GetProtoMessageFromDescriptorSet(proto_desc, topic_type, error_s));
          if (proto_msg_ptr == nullptr)
          {
            std::stringstream s;
            s << "ProtobufDynamicDeserializer: Message of type " + std::string(topic_type) << " could not be decoded" << std::endl;
            s << error_s;
            throw DynamicReflectionException(s.str());
          }

          return proto_msg_ptr;
        }

        eCAL::protobuf::CProtoDynDecoder                                           m_dynamic_decoder;
        std::map<SDataTypeInformation, std::shared_ptr<google::protobuf::Message>> m_message_map;
      };
    }

    namespace protobuf
    {
      /**
       * @brief  eCAL protobuf dynamic subscriber class.
       *
       * Dynamic subscriber class for protobuf messages. For details see documentation of CDynamicMessageSubscriber class.
       *
      **/
      using CDynamicSubscriber = CDynamicMessageSubscriber<std::shared_ptr<google::protobuf::Message>, internal::ProtobufDynamicDeserializer>;

      /** @example proto_dyn_rec.cpp
      * This is an example how to use eCAL::protobuf::CDynamicSubscriber to receive dynamic protobuf data with eCAL. To receive the data, see @ref proto_dyn_rec.cpp .
      */
    }
}
