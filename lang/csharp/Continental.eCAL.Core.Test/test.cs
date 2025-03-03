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

using System;
using System.Text;
using System.Threading.Tasks;
using Continental.eCAL.Core;
using Microsoft.VisualStudio.TestTools.UnitTesting;

[TestClass]
public class eCALBinaryTest
{
  [TestInitialize]
  public void Initialize()
  {
    Continental.eCAL.Core.Core.Initialize("Binary Test C#");
  }

  [TestCleanup]
  public void Cleanup()
  {
    Continental.eCAL.Core.Core.Terminate();
  }

  [TestMethod]
  public async Task PublishSubscribeTest()
  {
    // Create publisher and subscriber for topic "Hello"
    Publisher publisher = new Publisher("Hello");
    Subscriber subscriber = new Subscriber("Hello");

    // Use TaskCompletionSource to wait asynchronously for the callback
    var tcs = new TaskCompletionSource<string>();

    // Register a callback that converts the received binary buffer to a string
    subscriber.SetReceiveCallback((publisherId, dtInfo, data) =>
    {
      // Use the topic name from the publisher identifier
      string topic = publisherId.TopicName;
      // Convert the received binary data into a string
      string received = Encoding.Default.GetString(data.Buffer);
      tcs.TrySetResult(received);
    });

    string message = "HELLO WORLD FROM C#";
    // Send the message
    publisher.Send(message);

    // Wait for the callback, with a timeout 5 seconds
    Task delay = Task.Delay(TimeSpan.FromSeconds(5));
    Task completedTask = await Task.WhenAny(tcs.Task, delay);
    Assert.IsTrue(completedTask == tcs.Task, "Timeout waiting for message");
    string result = tcs.Task.Result;

    Assert.IsNotNull(result);
    Assert.AreEqual(message, result);
  }
}
