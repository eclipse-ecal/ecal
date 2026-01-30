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
 * @file hello_send.cs
 *
 * @brief A minimal example of using the eCAL API to send string messages.
 *
 * This example demonstrates how to initialize the eCAL API, print version information,
 * create a string publisher for the topic "hello", construct and send messages, and keep
 * the application running until eCAL is terminated. It serves as a basic reference for
 * implementing a string publisher in C#.
 */

using System;
using System.Threading;
// Include the eCAL API namespace
using Eclipse.eCAL.Core;

public class HelloSend
{
  public static void Main()
  {
    Console.WriteLine("------------------------");
    Console.WriteLine(" C#: HELLO WORLD SENDER");
    Console.WriteLine("------------------------");

    /*
      Initialize eCAL. You always have to initialize eCAL before using its API.
      The name of our eCAL Process will be "hello send c#". 
      This name will be visible in the eCAL Monitor, once the process is running.
    */
    Core.Initialize("hello send c#");

    /*
      Print eCAL version information.
    */
    Console.WriteLine(String.Format("eCAL {0} ({1})\n", Core.GetVersionString(), Core.GetVersionDateString()));

    /*
      Set the state for the program.
      You can vary between different states like healthy, warning, critical ...
      This can be used to communicate the application state to applications like eCAL Monitor/Sys.
    */
    Process.SetState(eProcessSeverity.Healthy, eProcessSeverityLevel.Level1, "I feel good!");

    /*
      Now we create a new publisher that will publish the topic "hello".      
    */
    StringPublisher publisher = new StringPublisher("hello");

    /*
      Creating an infinite publish-loop.
      eCAL Supports a stop signal; when an eCAL Process is stopped, eCAL_Ok() will return false.
    */
    int loop_count = 0;
    while (Core.Ok())
    {
      /*
        Construct a message. The message is a string that will be sent to the subscribers.
      */
      string message = String.Format("HELLO WORLD FROM C# ({0})", ++loop_count);

      /*
        Send the message. The message is sent to all subscribers that are currently connected to the topic "hello".
      */
      if(publisher.Send(message))
        Console.WriteLine(String.Format("Sent string message in C#: \"{0}\"", message));
      else
        Console.WriteLine("Sending string message in C# failed!");
      
      /*
        Sleep for 500ms to send in a frequency of 2 hz.
      */
      Thread.Sleep(500);
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
