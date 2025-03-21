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
 * @file   ecal_proto_dyn_json_sub.h
 * @brief  dynamic protobuf message to json decoder
**/

#pragma once

#include <ecal/msg/exception.h>
#include <ecal/msg/protobuf/ecal_proto_dyn.h>

#include <map>
#include <string>
#include <memory>
#include <sstream>

#ifdef _MSC_VER
#pragma warning(push, 0) // disable proto warnings
#endif
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
#include <google/protobuf/stubs/common.h>
#include <google/protobuf/util/json_util.h>
#include <google/protobuf/util/type_resolver_util.h>
#include <google/protobuf/util/type_resolver.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

namespace
{
  /* @cond */
  inline bool StrEmptyOrNull(const std::string& str)
  {
    if (str.empty())
    {
      return true;
    }
    else
    {
      for (auto c : str)
      {
        if (c != '\0')
        {
          return false;
        }
      }
      return true;
    }
  }
  /* @endcond */
}

namespace eCAL
{
  namespace protobuf
  {

    namespace internal
    {
      template <typename DatatypeInformation>
      class ProtobufDynamicJSONDeserializer
      {
      public:
        static DatatypeInformation GetDataTypeInformation()
        {
          DatatypeInformation topic_info;
          topic_info.encoding = "proto";
          topic_info.name = "*";
          topic_info.descriptor = "*";
          return topic_info;
        }

        std::string Deserialize(const void* buffer_, size_t size_, const DatatypeInformation& datatype_info_)
        {
          google::protobuf::util::JsonPrintOptions options;
#if GOOGLE_PROTOBUF_VERSION >= 5026000
          options.always_print_fields_with_no_presence = true;
#else
          options.always_print_primitive_fields = true;
#endif

          std::string binary_input;
          binary_input.assign(static_cast<const char*>(buffer_), static_cast<size_t>(size_));
          std::string json_output;
          auto status = google::protobuf::util::BinaryToJsonString(GetTypeResolver(datatype_info_).get(), GetQualifiedTopicType(datatype_info_), binary_input, &json_output, options);
          if (status.ok())
          {
            return json_output;
          }
          else
          {
            throw DeserializationException("Error deserializing Protobuf data to json object.");
          }
        }

      private:
        std::shared_ptr<google::protobuf::util::TypeResolver> GetTypeResolver(const DatatypeInformation& datatype_info_)
        {
          auto schema = m_type_resolver_map.find(datatype_info_);
          if (schema == m_type_resolver_map.end())
          {
            m_type_resolver_map[datatype_info_] = CreateTypeResolver(datatype_info_);
          }
          return m_type_resolver_map[datatype_info_];
        }

        std::shared_ptr<google::protobuf::util::TypeResolver> CreateTypeResolver(const DatatypeInformation& datatype_info_)
        {
          std::string unqualified_topic_type = GetUnqualifiedTopicType(datatype_info_);

          if (StrEmptyOrNull(unqualified_topic_type))
          {
            throw DeserializationException("ProtobufDynamicJSONDeserializer: Could not get type");
          }

          std::string topic_desc = datatype_info_.descriptor;
          if (StrEmptyOrNull(topic_desc))
          {
            throw DeserializationException("ProtobufDynamicJSONDeserializer: Could not get description for type" + std::string(unqualified_topic_type));
          }

          google::protobuf::FileDescriptorSet proto_desc;
          proto_desc.ParseFromString(topic_desc);
          std::string error_s;
          const std::shared_ptr<google::protobuf::Message> msg(m_dynamic_decoder.GetProtoMessageFromDescriptorSet(proto_desc, unqualified_topic_type, error_s));
          std::shared_ptr<google::protobuf::util::TypeResolver> resolver{ google::protobuf::util::NewTypeResolverForDescriptorPool("", m_dynamic_decoder.GetDescriptorPool()) };

          if (resolver == nullptr)
          {
            std::stringstream s;
            s << "ProtobufDynamicJSONDeserializer: Message of type " + unqualified_topic_type << " could not be decoded" << std::endl;
            s << error_s;
            throw DeserializationException(s.str());
          }

          return resolver;
        }

        std::string GetQualifiedTopicType(const DatatypeInformation& data_type_info_)
        {
          return  "/" + data_type_info_.name;
        }

        std::string GetUnqualifiedTopicType(const DatatypeInformation& data_type_info_)
        {
          const auto& type_name = data_type_info_.name;
          return  type_name.substr(type_name.find_last_of('.') + 1, type_name.size());
        }


        eCAL::protobuf::CProtoDynDecoder                                                      m_dynamic_decoder;
        std::map<DatatypeInformation, std::shared_ptr<google::protobuf::util::TypeResolver>>  m_type_resolver_map;
      };

      template <typename DatatypeInformation>
      class ProtobufDynamicDeserializer
      {
      public:
        static DatatypeInformation GetDataTypeInformation()
        {
          DatatypeInformation topic_info;
          topic_info.encoding = "proto";
          topic_info.name = "*";
          topic_info.descriptor = "*";
          return topic_info;
        }


        std::shared_ptr<google::protobuf::Message> Deserialize(const void* buffer_, size_t size_, const DatatypeInformation& datatype_info_)
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
            throw DeserializationException("Error deserializing Protobuf data.");
          }
        }

      private:
        std::shared_ptr<google::protobuf::Message> GetMessagePointer(const DatatypeInformation& datatype_info_)
        {
          auto schema = m_message_map.find(datatype_info_);
          if (schema == m_message_map.end())
          {
            m_message_map[datatype_info_] = CreateMessagePointer(datatype_info_);
          }
          return m_message_map[datatype_info_];
        }

        std::shared_ptr<google::protobuf::Message> CreateMessagePointer(const DatatypeInformation& topic_info_)
        {
          // get topic type
          std::string topic_type{ topic_info_.name };
          topic_type = topic_type.substr(topic_type.find_last_of('.') + 1, topic_type.size());
          if (StrEmptyOrNull(topic_type))
          {
            throw DeserializationException("ProtobufDynamicDeserializer: Could not get type");
          }

          std::string topic_desc = topic_info_.descriptor;
          if (StrEmptyOrNull(topic_desc))
          {
            throw DeserializationException("ProtobufDynamicDeserializer: Could not get description for type" + std::string(topic_type));
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
            throw DeserializationException(s.str());
          }

          return proto_msg_ptr;
        }

        eCAL::protobuf::CProtoDynDecoder                                           m_dynamic_decoder;
        std::map<DatatypeInformation, std::shared_ptr<google::protobuf::Message>> m_message_map;
      };



    }
  }
}
