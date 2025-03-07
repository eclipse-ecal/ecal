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
 * @file StringPublisher.cs
 *
 * @brief eCAL class to publish string data.
 *
 * This class wraps a binary Publisher and provides a strongly-typed
 * interface for sending string messages.
 */

using System;
using System.Text;

namespace Continental.eCAL.Core
{
  public class StringPublisher : IDisposable
  {
    private Publisher binaryPublisher;
    private bool disposed = false;

    /**
     * @brief Initializes a new instance of the StringPublisher class.
     *
     * @param topicName Topic name on which the publisher publishes data.
     */
    public StringPublisher(string topicName)
    {
      // Create a publisher for the given topic using a std::string type.
      binaryPublisher = new Publisher(topicName, new DataTypeInformation("std::string", "base", new byte[0]));
    }

    /**
     * @brief Sends a string message using the default eCAL send time.
     *
     * @param message Message to send.
     * @return True if the message was sent successfully; otherwise, false.
     */
    public bool Send(string message)
    {
      byte[] array = Encoding.UTF8.GetBytes(message);
      return binaryPublisher.Send(array);
    }

    /**
     * @brief Sends a string message with a specified send time.
     *
     * @param message Message to send.
     * @param time Send time in microseconds.
     * @return True if the message was sent successfully; otherwise, false.
     */
    public bool Send(string message, long time)
    {
      byte[] array = Encoding.UTF8.GetBytes(message);
      return binaryPublisher.Send(array, time);
    }

    /**
     * @brief Disposes the StringPublisher and releases all associated resources.
     */
    public void Dispose()
    {
      Dispose(true);
      GC.SuppressFinalize(this);
    }

    /**
     * @brief Protected Dispose method to free managed resources.
     *
     * @param disposing True if called from Dispose; false if called from finalizer.
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
    ~StringPublisher()
    {
      Dispose(false);
    }
  }
}
