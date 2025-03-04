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
using System.Collections.Generic;
using System.Text;
using Continental.eCAL.Core;

public class MinimalServiceClient
{
  // Print the result of a service call.
  static void PrintServiceResponse(ServiceResponse serviceResponse)
  {
    switch (serviceResponse.CallState)
    {
      case CallState.Executed:
        Console.WriteLine("Received response for method " +
            serviceResponse.MethodInformation.MethodName +
            ": " + Encoding.UTF8.GetString(serviceResponse.Response) +
            " from host " + serviceResponse.ServerId.EntityID.HostName);
        break;
      case CallState.Failed:
        Console.WriteLine("Received error: " + serviceResponse.ErrorMessage +
            " from host " + serviceResponse.ServerId.EntityID.HostName);
        break;
      default:
        Console.WriteLine("Received response in unknown state.");
        break;
    }
  }

  static void Main()
  {
    // Initialize eCAL API.
    Core.Initialize("minimal client csharp");

    Console.WriteLine(String.Format("eCAL {0} ({1})\n", Core.GetVersion(), Core.GetDate()));

    // Create a service client for service "service1"
    ServiceClient serviceClient = new ServiceClient("service1");

    while (Core.Ok())
    {
      // Create the request payload.
      byte[] content = Encoding.UTF8.GetBytes("hello");

      // Call the service method "echo" with a 100 ms timeout.
      List<ServiceResponse> responseList = serviceClient.CallWithResponse("echo", content, 100);
      if (responseList.Count > 0)
      {
        foreach (ServiceResponse response in responseList)
        {
          PrintServiceResponse(response);
        }
      }
      else
      {
        Console.WriteLine("Calling service echo failed!");
      }

      System.Threading.Thread.Sleep(1000);
    }

    // Dispose the service client.
    serviceClient.Dispose();

    // Finalize eCAL API.
    Core.Terminate();
  }
}
