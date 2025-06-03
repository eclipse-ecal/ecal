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
 * @file minimal_events_rec_csharp.cs
 *
 * @brief Sample demonstrating the use of the subscriber event API.
 *
 * This sample initializes the eCAL API, creates a Subscriber for the topic "Hello" with a 
 * DataTypeInformation for a std::string message, and subscribes to its SubscriberEvent. 
 * The event callback logs all event types (e.g. Connected, Disconnected) to the console.
 */

using System;
using System.Text;
using System.Threading;
using Eclipse.eCAL.Core;

public class MinimalSendWithEvents
{
  public static void Main()
  {
    // Initialize eCAL API.
    Core.Initialize("minimal subscriber csharp with events");

    // Print version info.
    Console.WriteLine(String.Format("eCAL {0} ({1})\n", Core.GetVersionString(), Core.GetVersionDateString()));

    // Create a subscriber with a publisher event callback passed in the constructor.
    Subscriber subscriber = new Subscriber(
        "Hello",
        new DataTypeInformation("string", "utf-8", new byte[0]),
        new SubscriberEventCallbackDelegate((topicId, eventData) =>
        {
          Console.WriteLine("Subscriber Event: {0} on topic {1} at {2} �s",
                                  eventData.EventType, topicId.TopicName, eventData.EventTime);
        })
    );

    // Register a receive callback.
    subscriber.SetReceiveCallback((publisherId, dataTypeInfo, data) =>
    {
      string message = Encoding.UTF8.GetString(data.Buffer);
      Console.WriteLine(String.Format("Receiving: {0}", message));
    });

    // Idle main thread.
    while (Core.Ok())
    {
      Thread.Sleep(100);
    }

    // Dispose subscriber.
    subscriber.Dispose();

    // Finalize eCAL API.
    Core.Terminate();
  }
}
