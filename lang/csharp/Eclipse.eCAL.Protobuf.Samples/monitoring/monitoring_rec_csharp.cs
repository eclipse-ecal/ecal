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
 * @file monitoring_rec_csharp.cs
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
    Core.Initialize("monitoring receive csharp", Init.Monitoring);

    // Print version info.
    Console.WriteLine(String.Format("eCAL {0} ({1})\n", Core.GetVersion(), Core.GetDate()));

    // Idle main thread.
    while (Core.Ok())
    {
      // Get monitoring data.
      var monitoring_bytes = Monitoring.GetMonitoring(MonitoringEntity.All);

      // Parse monitoring data.
      var monitoring = ECAL.Pb.Monitoring.Parser.ParseFrom(monitoring_bytes);

      // Print monitoring data.      
      Console.WriteLine(String.Format("Receiving: {0}", monitoring.ToString()));

      // Sleep for a second.
      System.Threading.Thread.Sleep(1000);
    }

    // Finalize eCAL API.
    Core.Terminate();
  }
}
