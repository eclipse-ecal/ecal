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

using System;
using System.Collections.Generic;
using System.Reflection;

public class TestExec
{
  static void Main()
  {
    var testCases = new Dictionary<string, List<string>>
    {
      { "ConfigTest", new List<string> { "TestConfigPassing" } },
      // Add more test classes and methods here
      // { "AnotherTestClass", new List<string> { "Test1", "Test2" } }
    };

    string assemblyPath = "Eclipse.Ecal.Core.Testd.dll";
    var assembly = Assembly.LoadFrom(assemblyPath);

    var failedTests = new List<string>();

    foreach (var kvp in testCases)
    {
      string className = kvp.Key;
      var testClass = assembly.GetType(className);
      if (testClass == null)
      {
        Console.WriteLine($"[ERROR] Test class '{className}' not found.");
        failedTests.Add($"{className} (class not found)");
        continue;
      }

      object instance = Activator.CreateInstance(testClass);

      foreach (var methodName in kvp.Value)
      {
        var method = testClass.GetMethod(methodName);
        if (method == null)
        {
          Console.WriteLine($"[ERROR] Method '{methodName}' not found in '{className}'.");
          failedTests.Add($"{className}.{methodName} (method not found)");
          continue;
        }

        try
        {
          Console.WriteLine($"Running {className}.{methodName}... ");
          object result = method.Invoke(instance, null);
          Console.WriteLine("SUCCESS");
        }
        catch (Exception ex)
        {
          Console.WriteLine("FAILED");
          Console.WriteLine($"  Exception: {ex.InnerException?.Message ?? ex.Message}");
          failedTests.Add($"{className}.{methodName}");
        }
      }
    }

    Console.WriteLine("\nTest run complete.");
    if (failedTests.Count > 0)
    {
      Console.WriteLine("Failed tests:");
      foreach (var fail in failedTests)
        Console.WriteLine($"  {fail}");
    }
    else
    {
      Console.WriteLine("All tests passed!");
    }
  }
}