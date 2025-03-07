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
 * @file ProtobufSubscriber.cs
 *
 * @brief eCAL class to subscribe to protobuf messages.
 *
 * This class wraps a binary Subscriber and provides a strongly-typed
 * interface for receiving protobuf messages.
 */

using System;
using System.IO;
using System.Linq;
using Google.Protobuf;

namespace Continental.eCAL.Core
{
  /**
   * @brief eCAL class to subscribe to protobuf messages.
   *
   * @tparam T The type of protobuf message.
   */
  public class ProtobufSubscriber<T> : IDisposable where T : IMessage<T>, new()
  {
    private Subscriber binarySubscriber;
    private ReceiverCallback callback;
    private bool disposed = false;

    /**
     * @brief Data structure containing a received protobuf message and its metadata.
     */
    public class ProtobufData
    {
      /**
       * @brief The received protobuf message.
       */
      public T Data { get; set; }

      /**
       * @brief The message identifier.
       */
      public long Id { get; set; }

      /**
       * @brief The message timestamp.
       */
      public long Time { get; set; }

      /**
       * @brief The message write clock.
       */
      public long Clock { get; set; }

      /**
       * @brief Constructor for ProtobufData.
       */
      public ProtobufData()
      {
        Data = new T();
      }
    };

    /**
     * @brief Signature for a data callback.
     *
     * @param topic The topic name from which the data was received.
     * @param data The received protobuf data.
     */
    public delegate void ReceiverCallback(string topic, ProtobufData data);

    /**
     * @brief Initializes a new instance of the ProtobufSubscriber class.
     *
     * @param topicName The topic name on which the subscriber subscribes to data.
     */
    public ProtobufSubscriber(string topicName)
    {
      T msg = new T();
      DataTypeInformation dataTypeInfo = new DataTypeInformation(
              Common.ProtobufHelper.GetProtoMessageTypeName(msg),
              "proto",
              Common.ProtobufHelper.GetProtoMessageDescription(msg).ToArray()
            );
      binarySubscriber = new Subscriber(topicName, dataTypeInfo);
    }

    /**
     * @brief Adds a callback function to this subscriber.
     *
     * @param callbackFunction The function to be called when new data is available.
     */
    public void SetReceiveCallback(ReceiverCallback callbackFunction)
    {
      callback = callbackFunction;
      binarySubscriber.SetReceiveCallback(InternalCallBack);
    }

    /**
     * @brief Removes the callback function from this subscriber.
     */
    public void RemoveReceiveCallback()
    {
      binarySubscriber.RemoveReceiveCallback();
      callback = null;
    }

    /**
     * @brief Internal callback that converts binary data into a protobuf message.
     *
     * @param publisherId Identifier of the publisher (contains the topic name).
     * @param dataTypeInfo Data type information.
     * @param binaryData The received binary data.
     */
    private void InternalCallBack(TopicId publisherId, DataTypeInformation dataTypeInfo, ReceiveCallbackData binaryData)
    {
      // Use the topic name from the publisher identifier.
      string topic = publisherId.TopicName;

      // Create a new instance of our Protobuf data.
      var receivedData = new ProtobufData();

      // Deserialize the protobuf message.
      using (MemoryStream msgStream = new MemoryStream(binaryData.Buffer))
      {
        receivedData.Data.MergeFrom(msgStream);
      }

      // Copy metadata
      receivedData.Id = 0;
      receivedData.Time = binaryData.SendTimestamp;
      receivedData.Clock = binaryData.SendClock;

      // Execute the user-supplied callback.
      callback?.Invoke(topic, receivedData);
    }

    /**
     * @brief Releases all resources used by the ProtobufSubscriber.
     */
    public void Dispose()
    {
      Dispose(true);
      GC.SuppressFinalize(this);
    }

    /**
     * @brief Protected Dispose method to free managed resources.
     *
     * @param disposing True if called from Dispose; false if called from the finalizer.
     */
    protected virtual void Dispose(bool disposing)
    {
      if (!disposed)
      {
        if (disposing)
        {
          if (binarySubscriber != null)
          {
            binarySubscriber.RemoveReceiveCallback();
            binarySubscriber.Dispose();
            binarySubscriber = null;
          }
        }
        disposed = true;
      }
    }

    /**
     * @brief Finalizer.
     */
    ~ProtobufSubscriber()
    {
      Dispose(false);
    }
  }
}
