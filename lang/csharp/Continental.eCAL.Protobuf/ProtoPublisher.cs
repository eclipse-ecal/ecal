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

using System;
using System.Text;
using System.IO;
using Google.Protobuf;
using System.Linq;

namespace Continental
{
  namespace eCAL
  {
    namespace Core
    {
      /**
       * @brief eCAL class to publish protobuf Data.
      **/
      public class ProtobufPublisher<T> where T : IMessage<T>, new()
      {
        private Publisher binaryPublisher;

        /**
        * @brief Initialize eCAL API.
        *
        * @param topicName Topic name on which the publisher publishes Data.
        **/
        public ProtobufPublisher(string topicName)
        {
          T msg = new T();
          DataTypeInformation dataTypeInfo = new DataTypeInformation(
                  Common.ProtobufHelper.GetProtoMessageTypeName(msg),
                  "proto",
                  Common.ProtobufHelper.GetProtoMessageDescription(msg).ToArray<byte>()
                );
          binaryPublisher = new Publisher(topicName, dataTypeInfo);
        }

        /**
        * @brief Send a protobuf message use the default eCAL send time.
        *
        * @param message Message to send out.
        **/
        public bool Send(T message)
        {
          var serialized = message.ToByteArray();
          return binaryPublisher.Send(serialized);
        }

        /**
         * @brief Send a protobuf message with a specified send time.
         *
         * @param message Message to send out.
         * @param time Send time in microseconds.
         * 
         * @return True if the message was sent successfully; otherwise, false.
         */
        public bool Send(T message, long time)
        {
          var serialized = message.ToByteArray();
          return binaryPublisher.Send(serialized, time);
        }
      }
    }
  }
}
