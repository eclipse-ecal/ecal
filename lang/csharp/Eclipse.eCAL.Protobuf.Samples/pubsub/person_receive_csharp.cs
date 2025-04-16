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
 * @file person_rec_csharp.cs
 *
 * @brief A minimal example of using the eCAL API to receive protobuf messages.
 *
 * This example demonstrates how to initialize the eCAL API, print version information,
 * create a subscriber for the topic "person" (using "Pb.People.Person" as the data type), register
 * a receive callback to process incoming messages, and keep the application running until eCAL
 * is terminated. It serves as a basic reference for implementing a protobuf subscriber in C#.
 */

using System;
// include ecal core namespace
using Eclipse.eCAL.Core;

public class PersonReceive
{
  public static void Main()
  {
    Console.WriteLine("---------------------");
    Console.WriteLine(" C#: PERSON RECEIVER");
    Console.WriteLine("---------------------");

    /*
      Initialize eCAL. You always have to initialize eCAL before using its API.
      The name of our eCAL Process will be "person receive c#". 
      This name will be visible in the eCAL Monitor, once the process is running.
    */
    Core.Initialize("person receive c#");

    /*
      Print version info.
    */
    Console.WriteLine(String.Format("eCAL {0} ({1})\n", Core.GetVersion(), Core.GetDate()));

    /*
      Set the state for the program.
      You can vary between different states like healthy, warning, critical ...
      This can be used to communicate the application state to applications like eCAL Monitor/Sys.
    */
    // This function is not wrapped yet.

    /*
      Creating the eCAL Subscriber. An eCAL Process can create multiple subscribers (and publishers).
      The topic we are going to receive is called "person".
      The data type is "Pb.People.Person", generated from the protobuf definition.
    */
    var subscriber = new ProtobufSubscriber<Pb.People.Person>("person");

    /*
      Create and register a receive callback. The callback will be called whenever a new message is received.
    */
    subscriber.SetReceiveCallback((publisherId, dataTypeInfo, data) =>
    {
      Console.WriteLine(String.Format("Receiving: {0}", data.Message.ToString()));
    });

    /*
      Creating an infinite loop.
      eCAL Supports a stop signal; when an eCAL Process is stopped, eCAL::Ok() will return false.
    */
    while (Core.Ok())
    {
      /*
        Sleep for 500ms to avoid busy waiting.
      */
      System.Threading.Thread.Sleep(500);
    }

    /*
      Cleanup. Dispose the subscriber to free resources.
    */
    subscriber.Dispose();

    /*
      Terminate eCAL. This will stop all eCAL processes and free all resources.
      You should always terminate eCAL before exiting your application.
    */
    Core.Terminate();
  }
}
