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

#include <iterator>
#include <string>

#ifdef _MSC_VER
#pragma warning(push, 0)
#endif /*_MSC_VER*/
#include <capnp/schema.h>
#include <capnp/schema-loader.h>
#include <capnp/dynamic.h>
#include <capnp/serialize.h>
#include <capnp/schema.capnp.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif /*_MSC_VER*/

#include <ecal/msg/dynamic.h>
#include <ecal/ecal_util.h>

namespace eCAL
{
  namespace capnproto
  {
    const std::string message_prefix{ "capnp:" };

    inline void appendMessageToString(capnp::MallocMessageBuilder& builder_, std::string& output_)
    {
      auto message_words = capnp::messageToFlatArray(builder_);
      auto message_bytes = message_words.asBytes();
      std::copy(message_bytes.begin(), message_bytes.end(), std::back_inserter(output_));
    }

    // From an abritrary type T, create a schema that contains all dependencies and
    // return it as a std::string
    // The first word (aka 8 bytes) is the id of T in Big Endian (Network) byte order.
    // Following is a serialized list of T and all its dependencies schemas.
    template <typename T>
    std::string SchemaAsString()
    {
      // Create a loader and load T and all its dependencies, 
      capnp::SchemaLoader loader;
      loader.loadCompiledTypeAndDependencies<T>();

      // Retrieve id information and save as big endian / network order
      // then save it to the descriptor string as first 8 bytes
      uint64_t id = capnp::typeId<T>();

      std::string descriptor_string;

      capnp::MallocMessageBuilder id_builder;
      auto type_builder = id_builder.getRoot<capnp::schema::Value>();
      type_builder.setUint64(id);
      appendMessageToString(id_builder, descriptor_string);

      // Serialize schema & all dependencies into string
      auto loaded = loader.getAllLoaded();
      for (auto schema : loaded)
      {
        capnp::MallocMessageBuilder builder;
        builder.setRoot(schema.getProto());
        appendMessageToString(builder, descriptor_string);
      }
      return descriptor_string;
    }

    template <typename T>
    inline std::string TypeAsString()
    {
      auto schema = capnp::Schema::from<T>();
      auto name = schema.getShortDisplayName();
      return(message_prefix + std::string(name.cStr()));
    }

    // Creates a Schema from a given descriptor, needs to be passed a Schema loader.
    inline capnp::Schema SchemaFromDescriptor(const std::string descriptor_string, capnp::SchemaLoader& loader)
    {
      kj::ArrayPtr<const capnp::word> schema_words = kj::arrayPtr(reinterpret_cast<const capnp::word*>(descriptor_string.data()), descriptor_string.size() / sizeof(capnp::word));

      // First, read out the id of the root message
      capnp::MallocMessageBuilder value_builder;
      schema_words = capnp::initMessageBuilderFromFlatArrayCopy(schema_words, value_builder);
      auto value_reader = value_builder.getRoot<capnp::schema::Value>();
      uint64_t id = value_reader.getUint64();

      // Read all messages into a builder, and load that into the loader
      while (schema_words.begin() != schema_words.end())
      {
        capnp::MallocMessageBuilder schema_builder;
        schema_words = capnp::initMessageBuilderFromFlatArrayCopy(schema_words, schema_builder);
        // Get builder root as a Node
        capnp::schema::Node::Reader reader = schema_builder.getRoot<capnp::schema::Node>();
        loader.load(reader);
      }
      // how to get the "root" schema here? So the AddressBookSchema?
      return loader.get(id);
    }


  }
}