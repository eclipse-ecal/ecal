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
 * @file minimal_snd_csharp.cs
 *
 * @brief Sample demonstrating the use of the publisher API.
 *
 * This sample initializes the eCAL API, creates a Publisher for the topic "Hello" with a 
 * DataTypeInformation for a std::string message. 
 * Then, the publisher sends messages in a loop.
 */

using System;
using System.Text;
using System.Threading;
using Continental.eCAL.Core;

public class MinimalSend
{
  public static void Main()
  {
    // Initialize eCAL API.
    Core.Initialize("minimal publisher csharp");

    // Print version info.
    Console.WriteLine(String.Format("eCAL {0} ({1})\n", Core.GetVersion(), Core.GetDate()));

    // Create a publisher (topic name "Hello", type "std::string", encoding "base", description "").
    Publisher publisher = new Publisher("Hello", new DataTypeInformation("std::string", "base", new byte[0]));

    // Idle main thread.
    int loop = 0;
    while (Core.Ok())
    {
      // Construct the message.
      string message = String.Format("HELLO WORLD FROM C# {0,6}", ++loop);

      // Print the message.
      Console.WriteLine(String.Format("Sending: {0}", message));

      // Send the message.
      publisher.Send(Encoding.UTF8.GetBytes(message));

      // Cool down.
      Thread.Sleep(100);
    }

    // Dispose publisher.
    publisher.Dispose();

    // Finalize eCAL API.
    Core.Terminate();
  }
}
