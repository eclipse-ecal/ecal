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

#include <map>
#include <cstddef>
#include <ecal/msg/capnproto/helper.h>
#include <ecal/msg/exception.h>

// capnp includes
#ifdef _MSC_VER
#pragma warning(push, 0)
#endif /*_MSC_VER*/
#include <capnp/serialize.h>
#include <capnp/message.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif /*_MSC_VER*/

namespace eCAL
{
  namespace capnproto
  {
    namespace internal
    {
      template <typename T, typename DatatypeInformation>
      class Serializer
      {
      public:
        static DatatypeInformation GetDataTypeInformation()
        {
          DatatypeInformation topic_info;
          topic_info.encoding = eCAL::capnproto::EncodingAsString();
          topic_info.name = eCAL::capnproto::TypeAsString<T>();
          topic_info.descriptor = eCAL::capnproto::SchemaAsString<T>();
          return topic_info;
        }

        size_t MessageSize(const capnp::MallocMessageBuilder& message_builder_) const
        {
          return(capnp::computeSerializedSizeInWords(const_cast<capnp::MallocMessageBuilder&>(message_builder_)) * sizeof(capnp::word));
        }

        bool Serialize(const capnp::MallocMessageBuilder& message_builder_, void* buffer_, size_t size_) const
        {
          kj::Array<capnp::word> words = capnp::messageToFlatArray(const_cast<capnp::MallocMessageBuilder&>(message_builder_));
          kj::ArrayPtr<kj::byte> bytes = words.asBytes();
          if (size_ < bytes.size()) return(false);
          memcpy(buffer_, bytes.begin(), bytes.size());
          return(true);
        }

        typename T::Reader Deserialize(const void* buffer_, size_t size_, const DatatypeInformation& /*data_type_info_*/)
        {
          try
          {
            // TODO: It seems that the MessageBuilder needs to persist.
            // We will probably have to lock this function?
            // This really needs to be analyzed!!!
            kj::ArrayPtr<const capnp::word> words = kj::arrayPtr(reinterpret_cast<const capnp::word*>(buffer_), size_ / sizeof(capnp::word));
            kj::ArrayPtr<const capnp::word> rest = initMessageBuilderFromFlatArrayCopy(words, m_msg_builder);

            // We should consider somehow validating everything
            // if (rest.size() != 0)

            typename T::Builder root_builder = typename T::Builder(m_msg_builder.getRoot<T>());
            return root_builder.asReader();
          }
          catch (...)
          {
            throw DeserializationException("Error deserializing Capnproto data.");
          }
        }

      private:
        capnp::MallocMessageBuilder m_msg_builder;
      };
      
    template <typename DatatypeInformation>
    class DynamicSerializer
    {
    public:
      static DatatypeInformation GetDataTypeInformation()
      {
        DatatypeInformation topic_info;
        topic_info.encoding = eCAL::capnproto::EncodingAsString();
        topic_info.name = "*";
        topic_info.descriptor = "*";
        return topic_info;
      }

      // This function is NOT threadsafe!!!
      // what about the lifetime of the objects?
      // It's totally unclear to me :/
      capnp::DynamicStruct::Reader Deserialize(const void* buffer_, size_t size_, const DatatypeInformation& datatype_info_)
      {
        try
        {
          // Put the pointer into a capnp::MallocMessageBuilder, it holds the memory to later access the object via a capnp::Dynami
          kj::ArrayPtr<const capnp::word> words = kj::arrayPtr(reinterpret_cast<const capnp::word*>(buffer_), size_ / sizeof(capnp::word));
          kj::ArrayPtr<const capnp::word> rest = initMessageBuilderFromFlatArrayCopy(words, m_msg_builder);

          capnp::Schema schema = GetSchema(datatype_info_);
          auto root_builder = m_msg_builder.getRoot<capnp::DynamicStruct>(schema.asStruct());
          return root_builder.asReader();
        }
        catch (...)
        {
          throw DeserializationException("Error deserializing Capnproto data.");
        }
      }

    private:
      capnp::Schema GetSchema(const DatatypeInformation& datatype_info_)
      {
        auto schema = m_schema_map.find(datatype_info_);
        if (schema == m_schema_map.end())
        {
          m_schema_map[datatype_info_] = ::eCAL::capnproto::SchemaFromDescriptor(datatype_info_.descriptor, m_loader);
        }
        return m_schema_map[datatype_info_];
      }

      capnp::MallocMessageBuilder                   m_msg_builder;
      std::map<DatatypeInformation, capnp::Schema>  m_schema_map;
      capnp::SchemaLoader                           m_loader;
    };
    }
  }
}