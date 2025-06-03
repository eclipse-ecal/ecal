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
 * @file logging_rec_csharp.cs
 *
 * @brief A minimal example of using the eCAL Logging GetLogging API.
 *
 * This example demonstrates how to initialize the eCAL API, print version information, get logging data.
 */

using System;
using Eclipse.eCAL.Core;

public class LoggingReceive
{
  public static void Main()
  {
    // Initialize eCAL API.
    Core.Initialize("logging receive csharp");

    // Print version info.
    Console.WriteLine(String.Format("eCAL {0} ({1})\n", Core.GetVersionString(), Core.GetVersionDateString()));

    // Idle main thread.
    while (Core.Ok())
    {
      // Get logging data.
      var logging_bytes = Logging.GetLogging();

      // Parse logging data.
      var logging = ECAL.Pb.LogMessageList.Parser.ParseFrom(logging_bytes);

      // Print logging data.      
      Console.WriteLine(String.Format("Receiving: {0}", logging.ToString()));

      // Sleep for a second.
      System.Threading.Thread.Sleep(1000);
    }

    // Finalize eCAL API.
    Core.Terminate();
  }
}
