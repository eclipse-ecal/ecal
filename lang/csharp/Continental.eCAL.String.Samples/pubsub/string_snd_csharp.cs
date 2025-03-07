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
 * @file string_snd_csharp.cs
 *
 * @brief A minimal example of using the eCAL API to send string messages.
 *
 * This example demonstrates how to initialize the eCAL API, print version information,
 * create a string publisher for the topic "Hello", construct and send messages, and keep
 * the application running until eCAL is terminated. It serves as a basic reference for
 * implementing a string publisher in C#.
 */

using System;
using System.Threading;
using Continental.eCAL.Core;

public class StringSend
{
  public static void Main()
  {
    // Initialize eCAL API.
    Core.Initialize("minimal string publisher csharp");

    // Print version info.
    Console.WriteLine(String.Format("eCAL {0} ({1})\n", Core.GetVersion(), Core.GetDate()));

    // Create a string publisher (topic name "Hello").
    StringPublisher publisher = new StringPublisher("Hello");

    // Idle main thread.
    int loop = 0;
    while (Core.Ok())
    {
      // Construct the message.
      string message = String.Format("HELLO WORLD FROM C# {0,6}", ++loop);

      // Print the message.
      Console.WriteLine(String.Format("Sending: {0}", message));

      // Send the message.
      publisher.Send(message);

      // Cool down.
      Thread.Sleep(100);
    }

    // Dispose publisher.
    publisher.Dispose();

    // Finalize eCAL API.
    Core.Terminate();
  }
}
