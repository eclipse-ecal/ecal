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
using System.Text;
using System.Reflection;
using Eclipse.eCAL.Core;

public class TestExec
{
  static void Main()
  {
    var assembly = Assembly.LoadFrom("Eclipse.Ecal.Core.Testd.dll");
    var testClass = assembly.GetType("ConfigTest");
    
// Iterate through all types in the assembly
    foreach (Type type in assembly.GetTypes())
    {
      Console.WriteLine($"Type: {type.FullName}");

              // Iterate through all methods in the type
      foreach (MethodInfo method in type.GetMethods())
      {
        Console.WriteLine($"  Method: {method.Name}");
      }
    }

    var testMethod = testClass.GetMethod("TestConfigPassing");
    var instance = Activator.CreateInstance(testClass);
    
// Invoke the method  
    object result = testMethod.Invoke(instance, null);
// Display the result
    Console.WriteLine(result);

  }
}