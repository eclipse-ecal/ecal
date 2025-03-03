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

namespace Continental
{
  namespace eCAL
  {
    namespace Core
    {
      /**
       * @brief eCAL class to subscribe to protobuf Data.
       */
      public class ProtobufSubscriber<T> where T : IMessage<T>, new()
      {
        private Subscriber binarySubscriber;
        private ReceiverCallback callback;

        /**
         * @brief Data which is received from a callback.
         */
        public class ProtobufData
        {
          public T data;           /*!< Message */
          public long id;          /*!< Message id */
          public long time;        /*!< Message time stamp */
          public long clock;       /*!< Message write clock */

          public ProtobufData()
          {
            data = new T();
          }
        };

        /**
         * @brief Signature for a data callback.
         */
        public delegate void ReceiverCallback(string topic, ProtobufData data);

        /**
         * @brief Constructor for a Protobuf Subscriber.
         * 
         * @param topicName Topic name on which the subscriber subscribes to data.
         */
        public ProtobufSubscriber(string topicName)
        {
          T msg = new T();
          DataTypeInformation dataTypeInfo = new DataTypeInformation(
                  Common.ProtobufHelper.GetProtoMessageTypeName(msg),
                  "proto",
                  Common.ProtobufHelper.GetProtoMessageDescription(msg).ToString()
                );
          binarySubscriber = new Subscriber(topicName, dataTypeInfo);
        }

        /**
         * @brief Add a callback function to this subscriber.
         * 
         * @param callbackFunction Function which will be called when new data is available.
         */
        public void SetReceiveCallback(ReceiverCallback callbackFunction)
        {
          this.callback = callbackFunction;
          binarySubscriber.SetReceiveCallback(InternalCallBack);
        }

        /**
         * @brief Remove a callback function from this subscriber.
         */
        public void RemoveReceiveCallback()
        {
          binarySubscriber.RemoveReceiveCallback();
          this.callback = null;
        }
        
        private void InternalCallBack(TopicId publisherId, DataTypeInformation dataTypeInfo, Continental.eCAL.Core.ReceiveCallbackData binaryData)
        {
          // Use the topic name from the publisher identifier.
          string topic = publisherId.TopicName;

          // Create a new instance of our Protobuf data.
          var receivedData = new ProtobufData();
          // Create a MemoryStream from the binary data's buffer.
          using (MemoryStream msgStream = new MemoryStream(binaryData.Buffer))
          {
            receivedData.data.MergeFrom(msgStream);
          }
          // Copy metadata (set id to 0 if not provided).
          receivedData.id = 0; // Adjust if binaryData contains an id field.
          receivedData.time = binaryData.SendTimestamp;
          receivedData.clock = binaryData.SendClock;
          // Execute the user-supplied callback.
          callback(topic, receivedData);
        }

        /**
         * @brief Unregister this subscriber; no more data will be received.
         */
        public void FinalizeSubscriber()
        {
          binarySubscriber.RemoveReceiveCallback();
          binarySubscriber.Dispose();
        }
      }
    }
  }
}
