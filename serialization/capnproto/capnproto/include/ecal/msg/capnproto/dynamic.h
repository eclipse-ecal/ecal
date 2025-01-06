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
 * @file   dynamic.h
 * @brief  eCAL dynamic subscriber interface for Cap'n Proto  message definitions
**/

#ifdef _MSC_VER
#pragma warning(push, 0)
#endif /*_MSC_VER*/
#include <capnp/schema.h>
#include <capnp/schema-loader.h>
#include <capnp/dynamic.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif /*_MSC_VER*/

#include <ecal/msg/dynamic.h>
#include <ecal/msg/capnproto/helper.h>

namespace eCAL
{

  namespace internal
  {
    class CapnprotoDynamicDeserializer
    {
    public:
      // This function is NOT threadsafe!!!
      // what about the lifetime of the objects?
      // It's totally unclear to me :/
      capnp::DynamicStruct::Reader Deserialize(const void* buffer_, size_t size_, const SDataTypeInformation& datatype_info_)
      {
        try
        {
          // Put the pointer into a capnp::MallocMessageBuilder, it holds the memory to later access the object via a capnp::Dynami
          kj::ArrayPtr<const capnp::word> words = kj::arrayPtr(reinterpret_cast<const capnp::word*>(buffer_), size_ / sizeof(capnp::word));
          kj::ArrayPtr<const capnp::word> rest = initMessageBuilderFromFlatArrayCopy(words, m_msg_builder);

          capnp::Schema schema = GetSchema(datatype_info_);
          capnp::DynamicStruct::Builder root_builder = m_msg_builder.getRoot<capnp::DynamicStruct>(schema.asStruct());
          return root_builder.asReader();
        }
        catch (...)
        {
          throw new DynamicReflectionException("Error deserializing Capnproto data.");
        }
      }

    private:
      capnp::Schema GetSchema(const SDataTypeInformation& datatype_info_)
      {
        auto schema = m_schema_map.find(datatype_info_);
        if (schema != m_schema_map.end())
        {
          m_schema_map[datatype_info_] = ::eCAL::capnproto::SchemaFromDescriptor(datatype_info_.descriptor, m_loader);
        }
        return m_schema_map[datatype_info_];
      }

      capnp::MallocMessageBuilder                   m_msg_builder;
      std::map<SDataTypeInformation, capnp::Schema> m_schema_map;
      capnp::SchemaLoader                           m_loader;
    };
  }

  namespace capnproto
  {
    /**
     * @brief  eCAL capnp subscriber class.
     *
     * Subscriber template  class for capnp messages. For details see documentation of CSubscriber class.
     *
    **/
    using CDynamicSubscriber = CDynamicMessageSubscriber<typename capnp::DynamicStruct::Reader, internal::CapnprotoDynamicDeserializer>;

    /** @example addressbook_rec.cpp
    * This is an example how to use eCAL::capnproto::CSubscriber to receive capnp data with eCAL. To receive the data, see @ref addressbook_rec.cpp .
    */
  }
}
