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
 * distributed under the License on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * ========================= eCAL LICENSE =================================
*/

using System;
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

    Console.WriteLine("Received service response in C#: " + callState);
    Console.WriteLine("Method    : " + serviceResponse.MethodInformation.MethodName);
    Console.WriteLine("Response  : " + Encoding.UTF8.GetString(serviceResponse.Response));
    Console.WriteLine("Server ID : " + serviceResponse.ServerId.EntityID.Id);
    Console.WriteLine("Host      : " + serviceResponse.ServerId.EntityID.HostName);
    Console.WriteLine();
  }

  public static void Main()
  {
    Console.WriteLine("---------------------------------");
    Console.WriteLine(" C#: MIRROR CLIENT WITH CALLBACK");
    Console.WriteLine("---------------------------------");

    /*
      As always: initialize the eCAL API and give your process a name.
    */
    Core.Initialize("mirror client with callback c#");

    Console.WriteLine(String.Format("eCAL {0} ({1})\n", Core.GetVersion(), Core.GetDate()));

    /*
      Create a client that connects to a "mirror" server.
      It may call the methods "echo" and "reverse"
    */
    ServiceMethodInformationList methodInformationList = new ServiceMethodInformationList();
    methodInformationList.Methods.Add(new ServiceMethodInformation("echo", new DataTypeInformation(), new DataTypeInformation()));
    methodInformationList.Methods.Add(new ServiceMethodInformation("reverse", new DataTypeInformation(), new DataTypeInformation()));

    // Create the client
    var mirrorClient = new ServiceClient("mirror", methodInformationList);

    // Wait until we have at least one server connected
    while (!mirrorClient.IsConnected())
    {
      System.Threading.Thread.Sleep(100);
    }

    /*
      Allow to alternate between the two methods "echo" and "reverse".
    */
    int i = 0;
    var methods = new[] { "echo", "reverse" };

    while (Core.Ok())
    {
      /* 
        Alternate between the two methods "echo" and "reverse".
        Create the request payload.
      */
      var method  = methods[i++ % methods.Length];
      byte[] request = Encoding.UTF8.GetBytes("stressed");

      /*
        Service call: blocking with callback
      */
      mirrorClient.CallWithCallback(
        method,
        request,
        response => PrintServiceResponse(response),
        (int)ServiceClient.DefaultTimeArgument
      );

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
