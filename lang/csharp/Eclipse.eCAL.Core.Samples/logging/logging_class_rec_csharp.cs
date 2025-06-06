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
 * @file logging_class_rec_csharp.cs
 *
 * @brief A minimal example of using the eCAL Logging GetLogging API.
 *
 * This example demonstrates how to initialize the eCAL API, print version information, get logging data.
 */

using System;
using Eclipse.eCAL.Core;
using System.Threading;

public class LoggingReceive
{
  public static void Main()
  {
    var config = Init.InitConfiguration();

    config.Logging.Receiver.Enable = true;
    config.Logging.Provider.Console.Enable = false;

    // Initialize eCAL API.
    Core.Initialize(config, "Logging Class Receive C#", Init.Flags.All);

    // Print version info.
    Console.WriteLine(String.Format("eCAL {0} ({1})\n", Core.GetVersionString(), Core.GetVersionDateString()));

    Thread logThread = new Thread(LogWorker);
    logThread.Start();

    // Idle main thread.
    while (Core.Ok())
    {
      // Get logging data.
      var logging = Logging.GetLogging();

      if (logging != null)
      {
        Console.WriteLine("Logging data received:");
        // Print logging data header.
        Console.WriteLine("  | HostName           | Process ID |         UnitName         |   Level   | Message |");
        Console.WriteLine("  ------------------------------------------------------------------------------------");
        // Print each log entry.
        foreach (var log_entry in logging.LogMessages)
        {
          Console.WriteLine(String.Format("    {0,-18} | {1,10} | {2,24} | {3,9} | {4}",
            log_entry.HostName,
            log_entry.ProcessId,
            log_entry.UnitName,
            log_entry.Level,
            log_entry.Content));
        }
      }
      else
      {
        Console.WriteLine("No logging data received.");
      }

      Console.WriteLine();

      // Sleep for a second.
      System.Threading.Thread.Sleep(1000);
    }

    logThread.Join();
    // Finalize eCAL API.
    Core.Terminate();
  }

  private static void LogWorker()
  {
    int counter = 0;
    while (Core.Ok())
    {
      // Log a message every second.
      Logging.Log(eLoggingLogLevel.Warning, "Message Nr: " + counter);
      counter++;
      Thread.Sleep(1000);
    }
  }
}
