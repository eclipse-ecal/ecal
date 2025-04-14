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
using System.Text;
using Eclipse.eCAL.Core;

public class MinimalServiceServer
{
  static byte[] OnEchoCallback(ServiceMethodInformation methodInfo, byte[] request)
  {
    byte[] response = request;

    Console.WriteLine("Method  : '" + methodInfo.MethodName + "' called");
    Console.WriteLine("Request : " + Encoding.UTF8.GetString(request));
    Console.WriteLine("Response: " + Encoding.UTF8.GetString(response));
    Console.WriteLine();

    // Echo the request data back as the response.
    return request;
  }
  static byte[] OnReverseCallback(ServiceMethodInformation methodInfo, byte[] request)
  {
    byte[] response = (byte[])request.Clone();  // Make a copy
    Array.Reverse(response);                    // Reverse in place

    Console.WriteLine("Method  : '" + methodInfo.MethodName + "' called");
    Console.WriteLine("Request : " + Encoding.UTF8.GetString(request));
    Console.WriteLine("Response: " + Encoding.UTF8.GetString(response));
    Console.WriteLine();

    // Echo the request data back as the response.
    return response;
  }

  static void Main()
  {
    // Initialize eCAL API.
    Core.Initialize("mirror server c#");

    // Print version info.
    Console.WriteLine(string.Format("eCAL {0} ({1})\n", Core.GetVersion(), Core.GetDate()));

    // Create a service server named "service1".
    ServiceServer serviceServer = new ServiceServer("mirror");

    // Register the method callback.
    ServiceMethodInformation echoMethodInfo = new ServiceMethodInformation();
    echoMethodInfo.MethodName = "echo";
    serviceServer.SetMethodCallback(echoMethodInfo, OnEchoCallback);

    ServiceMethodInformation reverseMethodInfo = new ServiceMethodInformation();
    reverseMethodInfo.MethodName = "reverse";
    serviceServer.SetMethodCallback(reverseMethodInfo, OnReverseCallback);

    // Idle main thread until eCAL is no longer OK.
    while (Core.Ok())
    {
      System.Threading.Thread.Sleep(100);
    }

    // Dispose service server.
    serviceServer.Dispose();

    // Finalize eCAL API.
    Core.Terminate();
  }
}
