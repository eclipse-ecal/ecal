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
using System.Text;
using Eclipse.eCAL.Core;

public class MirrorClient
{
  // Print a service response
  static void PrintServiceResponse(ServiceResponse serviceResponse)
  {
    string callState;
    switch (serviceResponse.CallState)
    {
      case CallState.Executed:
        callState = "EXECUTED";
        break;
      case CallState.Failed:
        callState = "FAILED";
        break;
      default:
        callState = "UNKNOWN";
        break;
    }

    Console.WriteLine("Received service response in C#: " + callState);
    Console.WriteLine("Method    : " + serviceResponse.MethodInformation.MethodName);
    Console.WriteLine("Response  : " + Encoding.UTF8.GetString(serviceResponse.Response));
    Console.WriteLine("Server ID : " + serviceResponse.ServerId.EntityID.Id);
    Console.WriteLine("Host      : " + serviceResponse.ServerId.EntityID.HostName);
    Console.WriteLine();
  }

  static void Main()
  {
    Console.WriteLine("-------------------");
    Console.WriteLine(" C#: MIRROR CLIENT");
    Console.WriteLine("-------------------");

    // Initialize eCAL API
    Core.Initialize("mirror client c#");
    Console.WriteLine(String.Format("eCAL {0} ({1})\n", Core.GetVersion(), Core.GetDate()));

    // Prepare the method information list (echo + reverse)
    ServiceMethodInformationList methodInformationList = new ServiceMethodInformationList();
    methodInformationList.Methods.Add(new ServiceMethodInformation("echo", new DataTypeInformation(), new DataTypeInformation()));
    methodInformationList.Methods.Add(new ServiceMethodInformation("reverse", new DataTypeInformation(), new DataTypeInformation()));
    
    // Create the client
    ServiceClient mirrorClient = new ServiceClient("mirror", methodInformationList);
    
    // Wait until we have at least one server connected
    while(!mirrorClient.IsConnected())
    {
      System.Threading.Thread.Sleep(100);
    }

    // Alternate calls between "echo" and "reverse"
    int i = 0;
    string[] methods = new string[] { "echo", "reverse" };

    while (Core.Ok())
    {
      string method = methods[i++ % methods.Length];
      byte[] request = Encoding.UTF8.GetBytes("stressed");

      // Call with response
      List<ServiceResponse> responseList = mirrorClient.CallWithResponse(
        method,
        request,
        (int)ServiceClient.DefaultTimeArgument);
      
      // Iterate through all responses and print them
      if (responseList.Count > 0)
      {
        foreach (ServiceResponse response in responseList)
        {
          PrintServiceResponse(response);
        }
      }
      else
      {
        Console.WriteLine("Method blocking call failed.");
      }

      System.Threading.Thread.Sleep(1000);
    }

    // Dispose the client to clean up properly
    mirrorClient.Dispose();

    // Finalize eCAL API
    Core.Terminate();
  }
}
