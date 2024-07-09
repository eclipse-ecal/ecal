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
using Continental.eCAL.Core;

public class minimal_client
{

  // This is pass by value. This is truly not optimal
  static void PrintCallbackResult(ServiceClient.ServiceClientCallbackData data)
  {
    switch (data.call_state)
    {
      case ServiceClient.CallState.Executed:
        System.Console.WriteLine("Received response for method " + data.method_name + ":  " + System.Text.Encoding.UTF8.GetString(data.response) + " from host " + data.host_name);
        break;
      case ServiceClient.CallState.Failed:
        System.Console.WriteLine("Received error : " + data.error_msg + " from host " + data.host_name);
        break;
    }
  }

    static void Main()
    {
      // initialize eCAL API
      Util.Initialize("minimal_client_csharp");

      // print version info
      System.Console.WriteLine(String.Format("eCAL {0} ({1})\n", Util.GetVersion(), Util.GetDate()));

      // create a subscriber (topic name "Hello", type "base:std::string")
      var service_client = new ServiceClient("service1");

      // idle main thread
      while (Util.Ok())
      {
        var content = System.Text.Encoding.UTF8.GetBytes("hello");
        var result = service_client.Call("echo", content, 100);
        if (result != null)
        {
          foreach (var res in result)
          {
            PrintCallbackResult(res);
          }
        }
        else
        {
          System.Console.WriteLine("Calling service echo failed!");
        }

      System.Threading.Thread.Sleep(1000);
      }

      // dispose service server
      // Is this neccessary in a C# context?
      service_client.Dispose();

      // finalize eCAL API
      Util.Terminate();
    }
  };


