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

public class minimal_snd
{
  static void Main()
  {
    // initialize eCAL API
    Util.Initialize("minimal_snd");

    // print version info
    System.Console.WriteLine(String.Format("eCAL {0} ({1})\n", Util.GetVersion(), Util.GetDate()));

    // create a publisher (topic name "Hello", type "base:std::string", description "")
    Publisher publisher = new Publisher("Hello", "base:std::string", "");

    // idle main thread
    int loop = 0;
    while (Util.Ok())
    {
      // message to send
      string message = String.Format("HELLO WORLD FROM C# {0,6}", ++loop);

      // print message
      System.Console.WriteLine(String.Format("Sending:  {0}", message));

      // send the content
      publisher.Send(message, -1);

      // cool down
      System.Threading.Thread.Sleep(100);
    }

    // dispose publisher
    publisher.Dispose();

    // finalize eCAL API
    Util.Terminate();
  }
}
