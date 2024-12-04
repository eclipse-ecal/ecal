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
using Continental.eCAL.Core;

using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.Threading.Tasks;


[TestClass]
public class ProtobufTest
{
  [TestInitialize]
  public void Initialize()
  {
    Util.Initialize("Person Test C#");
  }

  [TestCleanup]
  public void Cleanup()
  {
    Util.Terminate();
  }

  // This test ensures that 0  values which are not present on the wire, are still deserialized correctly. (#1593)
  [TestMethod]
  public void PublishSubscribeTest()
  {
    // Publisher
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

    Task.Delay(2000).Wait();

    {
      publisher.Send(person0);
      var person_rec_0 = subscriber.Receive(100);
      Assert.IsNotNull(person_rec_0);
      Assert.IsTrue(person0.Id == person_rec_0.data.Id);
    }

    {
      publisher.Send(person1);
      var person_rec_1 = subscriber.Receive(100);
      Assert.IsNotNull(person_rec_1);
      Assert.IsTrue(person1.Id == person_rec_1.data.Id);
    }

    {
      publisher.Send(person0);
      var person_rec_0 = subscriber.Receive(100);
      Assert.IsNotNull(person_rec_0);
      Assert.IsTrue(person0.Id == person_rec_0.data.Id);
    }
  }
}


