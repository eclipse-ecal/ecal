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
 * @file StringSubscriber.cs
 *
 * @brief eCAL class to subscribe to string data.
 *
 * This class wraps a binary Subscriber and provides a strongly-typed interface
 * for receiving string messages. It implements IDisposable to ensure that resources
 * are properly released.
 */

using System;
using System.Text;
using Continental.eCAL.Core;

namespace Continental.eCAL.Core
{
  public class StringSubscriber : IDisposable
  {
    private Subscriber binarySubscriber;
    private bool disposed = false;

    /**
     * @brief Initializes a new instance of the StringSubscriber class.
     *
     * @param topicName Topic name on which to subscribe.
     */
    public StringSubscriber(string topicName)
    {
      // Create a subscriber for the given topic using a std::string type.
      binarySubscriber = new Subscriber(topicName, new DataTypeInformation("std::string", "base", new byte[0]));
    }

    /**
     * @brief Registers a receive callback.
     *
     * The provided callback is invoked whenever a message is received. The payload is
     * converted from a byte array to a UTF8 string.
     *
     * @param callback The callback to be invoked with the received string.
     */
    public void SetReceiveCallback(Action<string> callback)
    {
      binarySubscriber.SetReceiveCallback((publisherId, dataTypeInfo, data) =>
      {
        string message = Encoding.UTF8.GetString(data.Buffer);
        callback(message);
      });
    }

    /**
     * @brief Removes the currently registered receive callback.
     *
     * @return True if the callback was successfully removed; otherwise, false.
     */
    public bool RemoveReceiveCallback()
    {
      return binarySubscriber.RemoveReceiveCallback();
    }

    /**
     * @brief Disposes the StringSubscriber.
     */
    public void Dispose()
    {
      Dispose(true);
      GC.SuppressFinalize(this);
    }

    /**
     * @brief Protected Dispose method.
     *
     * @param disposing True if called from Dispose; false if called from finalizer.
     */
    protected virtual void Dispose(bool disposing)
    {
      if (!disposed)
      {
        if (disposing)
        {
          if (binarySubscriber != null)
          {
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
    ~StringSubscriber()
    {
      Dispose(false);
    }
  }
}
