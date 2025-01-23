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
 * @file   ecal_proto_dyn_json_sub.h
 * @brief  dynamic protobuf message to json decoder
**/

#pragma once

#include <ecal/ecal.h>
#include <ecal/os.h>
#include <ecal/msg/dynamic.h>
#include <ecal/msg/protobuf/ecal_proto_dyn.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include <memory>
#include <cstdio>
#include <string>

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

namespace eCAL
{
    namespace internal
    {
      class ProtobufDynamicJSONDeserializer
      {
      public:
        std::string Deserialize(const void* buffer_, size_t size_, const SDataTypeInformation& datatype_info_)
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
            throw new DynamicReflectionException("Error deserializing Protobuf data to json object.");
          }
        }

      private:
        std::shared_ptr<google::protobuf::util::TypeResolver> GetTypeResolver(const SDataTypeInformation& datatype_info_)
        {
          auto schema = m_type_resolver_map.find(datatype_info_);
          if (schema == m_type_resolver_map.end())
          {
            m_type_resolver_map[datatype_info_] = CreateTypeResolver(datatype_info_);
          }
          return m_type_resolver_map[datatype_info_];
        }

        std::shared_ptr<google::protobuf::util::TypeResolver> CreateTypeResolver(const SDataTypeInformation& datatype_info_)
        {
          std::string unqualified_topic_type = GetUnqualifiedTopicType(datatype_info_);

          if (StrEmptyOrNull(unqualified_topic_type))
          {
            throw DynamicReflectionException("ProtobufDynamicJSONDeserializer: Could not get type");
          }

          std::string topic_desc = datatype_info_.descriptor;
          if (StrEmptyOrNull(topic_desc))
          {
            throw DynamicReflectionException("ProtobufDynamicJSONDeserializer: Could not get description for type" + std::string(unqualified_topic_type));
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
            throw DynamicReflectionException(s.str());
          }

          return resolver;
        }

        std::string GetQualifiedTopicType(const SDataTypeInformation& data_type_info_)
        {
            return  "/" + data_type_info_.name;
        }

        std::string GetUnqualifiedTopicType(const SDataTypeInformation& data_type_info_)
        {
          const auto& type_name = data_type_info_.name;
          return  type_name.substr(type_name.find_last_of('.') + 1, type_name.size());
        }


        eCAL::protobuf::CProtoDynDecoder                                                      m_dynamic_decoder;
        std::map<SDataTypeInformation, std::shared_ptr<google::protobuf::util::TypeResolver>> m_type_resolver_map;
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
      using CDynamicJSONSubscriber = CDynamicMessageSubscriber<std::string, internal::ProtobufDynamicJSONDeserializer>;

      /** @example proto_dyn_rec.cpp
      * This is an example how to use eCAL::protobuf::CDynamicSubscriber to receive dynamic protobuf data with eCAL. To receive the data, see @ref proto_dyn_rec.cpp .
      */
    }
}
