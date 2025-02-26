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
#include <flatbuffers/flatbuffers.h>

namespace eCAL
{
  namespace flatbuffers
  {
    namespace internal
    {
      template <typename ObjectType>
      class BaseSerializer
      {
      public:
        static SDataTypeInformation GetDataTypeInformation()
        {
          SDataTypeInformation topic_info;
          topic_info.encoding = "flatb";
          // empty type, empty descriptor
          return topic_info;
        }
      };

      /*
      * Serializer class for flatbuffers.
      * This class assumes that MessageSize() is called prior to Serialize(), 
      * because MessageSize is actually performing the serialization.
      * and later Serialize() is only copying the data.
      *
      * Can be used with table types, e.g. MonsterT
      */
      template <typename ObjectType>
      class ObjectSerializer 
        : public BaseSerializer<ObjectType>
      {
      public:
        size_t MessageSize(const ObjectType& msg_)
        {
          builder.Clear();
          builder.Finish(ObjectType::TableType::Pack(builder, &msg_));
          return((size_t)builder.GetSize());
        }

        bool Serialize(const ObjectType& /*msg_*/, void* buffer_, size_t size_)
        {
          if (size_ < builder.GetSize()) return(false);
          memcpy(buffer_, builder.GetBufferPointer(), builder.GetSize());
          return(true);
        }
      private:
        ::flatbuffers::FlatBufferBuilder builder;
      };


      /*
       * Deerializer class for flatbuffers.
       * This class works with Object Types, but has to be specialized with const *  only.
       * E.g. const MonsterT*
       */
      template <typename ObjectType>
      class ObjectDeserializer
        : public BaseSerializer<ObjectType>
      {
      public:
        static ObjectType Deserialize(const void* buffer_, size_t /*size_*/, const SDataTypeInformation& /*data_type_info_*/)
        {
          using CleanObjectType = std::remove_const_t<std::remove_pointer_t<ObjectType>>;

          const uint8_t* buffer = static_cast<const uint8_t*>(buffer_);
          // We should probably verify the message
          //::flatbuffers::Verifier verifier(buffer, size_);

          const CleanObjectType::TableType* table_type_ = ::flatbuffers::GetRoot<CleanObjectType::TableType>(buffer);
          return table_type_->UnPack();
        }
      };

      /*
       * Deerializer class for flatbuffers.
       * This class works with Flat Types, but has to be specialized with const *  only.
       * E.g. const Monster*
       */
      template <typename FlatType>
      class FlatDeserializer 
        : public BaseSerializer<FlatType>
      {
      public:
        static FlatType Deserialize(const void* buffer_, size_t /*size_*/, const SDataTypeInformation& /*data_type_info_*/)
        {
          using CleanFlatType = std::remove_const_t<std::remove_pointer_t<FlatType>>;
          const uint8_t* buffer = static_cast<const uint8_t*>(buffer_);

          return ::flatbuffers::GetRoot<CleanFlatType>(buffer);
        }
      };

    }
  }
}