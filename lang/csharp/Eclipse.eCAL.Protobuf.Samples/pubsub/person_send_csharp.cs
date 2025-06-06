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
 * @file person_send_csharp.cs
 *
 * @brief A minimal example of using the eCAL API to send protobuf messages.
 *
 * This example demonstrates how to initialize the eCAL API, print version information,
 * create a protobuf publisher for the topic "person" (using "Person" as the data type), construct
 * and send messages, and keep the application running until eCAL is terminated. It serves as
 * a basic reference for implementing a protobuf publisher in C#.
 */

using System;
// Include the eCAL API namespace
using Eclipse.eCAL.Core;

public class PersonSend
{
  static void Main()
  {
    Console.WriteLine("-------------------");
    Console.WriteLine(" C#: PERSON SENDER");
    Console.WriteLine("-------------------");

    /*
      Initialize eCAL. You always have to initialize eCAL before using its API.
      The name of our eCAL Process will be "person send c#". 
      This name will be visible in the eCAL Monitor, once the process is running.
    */
    Core.Initialize("person send c#");

    /*
      Print version info.
    */
    Console.WriteLine(String.Format("eCAL {0} ({1})\n", Core.GetVersionString(), Core.GetVersionDateString()));

    /*
      Set the state for the program.
      You can vary between different states like healthy, warning, critical ...
      This can be used to communicate the application state to applications like eCAL Monitor/Sys.
    */
    Process.SetState(eProcessSeverity.Healthy, eProcessSeverityLevel.Level1, "I feel good!");

    /*
      Now we create a new publisher that will publish the topic "person".
      The data type is "Pb.People.Person", generated from the protobuf definition.    
    */
    var publisher = new ProtobufPublisher<Pb.People.Person>("person");

    /*
      Construct a message. The message is a protobuf struct that will be sent to the subscribers.
    */
    var person = new Pb.People.Person
    {
      Id = 0,
      Email = "max@online.de",
      Name = "Max",
      Stype = Pb.People.Person.Types.SType.Female,
      Dog = new Pb.Animal.Dog { Name = "Brandy" },
      House = new Pb.Environment.House { Rooms = 4 }
    };

    /*
      Creating an infinite publish-loop.
      eCAL Supports a stop signal; when an eCAL Process is stopped, eCAL_Ok() will return false.
    */
    int loop_count = 0;
    while (Core.Ok())
    {
      /*
        Change in each loop the content of the message to see a difference per message.
      */
      person.Id = loop_count++;

      /*
        Send the message. The message is sent to all subscribers that are currently connected to the topic "person".
      */
      if (publisher.Send(person))
      {
        Console.WriteLine("----------------------------------");
        Console.WriteLine("Sent protobuf message in C#: ");
        Console.WriteLine("----------------------------------");
        Console.WriteLine("person id    : {0}", person.Id);
        Console.WriteLine("person name  : {0}", person.Name);
        Console.WriteLine("person stype : {0}", person.Stype);
        Console.WriteLine("person email : {0}", person.Email);
        Console.WriteLine("dog.name     : {0}", person.Dog.Name);
        Console.WriteLine("house.rooms  : {0}", person.House.Rooms);
        Console.WriteLine("----------------------------------");
        Console.WriteLine("");
      }
      else
      {
        Console.WriteLine("Sending protobuf message in C# failed!");
      }

      /*
        Sleep for 500ms to send in a frequency of 2 hz.
      */
      System.Threading.Thread.Sleep(500);
    }

    /*
      Cleanup. Dispose the publisher to free resources.
    */
    publisher.Dispose();

    /*
      Terminate eCAL. This will stop all eCAL processes and free all resources.
      You should always terminate eCAL when you are done using it.
    */
    Core.Terminate();
  }
}
