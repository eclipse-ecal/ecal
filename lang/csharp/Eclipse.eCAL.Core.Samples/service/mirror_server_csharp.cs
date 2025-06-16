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
using Eclipse.eCAL.Core;

public class MirrorServer
{
  static void PrintCallbackInformation(string method, byte[] request, byte[] response)
  {
    Console.WriteLine("Method   : '" + method + "' called in C#");
    Console.WriteLine("Request  : " + Encoding.UTF8.GetString(request));
    Console.WriteLine("Response : " + Encoding.UTF8.GetString(response));
    Console.WriteLine();
  }

  /*
    We define the callback function that will be called when a client calls the service method "echo".
    This callback will simply return the request as the response.
  */
  static byte[] OnEchoCallback(ServiceMethodInformation methodInfo, byte[] request)
  {
    byte[] response = request;

    PrintCallbackInformation(methodInfo.MethodName, request, response);
    
    return response;
  }

  /* 
    This callback will be called when a client calls the service method "reverse".
    It will return the request in reverse order as the response.
  */
  static byte[] OnReverseCallback(ServiceMethodInformation methodInfo, byte[] request)
  {
    byte[] response = (byte[])request.Clone();
    Array.Reverse(response);

    PrintCallbackInformation(methodInfo.MethodName, request, response);

    return response;
  }

  static void Main()
  {
    Console.WriteLine("-------------------");
    Console.WriteLine(" C#: MIRROR SERVER");
    Console.WriteLine("-------------------");

    /*
      As always: initialize the eCAL API and give your process a name.
    */
    Core.Initialize("mirror server c#");

    Console.WriteLine(string.Format("eCAL {0} ({1})\n", Core.GetVersionString(), Core.GetVersionDateString()));

    /*
      Now we create the mirror server and give it the name "mirror".
    */
    ServiceServer mirrorServer = new ServiceServer("mirror");

    /*
      The server will have two methods: "echo" and "reverse".
      To set a callback, we need to set a ServiceMethodInformation struct as well as the callback function.
      In our example we will just set the method name of the struct and leave the other two fields empty.
    */
    ServiceMethodInformation echoMethodInfo = new ServiceMethodInformation();
    echoMethodInfo.MethodName = "echo";
    mirrorServer.SetMethodCallback(echoMethodInfo, OnEchoCallback);

    ServiceMethodInformation reverseMethodInfo = new ServiceMethodInformation();
    reverseMethodInfo.MethodName = "reverse";
    mirrorServer.SetMethodCallback(reverseMethodInfo, OnReverseCallback);

    /*
      Now we will go in an infinite loop, to wait for incoming service calls that will be handled with the callbacks.
    */
    while (Core.Ok())
    {
      System.Threading.Thread.Sleep(500);
    }

    /*
      When finished, we need to dispose the server to clean up properly.
    */
    mirrorServer.Dispose();

    /*
      After we are done, as always, finalize the eCAL API.
    */
    Core.Terminate();
  }
}