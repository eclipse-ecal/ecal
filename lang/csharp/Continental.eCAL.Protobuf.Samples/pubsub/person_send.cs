/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2019 Continental Corporation
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
 * @file person_snd_csharp.cs
 *
 * @brief A minimal example of using the eCAL API to send protobuf messages.
 *
 * This example demonstrates how to initialize the eCAL API, print version information,
 * create a protobuf publisher for the topic "person" (using "Person" as the data type), construct
 * and send messages, and keep the application running until eCAL is terminated. It serves as
 * a basic reference for implementing a protobuf publisher in C#.
 */

using System;
using Continental.eCAL.Core;
using Pb.People;

public class PersonSend
{
  static void Main()
  {
    // Initialize eCAL API.
    Core.Initialize("person publisher csharp");

    // Print version info.
    Console.WriteLine(String.Format("eCAL {0} ({1})\n", Core.GetVersion(), Core.GetDate()));

    // Create a protobuf publisher (topic name "person").
    var publisher = new ProtobufPublisher<Pb.People.Person>("person");

    int loop = 0;
    var person = new Pb.People.Person
    {
      Id = 0,
      Email = "max@online.de",
      Name = "Max",
      Stype = Pb.People.Person.Types.SType.Female,
      Dog = new Pb.Animal.Dog { Name = "Brandy" },
      House = new Pb.Environment.House { Rooms = 4 }
    };

    // Idle main thread.
    while (Core.Ok())
    {
      // message to send
      person.Id = loop;
      loop++;

      // print message
      Console.WriteLine(String.Format("Sending: {0}", person.ToString()));

      // send the content
      publisher.Send(person);

      // cool down
      System.Threading.Thread.Sleep(500);
    }

    // Dispose publisher.
    publisher.Dispose();

    // Finalize eCAL API.
    Core.Terminate();
  }
}
