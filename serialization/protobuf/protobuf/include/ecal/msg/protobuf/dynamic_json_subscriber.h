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

#include <ecal/msg/subscriber.h>
#include <ecal/msg/protobuf/dynamic_serializer.h>

namespace eCAL
{
    namespace protobuf
    {
      /**
       * @brief  eCAL protobuf dynamic subscriber class.
       *
       * Dynamic subscriber class for protobuf messages. For details see documentation of CDynamicMessageSubscriber class.
       *
      **/
      using CDynamicJSONSubscriber = CMessageSubscriber<std::string, internal::ProtobufDynamicJSONDeserializer>;

      /** @example proto_dyn_rec.cpp
      * This is an example how to use eCAL::protobuf::CDynamicSubscriber to receive dynamic protobuf data with eCAL. To receive the data, see @ref proto_dyn_rec.cpp .
      */
    }
}
