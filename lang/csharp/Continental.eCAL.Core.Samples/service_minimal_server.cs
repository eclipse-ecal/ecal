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

public class minimal_server
{
  // This is pass by value. This is truly not optimal
  static byte[] OnMethodCallback(String method_, String request_type_, String response_type_, byte[] request_)
  {
    System.Console.WriteLine("Method called " + method_);
    System.Console.WriteLine("Request " + System.Text.Encoding.UTF8.GetString(request_));
    // Maybe need to copy???
    return request_;
  }

  static void Main()
  {
    // initialize eCAL API
    Util.Initialize("minimal_service_csharp");

    // print version info
    System.Console.WriteLine(String.Format("eCAL {0} ({1})\n", Util.GetVersion(), Util.GetDate()));

    // create a subscriber (topic name "Hello", type "base:std::string")
    var service_server_ = new ServiceServer("service1");
    ServiceServer.MethodCallback callback = OnMethodCallback;
    service_server_.AddMethodCallback("echo", "", "", callback);

    // idle main thread
    while (Util.Ok())
    {
      System.Threading.Thread.Sleep(100);
    }

    // dispose service server
    // Is this neccessary in a C# context?
    service_server_.Dispose();

    // finalize eCAL API
    Util.Terminate();
  }
}
