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
using System.Threading.Tasks;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Continental.eCAL.Core;
using Google.Protobuf;

[TestClass]
public class ProtobufTest
{
  [TestInitialize]
  public void Initialize()
  {
    Core.Initialize("Person Test C#");
  }

  [TestCleanup]
  public void Cleanup()
  {
    Core.Terminate();
  }

  [TestMethod]
  public async Task PublishSubscribeTest()
  {
    // Create publisher and subscriber for topic "example_topic"
    var publisher = new ProtobufPublisher<Pb.People.Person>("example_topic");
    var subscriber = new ProtobufSubscriber<Pb.People.Person>("example_topic");

    var person0 = new Pb.People.Person
    {
      Id = 0,
      Email = "max@online.de",
      Name = "Max",
      Stype = Pb.People.Person.Types.SType.Female,
      Dog = new Pb.Animal.Dog { Name = "Brandy" },
      House = new Pb.Environment.House { Rooms = 4 }
    };

    var person1 = new Pb.People.Person
    {
      Id = 1,
      Email = "max@online.de",
      Name = "Max",
      Stype = Pb.People.Person.Types.SType.Female,
      Dog = new Pb.Animal.Dog { Name = "Brandy" },
      House = new Pb.Environment.House { Rooms = 4 }
    };

    // Allow time for subscriber initialization.
    await Task.Delay(2000);

    // Test: publish person0, then wait for callback.
    {
      var tcs = new TaskCompletionSource<Pb.People.Person>();
      subscriber.SetReceiveCallback((publisherId, dataTypeInfo, data) =>
      {
        tcs.TrySetResult(data.Message);
      });
      publisher.Send(person0);
      var completedTask = await Task.WhenAny(tcs.Task, Task.Delay(100));
      Assert.IsTrue(tcs.Task.IsCompleted, "Timeout waiting for person0");
      Assert.AreEqual(person0.Id, tcs.Task.Result.Id);
      subscriber.RemoveReceiveCallback();
    }

    // Test: publish person1, then wait for callback.
    {
      var tcs = new TaskCompletionSource<Pb.People.Person>();
      subscriber.SetReceiveCallback((publisherId, dataTypeInfo, data) =>
      {
        tcs.TrySetResult(data.Message);
      });
      publisher.Send(person1);
      var completedTask = await Task.WhenAny(tcs.Task, Task.Delay(100));
      Assert.IsTrue(tcs.Task.IsCompleted, "Timeout waiting for person1");
      Assert.AreEqual(person1.Id, tcs.Task.Result.Id);
      subscriber.RemoveReceiveCallback();
    }

    // Test: publish person0 again, then wait for callback.
    {
      var tcs = new TaskCompletionSource<Pb.People.Person>();
      subscriber.SetReceiveCallback((publisherId, dataTypeInfo, data) =>
      {
        tcs.TrySetResult(data.Message);
      });
      publisher.Send(person0);
      var completedTask = await Task.WhenAny(tcs.Task, Task.Delay(100));
      Assert.IsTrue(tcs.Task.IsCompleted, "Timeout waiting for person0 (second time)");
      Assert.AreEqual(person0.Id, tcs.Task.Result.Id);
      subscriber.RemoveReceiveCallback();
    }
  }
}
