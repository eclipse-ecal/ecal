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
 * @file ProtoPublisher.cs
 *
 * @brief eCAL class to publish protobuf messages.
 *
 * This class wraps a binary Publisher and provides a strongly-typed
 * interface for sending protobuf messages.
 */

using System;
using Google.Protobuf;
using System.Linq;

namespace Eclipse.eCAL.Core
{
  public class ProtobufPublisher<T> : IDisposable where T : IMessage<T>, new()
  {
    private Publisher binaryPublisher;
    private bool disposed = false;

    /**
     * @brief Initializes a new instance of the ProtobufPublisher class.
     *
     * @param topicName Topic name on which the publisher publishes messages.
     */
    public ProtobufPublisher(string topicName)
    {
      T msg = new T();
      DataTypeInformation dataTypeInfo = new DataTypeInformation(
              Common.ProtobufHelper.GetProtoMessageTypeName(msg),
              "proto",
              Common.ProtobufHelper.GetProtoMessageDescription(msg).ToArray()
            );
      binaryPublisher = new Publisher(topicName, dataTypeInfo);
    }

    /**
     * @brief Sends a protobuf message using the default eCAL send time.
     *
     * @param message Protobuf message to send.
     *
     * @return True if the message was sent successfully; otherwise, false.
     */
    public bool Send(T message)
    {
      byte[] serialized = message.ToByteArray();
      return binaryPublisher.Send(serialized);
    }

    /**
     * @brief Sends a protobuf message with a specified send time.
     *
     * @param message Protobuf message to send.
     * @param time Send time in microseconds.
     *
     * @return True if the message was sent successfully; otherwise, false.
     */
    public bool Send(T message, long time)
    {
      byte[] serialized = message.ToByteArray();
      return binaryPublisher.Send(serialized, time);
    }

    /**
     * @brief Disposes the ProtobufPublisher and releases all associated resources.
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
          if (binaryPublisher != null)
          {
            binaryPublisher.Dispose();
            binaryPublisher = null;
          }
        }
        disposed = true;
      }
    }

    /**
     * @brief Finalizer.
     */
    ~ProtobufPublisher()
    {
      Dispose(false);
    }
  }
}
