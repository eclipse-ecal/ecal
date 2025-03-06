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
 * @file ExtendedPubSubTest.cs
 * @brief Extended tests for eCAL Publisher and Subscriber.
 *
 * This test class extends basic publish/subscribe coverage by:
 *  - Testing string and byte[] payloads.
 *  - Using an explicit send time and comparing it to the received timestamp.
 *  - Verifying behavior when no message is received (timeout).
 *  - Publishing multiple messages.
 */

using System;
using System.Collections.Generic;
using System.Text;
using System.Threading.Tasks;
using Continental.eCAL.Core;
using Microsoft.VisualStudio.TestTools.UnitTesting;

[TestClass]
public class PubSubReceiveTest
{
  [TestInitialize]
  public void Initialize()
  {
    Core.Initialize("PubSubReceiveTest");
  }

  [TestCleanup]
  public void Cleanup()
  {
    Core.Terminate();
  }

  [TestMethod]
  public async Task TestStringMessage()
  {
    Publisher publisher = new Publisher("TestTopic");
    Subscriber subscriber = new Subscriber("TestTopic");

    var tcs = new TaskCompletionSource<string>();

    subscriber.SetReceiveCallback((publisherId, dtInfo, data) =>
    {
      string received = Encoding.UTF8.GetString(data.Buffer);
      tcs.TrySetResult(received);
    });

    string message = "Hello from string message!";
    bool sendResult = publisher.Send(Encoding.UTF8.GetBytes(message));
    Assert.IsTrue(sendResult, "Send(string) failed.");

    Task delay = Task.Delay(TimeSpan.FromSeconds(5));
    Task completedTask = await Task.WhenAny(tcs.Task, delay);
    Assert.IsTrue(tcs.Task.IsCompleted, "Timeout waiting for string message.");
    Assert.AreEqual(message, tcs.Task.Result, "Received string does not match sent value.");

    publisher.Dispose();
    subscriber.Dispose();
  }

  [TestMethod]
  public async Task TestByteArrayMessage()
  {
    Publisher publisher = new Publisher("ByteTopic");
    Subscriber subscriber = new Subscriber("ByteTopic");

    var tcs = new TaskCompletionSource<string>();

    subscriber.SetReceiveCallback((publisherId, dtInfo, data) =>
    {
      string received = Encoding.UTF8.GetString(data.Buffer);
      tcs.TrySetResult(received);
    });

    string original = "Hello from byte array!";
    byte[] payload = Encoding.UTF8.GetBytes(original);
    bool sendResult = publisher.Send(payload);
    Assert.IsTrue(sendResult, "Send(byte[]) failed.");

    Task delay = Task.Delay(TimeSpan.FromSeconds(5));
    Task completed = await Task.WhenAny(tcs.Task, delay);
    Assert.IsTrue(tcs.Task.IsCompleted, "Timeout waiting for byte array message.");
    Assert.AreEqual(original, tcs.Task.Result, "Received byte array message mismatch.");

    publisher.Dispose();
    subscriber.Dispose();
  }

  [TestMethod]
  public async Task TestExplicitSendTime()
  {
    Publisher publisher = new Publisher("TimeTopic");
    Subscriber subscriber = new Subscriber("TimeTopic");

    // Use a TaskCompletionSource that returns a tuple containing the message and the send timestamp.
    var tcs = new TaskCompletionSource<(string message, long timestamp)>();

    subscriber.SetReceiveCallback((publisherId, dtInfo, data) =>
    {
      string received = Encoding.UTF8.GetString(data.Buffer);
      tcs.TrySetResult((received, data.SendTimestamp));
    });

    string message = "Message with explicit time";
    // Calculate an explicit time (for example, current time in microseconds).
    long explicitTime = DateTime.UtcNow.Ticks / 10;
    bool sendResult = publisher.Send(Encoding.UTF8.GetBytes(message), explicitTime);
    Assert.IsTrue(sendResult, "Send(string, time) failed.");

    Task delay = Task.Delay(TimeSpan.FromSeconds(5));
    Task completed = await Task.WhenAny(tcs.Task, delay);
    Assert.IsTrue(tcs.Task.IsCompleted, "Timeout waiting for explicit time message.");

    var result = tcs.Task.Result;
    Assert.AreEqual(message, result.message, "Explicit time message mismatch.");
    Assert.AreEqual(explicitTime, result.timestamp, "Timestamp mismatch: send and receive timestamps are not equal.");

    publisher.Dispose();
    subscriber.Dispose();
  }

  [TestMethod]
  public async Task TestTimeoutNoMessage()
  {
    Subscriber subscriber = new Subscriber("NoMessageTopic");
    var tcs = new TaskCompletionSource<string>();

    subscriber.SetReceiveCallback((publisherId, dtInfo, data) =>
    {
      string received = Encoding.UTF8.GetString(data.Buffer);
      tcs.TrySetResult(received);
    });

    Task delay = Task.Delay(TimeSpan.FromSeconds(2));
    Task completed = await Task.WhenAny(tcs.Task, delay);
    Assert.IsFalse(tcs.Task.IsCompleted, "Message received unexpectedly.");

    subscriber.Dispose();
  }

  [TestMethod]
  public async Task TestMultipleMessages()
  {
    Publisher publisher = new Publisher("MultiTopic");
    Subscriber subscriber = new Subscriber("MultiTopic");

    List<string> receivedMessages = new List<string>();
    int expectedMessageCount = 3;
    var tcs = new TaskCompletionSource<bool>();

    subscriber.SetReceiveCallback((publisherId, dtInfo, data) =>
    {
      string received = Encoding.UTF8.GetString(data.Buffer);
      lock (receivedMessages)
      {
        receivedMessages.Add(received);
        if (receivedMessages.Count >= expectedMessageCount)
          tcs.TrySetResult(true);
      }
    });

    string[] messages = new string[]
    {
            "First Message",
            "Second Message",
            "Third Message"
    };

    foreach (string message in messages)
    {
      bool result = publisher.Send(Encoding.UTF8.GetBytes(message));
      Assert.IsTrue(result, "Failed to send message: " + message);
      await Task.Delay(200);
    }

    Task delay = Task.Delay(TimeSpan.FromSeconds(5));
    Task completed = await Task.WhenAny(tcs.Task, delay);
    Assert.IsTrue(tcs.Task.IsCompleted, "Timeout waiting for multiple messages.");

    CollectionAssert.AreEquivalent(messages, receivedMessages, "Sent and received messages do not match.");

    publisher.Dispose();
    subscriber.Dispose();
  }
}
