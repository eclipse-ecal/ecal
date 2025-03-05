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

using System;
using Continental.eCAL.Core;

public class MinimalSend
{
  static void Main()
  {
    // Initialize eCAL API.
    Core.Initialize("minimal publisher csharp");

    // Print version info.
    Console.WriteLine(String.Format("eCAL {0} ({1})\n", Core.GetVersion(), Core.GetDate()));

    // Create a publisher (topic name "Hello", type "base:std::string", description "")
    Publisher publisher = new Publisher("Hello", new DataTypeInformation("std::string", "base", Array.Empty<byte>()));

    // Idle main thread.
    int loop = 0;
    while (Core.Ok())
    {
      // message to send
      string message = String.Format("HELLO WORLD FROM C# {0,6}", ++loop);

      // print message
      System.Console.WriteLine(String.Format("Sending:  {0}", message));

      // send the message
      publisher.Send(message);

      // cool down
      System.Threading.Thread.Sleep(100);
    }

    // Dispose publisher.
    publisher.Dispose();

    // finalize eCAL API.
    Core.Terminate();
  }
}
