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
 * @file ecal_clr_publisher.h
 * @brief Managed wrapper for the native eCAL::CPublisher class.
 *
 * This class provides methods to send messages over eCAL and query information about the publisher.
 */

#include "ecal_clr_types_pubsub.h"

#include <ecal/pubsub/publisher.h>

using namespace System;

namespace Continental {
  namespace eCAL {
    namespace Core {

      /**
       * @brief Managed wrapper for the native eCAL::CPublisher class.
       */
      public ref class Publisher
      {
      public:
        /**
         * @brief Constant used as the default time argument to let eCAL determine the send timestamp.
         */
        literal long long DEFAULT_TIME_ARGUMENT = -1;

        /**
         * @brief Constructs a Publisher with the given topic name.
         *
         * @param topicName Unique topic name.
         */
        Publisher(String^ topicName);

        /**
         * @brief Constructs a Publisher with the given topic name and data type information.
         *
         * @param topicName Unique topic name.
         * @param dataTypeInfo Topic data type information.
         */
        Publisher(String^ topicName, DataTypeInformation^ dataTypeInfo);

        /**
         * @brief Constructs a Publisher with the given topic name, data type information, and an event callback.
         *
         * @param topicName Unique topic name.
         * @param dataTypeInfo Topic data type information.
         * @param eventCallback Optional publisher event callback.
         */
        Publisher(String^ topicName, DataTypeInformation^ dataTypeInfo, PublisherEventCallbackDelegate^ eventCallback);

        /**
         * @brief Destructor.
         */
        ~Publisher();

        /**
         * @brief Finalizer.
         */
        !Publisher();

        /**
         * @brief Sends a message to all subscribers using a byte array payload (using eCAL time).
         *
         * @param data Payload as a byte array.
         *
         * @return True if the send operation succeeded; otherwise false.
         */
        bool Send(array<Byte>^ data);

        /**
         * @brief Sends a message to all subscribers using a byte array payload.
         *
         * @param data Payload as a byte array.
         * @param time Send time in microseconds.
         * 
         * @return True if the send operation succeeded; otherwise false.
         */
        bool Send(array<Byte>^ data, long long time);

        /**
         * @brief Queries the number of subscribers.
         *
         * @return The number of subscribers.
         */
        int GetSubscriberCount();

        /**
         * @brief Retrieves the topic name.
         *
         * @return The topic name as a System::String^.
         */
        String^ GetTopicName();

        /**
         * @brief Retrieves the topic identifier.
         *
         * @return An instance of TopicId containing the topic id details.
         */
        TopicId^ GetTopicId();

        /**
         * @brief Retrieves the topic data type information.
         *
         * @return An instance of DataTypeInformation containing the data type details.
         */
        DataTypeInformation^ GetDataTypeInformation();

      private:
        /**
         * @brief Pointer to the native CPublisher instance.
         */
        ::eCAL::CPublisher* m_native_publisher;
      };

    } // namespace Core
  } // namespace eCAL
} // namespace Continental
