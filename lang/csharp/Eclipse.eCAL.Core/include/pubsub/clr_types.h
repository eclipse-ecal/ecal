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
 * @file clr_types.h
 *
 * @brief Managed wrappers for eCAL pub/sub types.
 *
 * This file contains managed wrappers for native eCAL pub/sub types, including
 * the topic identifier (TopicId) and the receive callback data (ReceiveCallbackData),
 * as well as the delegate definition for subscriber receive callbacks and definitions
 * for publisher/subscriber events.
 */

#include "../clr_types.h"

using namespace System::Collections::Generic;

namespace Eclipse {
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
        property System::String^ TopicName;

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
        TopicId(EntityId^ entityID, System::String^ topicName) {
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
        property array<System::Byte>^ Buffer;

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

      /**
       * @brief Managed enumeration for publisher events.
       */
      public enum class PublisherEvent {
        None = 0,         /**< No event. */
        Connected = 1,    /**< A new subscriber has been connected to the publisher. */
        Disconnected = 2, /**< A previously connected subscriber has been disconnected from the publisher. */
        Dropped = 3       /**< A subscriber has missed a message. */
      };

      /**
       * @brief Managed enumeration for subscriber events.
       */
      public enum class SubscriberEvent {
        None = 0,         /**< No event. */
        Connected = 1,    /**< A new publisher has been connected to the subscriber. */
        Disconnected = 2, /**< A previously connected publisher has been disconnected from the subscriber. */
        Dropped = 3       /**< A message from a publisher has been dropped. */
      };

      /**
       * @brief Managed wrapper for the native SPubEventCallbackData structure.
       *
       * This class encapsulates publisher event callback information.
       */
      public ref class PubEventCallbackData {
      public:
        /**
         * @brief Gets or sets the publisher event type.
         */
        property PublisherEvent EventType;
        /**
         * @brief Gets or sets the event time in microseconds.
         */
        property long long EventTime;
        /**
         * @brief Gets or sets the subscriber's data type information.
         */
        property DataTypeInformation^ SubscriberDataType;
      };

      /**
       * @brief Delegate for publisher event callbacks.
       *
       * @param topicId Managed wrapper for the topic identifier.
       * @param data Managed wrapper for the publisher event callback data.
       */
      public delegate void PublisherEventCallbackDelegate(TopicId^ topicId, PubEventCallbackData^ data);

      /**
       * @brief Managed wrapper for the native SSubEventCallbackData structure.
       *
       * This class encapsulates subscriber event callback information.
       */
      public ref class SubEventCallbackData {
      public:
        /**
         * @brief Gets or sets the subscriber event type.
         */
        property SubscriberEvent EventType;
        /**
         * @brief Gets or sets the event time in microseconds.
         */
        property long long EventTime;
        /**
         * @brief Gets or sets the publisher's data type information.
         */
        property DataTypeInformation^ PublisherDataType;
      };

      /**
       * @brief Delegate for subscriber event callbacks.
       *
       * @param topicId Managed wrapper for the topic identifier.
       * @param data Managed wrapper for the subscriber event callback data.
       */
      public delegate void SubscriberEventCallbackDelegate(TopicId^ topicId, SubEventCallbackData^ data);

    } // namespace Core
  } // namespace eCAL
} // namespace Eclipse
