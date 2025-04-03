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
 * @file minimal_rec_csharp.cs
 *
 * @brief A minimal example of using the eCAL API to receive messages.
 *
 * This example demonstrates how to initialize the eCAL API, print version information,
 * create a subscriber for the topic "Hello" (using "std::string" as the data type), register
 * a receive callback to process incoming messages, and keep the application running until eCAL
 * is terminated. It serves as a basic reference for implementing a subscriber in C#.
 */

using System;
using System.Text;
using Continental.eCAL.Core;

public class MinimalReceive
{
  public static void Main()
  {
    // Initialize eCAL API.
    Core.Initialize("minimal subscriber csharp");

    // Print version info.
    Console.WriteLine(String.Format("eCAL {0} ({1})\n", Core.GetVersion(), Core.GetDate()));

    // Create a subscriber (topic name "Hello", type "string", encoding "utf-8", description "")
    Subscriber subscriber = new Subscriber("Hello", new DataTypeInformation("string", "utf-8", new byte[0]));

    // Register a receive callback.
    subscriber.SetReceiveCallback((publisherId, dataTypeInfo, data) =>
    {
      string message = Encoding.UTF8.GetString(data.Buffer);
      Console.WriteLine(String.Format("Receiving: {0}", message));
    });

    // Idle main thread.
    while (Core.Ok())
    {
      System.Threading.Thread.Sleep(100);
    }

    // Dispose subscriber.
    subscriber.Dispose();

    // Finalize eCAL API.
    Core.Terminate();
  }
}
