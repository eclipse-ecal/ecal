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
 * @file string_rec_csharp.cs
 *
 * @brief A minimal example of using the eCAL API to receive string messages.
 *
 * This example demonstrates how to initialize the eCAL API, print version information,
 * create a subscriber for the topic "Hello" (using "std::string" as the data type), register
 * a receive callback to process incoming messages, and keep the application running until eCAL
 * is terminated. It serves as a basic reference for implementing a string subscriber in C#.
 */

using System;
// Include the eCAL API namespace
using Continental.eCAL.Core;

public class StringReceive
{
  public static void Main()
  {
    /*
      Initialize eCAL. You always have to initialize eCAL before using its API.
      The name of our eCAL Process will be "hello_receive_csharp". 
      This name will be visible in the eCAL Monitor, once the process is running.
    */
    Core.Initialize("hello_receive_csharp");

    /*
      Creating the eCAL Subscriber. An eCAL Process can create multiple subscribers (and publishers).
      The topic we are going to receive is called "hello".
    */
    StringSubscriber subscriber = new StringSubscriber("hello");

    /*
      Creating an register a receive callback. The callback will be called whenever a new message is received.
    */
    subscriber.SetReceiveCallback((publisherId, dataTypeInfo, message) =>
    {
      Console.WriteLine(String.Format("Receiving: {0}", message));
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
