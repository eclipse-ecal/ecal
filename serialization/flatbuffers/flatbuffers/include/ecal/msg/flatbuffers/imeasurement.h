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
 * @file   subscriber.h
 * @brief  eCAL subscriber interface for google::protobuf message definitions
**/

#pragma once

#include <ecal/msg/imeasurement.h>
#include <ecal/msg/flatbuffers/serializer.h>

namespace eCAL
{
  namespace flatbuffers
  {
    /**
     * @brief  eCAL flatbuffers channel class.
    **/

    // We can specialize the channels either with the table types, or the object types.
    // Depending on which type is chosen, we need to use a differen Deserializer
    // Hence using SFINAE we can choose / declare the correct serializer

    // A traits class that selects the deserializer based on the presence of NativeTableType.
    template <typename T, typename = void>
    struct ChannelDeserializer {
      // Default: use ObjectDeserializer if T does not have NativeTableType.
      template <typename U, typename Info>
      using type = internal::ObjectDeserializer<U, Info>;
    };

    template <typename T>
    struct ChannelDeserializer<T, std::void_t<typename T::NativeTableType>> {
      // If T has a NativeTableType, use FlatDeserializer.
      template <typename U, typename Info>
      using type = internal::FlatDeserializer<U, Info>;
    };

    template <typename T>
    using IChannel = ::eCAL::measurement::IMessageChannel<
      const T*, 
      typename ChannelDeserializer<T>::template type<const T*, eCAL::experimental::measurement::base::DataTypeInformation>
    >;

    /** @example monster_read.cpp
    * This is an example how to use eCAL::protobuf::IChannel to read protobuf data from a measurement.
    */
  }
}

