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
 * @file monitoring_class_rec_csharp.cs
 * 
 * @brief A minimal example of using the eCAL Monitoring API.
 *
 * This example demonstrates how to initialize the eCAL API, print version information, get monitoring data.
 */

using System;
using Eclipse.eCAL.Core;

public class MonitoringReceive
{
  public static void Main()
  {
    // Initialize eCAL API.
    Core.Initialize("monitoring native receive csharp", Init.Flags.Monitoring);

    // Print version info.
    Console.WriteLine(String.Format("eCAL {0} ({1})\n", Core.GetVersionString(), Core.GetVersionDateString()));

    // Idle main thread.
    while (Core.Ok())
    {
      // Get monitoring data.
      var monitoring = Monitoring.GetMonitoring(MonitoringEntity.All);

      if (monitoring != null)
      {
        // Print monitoring data header.
        Console.WriteLine("Monitoring data received:");
        // Print monitoring data header.
        Console.WriteLine("  | Process ID |    Host Name    | RegistrationClock |       Topic ID       | Topic Name |");
        Console.WriteLine("  ----------------------------------------------------------------------------------------");
        // Print each entity's monitoring data.
        if (monitoring.Publishers.Count != 0)
        {
          Console.WriteLine("  Publishers:");
          foreach (var topic in monitoring.Publishers)
          {
            Console.WriteLine(String.Format("  {0,12} | {1,-15} | {2,-17} | {3,-20} | {4, -10}",
                topic.ProcessId,
                topic.HostName,
                topic.RegistrationClock.ToString(),
                topic.TopicId,
                topic.TopicName));
          }
        }
        if (monitoring.Subscribers.Count != 0)
        {
          Console.WriteLine("  Subscribers:");
          foreach (var topic in monitoring.Subscribers)
          {
            Console.WriteLine(String.Format("  {0,12} | {1,-15} | {2,-17} | {3,-20} | {4, -10}",
                topic.ProcessId,
                topic.HostName,
                topic.RegistrationClock.ToString(),
                topic.TopicId,
                topic.TopicName));
          }
        }
        Console.WriteLine("");
      }
      else
      {
        Console.WriteLine("No monitoring data received.");
      }

      // Sleep for a second.
      System.Threading.Thread.Sleep(1000);
    }

    // Finalize eCAL API.
    Core.Terminate();
  }
}
