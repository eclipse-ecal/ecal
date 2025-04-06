/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2024 Continental Corporation
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
 * @file PubSubEventHandlingTest.cs
 * @brief Sample test demonstrating publisher and subscriber event handling.
 *
 * This sample tests that:
 *   1. When a Publisher is created with an event callback and a Subscriber is created on the same topic,
 *      the Publisher receives a connect event.
 *   2. When the Subscriber is disposed, the Publisher receives a disconnect event.
 *   3. Similarly, when a Subscriber is created with an event callback and a Publisher is created,
 *      the Subscriber receives a connect event, and when the Publisher is disposed,
 *      the Subscriber receives a disconnect event.
 */

using System;
using System.Threading.Tasks;
using System.Threading;
using Continental.eCAL.Core;
using Microsoft.VisualStudio.TestTools.UnitTesting;

[TestClass]
public class PubSubEventHandlingTest
{
  [TestInitialize]
  public void Initialize()
  {
    Core.Initialize("PubSubEventHandlingTest");
  }

  [TestCleanup]
  public void Cleanup()
  {
    Core.Terminate();
  }

  [TestMethod]
  public async Task TestPublisherEventsUsingConstructorCallback()
  {
    // TaskCompletionSources to capture connect and disconnect events.
    var pubConnectedTcs = new TaskCompletionSource<PublisherEvent>();
    var pubDisconnectedTcs = new TaskCompletionSource<PublisherEvent>();

    // Create a Publisher with an event callback passed via constructor.
    Publisher publisher = new Publisher("TestTopic",
        new DataTypeInformation(),
        new PublisherEventCallbackDelegate((topicId, eventData) =>
        {
          // Only capture events for the topic "TestTopic"
          if (topicId.TopicName == "TestTopic")
          {
            if (eventData.EventType == PublisherEvent.Connected)
              pubConnectedTcs.TrySetResult(eventData.EventType);
            else if (eventData.EventType == PublisherEvent.Disconnected)
              pubDisconnectedTcs.TrySetResult(eventData.EventType);
          }
        }));

    // Create a Subscriber on the same topic to trigger a connection.
    Subscriber subscriber = new Subscriber("TestTopic", new DataTypeInformation());

    // Wait up to 5 seconds for the Publisher to receive a Connected event.
    Task delay = Task.Delay(TimeSpan.FromSeconds(5));
    Task connectedTask = await Task.WhenAny(pubConnectedTcs.Task, delay);
    Assert.IsTrue(pubConnectedTcs.Task.IsCompleted, "Publisher did not receive a connect event.");

    // Dispose the Subscriber to trigger a disconnect.
    subscriber.Dispose();

    // Wait up to 5 seconds for the Publisher to receive a Disconnected event.
    Task disconnectedTask = await Task.WhenAny(pubDisconnectedTcs.Task, Task.Delay(TimeSpan.FromSeconds(5)));
    Assert.IsTrue(pubDisconnectedTcs.Task.IsCompleted, "Publisher did not receive a disconnect event.");

    publisher.Dispose();
  }

  [TestMethod]
  public async Task TestSubscriberEventsUsingConstructorCallback()
  {
    // TaskCompletionSources to capture connect and disconnect events.
    var subConnectedTcs = new TaskCompletionSource<SubscriberEvent>();
    var subDisconnectedTcs = new TaskCompletionSource<SubscriberEvent>();

    // Create a Subscriber with an event callback passed via constructor.
    Subscriber subscriber = new Subscriber("TestTopic",
        new DataTypeInformation(),
        new SubscriberEventCallbackDelegate((topicId, eventData) =>
        {
          // Only capture events for the topic "TestTopic"
          if (topicId.TopicName == "TestTopic")
          {
            if (eventData.EventType == SubscriberEvent.Connected)
              subConnectedTcs.TrySetResult(eventData.EventType);
            else if (eventData.EventType == SubscriberEvent.Disconnected)
              subDisconnectedTcs.TrySetResult(eventData.EventType);
          }
        }));

    // Create a Publisher on the same topic to trigger a connection.
    Publisher publisher = new Publisher("TestTopic", new DataTypeInformation());

    // Wait up to 5 seconds for the Subscriber to receive a Connected event.
    Task delay = Task.Delay(TimeSpan.FromSeconds(5));
    Task connectedTask = await Task.WhenAny(subConnectedTcs.Task, delay);
    Assert.IsTrue(subConnectedTcs.Task.IsCompleted, "Subscriber did not receive a connect event.");

    // Dispose the Publisher to trigger a disconnect.
    publisher.Dispose();

    // Wait up to 5 seconds for the Subscriber to receive a Disconnected event.
    Task disconnectedTask = await Task.WhenAny(subDisconnectedTcs.Task, Task.Delay(TimeSpan.FromSeconds(5)));
    Assert.IsTrue(subDisconnectedTcs.Task.IsCompleted, "Subscriber did not receive a disconnect event.");

    subscriber.Dispose();
  }
}
