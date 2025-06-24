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
  /*
    Helper function to print the service response.
  */
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

    Console.WriteLine("Received service response in C: " + callState);
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

    /*
      As always: initialize the eCAL API and give your process a name.
    */
    Core.Initialize("mirror client c#");

    Console.WriteLine(String.Format("eCAL {0} ({1})\n", Core.GetVersionString(), Core.GetVersionDateString()));

    /*
      Create a client that connects to a "mirror" server.
      It may call the methods "echo" and "reverse"
    */
    ServiceMethodInformationList methodInformationList = new ServiceMethodInformationList();
    methodInformationList.Methods.Add(new ServiceMethodInformation("echo", new DataTypeInformation(), new DataTypeInformation()));
    methodInformationList.Methods.Add(new ServiceMethodInformation("reverse", new DataTypeInformation(), new DataTypeInformation()));

    ServiceClient mirrorClient = new ServiceClient("mirror", methodInformationList);

    /*
      We wait until the client is connected to a server,
      so we don't call methods that are not available.
    */
    while (!mirrorClient.IsConnected())
    {
      Console.WriteLine("Waiting for a service ...");
      System.Threading.Thread.Sleep(1000);
    }

    /*
      Allow to alternate between the two methods "echo" and "reverse".
    */
    int i = 0;
    string[] methods = new string[] { "echo", "reverse" };

    while (Core.Ok())
    {
      /* 
        Alternate between the two methods "echo" and "reverse".
        Create the request payload.
      */
      string method = methods[i++ % methods.Length];
      byte[] request = Encoding.UTF8.GetBytes("stressed");

      /*
        Service call with response
      */
      List<ServiceResponse> responseList = mirrorClient.CallWithResponse(method, request, (int)Eclipse.eCAL.Core.ServiceClient.DefaultTimeArgument);
      
      /*
        Iterate through all responses and print them.
      */
      if (responseList.Count > 0)
      {
        foreach (ServiceResponse response in responseList)
        {
          PrintServiceResponse(response);
        }
      }
      else
      {
        Console.WriteLine("Method call with response failed.");
      }

      /*
        Service call with callback
      */
      if (!mirrorClient.CallWithCallback(
        method,
        request,
        response => PrintServiceResponse(response),
        (int)ServiceClient.DefaultTimeArgument)
      )
      {
        Console.WriteLine("Method call with callback failed.");
      }

      System.Threading.Thread.Sleep(1000);
    }

    /*
      When finished, we need to dispose the client to clean up properly.
    */
    mirrorClient.Dispose();

    /*
      After we are done, as always, finalize the eCAL API.
    */
    Core.Terminate();
  }
}
