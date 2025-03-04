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

#pragma once

/**
 * @file ecal_clr_types_pubsub.h
 *
 * @brief Managed wrappers for eCAL pub/sub types.
 *
 * This file contains managed wrappers for native eCAL pub/sub types, including
 * the topic identifier (TopicId) and the receive callback data (ReceiveCallbackData),
 * as well as the delegate definition for subscriber receive callbacks.
 */

#include "ecal_clr_types.h"

using namespace System;

namespace Continental {
  namespace eCAL {
    namespace Core {

      /**
       * @brief Managed wrapper for the native STopicId structure.
       *
       * This class encapsulates the topic identifier, including the unique topic id
       * (extracted from SEntityId) and the topic name.
       */
      public ref class TopicId {
      public:
        /**
         * @brief Gets or sets the unique topic id.
         */
        property EntityId^ EntityID;

        /**
         * @brief Gets or sets the topic name.
         */
        property String^ TopicName;

        /**
         * @brief Default constructor.
         */
        TopicId() {}

        /**
         * @brief Constructs a TopicId with the specified values.
         *
         * @param entityID The unique topic id.
         * @param topicName The topic name.
         */
        TopicId(EntityId^ entityID, String^ topicName) {
          EntityID = entityID;
          TopicName = topicName;
        }
      };

      /**
       * @brief Managed wrapper for the native SReceiveCallbackData structure.
       *
       * This class encapsulates the payload buffer, the publisher send timestamp,
       * and the send clock.
       */
      public ref class ReceiveCallbackData {
      public:
        /**
         * @brief Gets the payload buffer as a byte array.
         */
        property array<Byte>^ Buffer;

        /**
         * @brief Gets the publisher send timestamp in microseconds.
         */
        property long long SendTimestamp;

        /**
         * @brief Gets the publisher send clock.
         */
        property long long SendClock;
      };

      /**
       * @brief Delegate for subscriber receive callbacks.
       *
       * @param publisherId Managed wrapper for the publisher's topic identifier.
       * @param dataTypeInfo Managed wrapper for the publisher's data type information.
       * @param data Managed wrapper for the received callback data.
       */
      public delegate void ReceiveCallbackDelegate(TopicId^ publisherId, DataTypeInformation^ dataTypeInfo, ReceiveCallbackData^ data);

    } // namespace Core
  } // namespace eCAL
} // namespace Continental
